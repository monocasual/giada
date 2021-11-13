/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual

 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */

#include "core/channels/sampleReactor.h"
#include "core/channels/channel.h"
#include "core/conf.h"
#include "core/model/model.h"
#include "utils/math.h"
#include <cassert>

namespace giada::m::sampleReactor
{
namespace
{
constexpr int Q_ACTION_PLAY   = 0;
constexpr int Q_ACTION_REWIND = 1;
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data::Data(ID channelId, Sequencer& sequencer, model::Model& model)
{
	sequencer.quantizer.schedule(Q_ACTION_PLAY + channelId, [channelId, &model](Frame delta) {
		channel::Data& ch = model.get().getChannel(channelId);
		ch.state->offset  = delta;
		ch.state->playStatus.store(ChannelStatus::PLAY);
	});

	sequencer.quantizer.schedule(Q_ACTION_REWIND + channelId, [this, channelId, &model](Frame delta) {
		channel::Data& ch = model.get().getChannel(channelId);
		ch.isPlaying() ? rewind(ch, delta) : reset(ch);
	});
}

/* -------------------------------------------------------------------------- */

void Data::react(channel::Data& ch, const EventDispatcher::Event& e,
    Sequencer& sequencer, const Conf::Data& conf) const
{
	if (!ch.hasWave())
		return;

	switch (e.type)
	{
	case EventDispatcher::EventType::KEY_PRESS:
		press(ch, sequencer, std::get<int>(e.data));
		break;

	case EventDispatcher::EventType::KEY_RELEASE:
		release(ch, sequencer);
		break;

	case EventDispatcher::EventType::KEY_KILL:
		kill(ch);
		break;

	case EventDispatcher::EventType::SEQUENCER_STOP:
		onStopBySeq(ch, conf.chansStopOnSeqHalt);
		break;

	case EventDispatcher::EventType::CHANNEL_TOGGLE_READ_ACTIONS:
		toggleReadActions(ch, sequencer.isRunning(), conf.treatRecsAsLoops);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void Data::rewind(channel::Data& ch, Frame localFrame) const
{
	ch.state->rewinding = true;
	ch.state->offset    = localFrame;
}

/* -------------------------------------------------------------------------- */

void Data::reset(channel::Data& ch) const
{
	ch.state->tracker.store(ch.samplePlayer->begin);
}

/* -------------------------------------------------------------------------- */

ChannelStatus Data::pressWhileOff(channel::Data& ch, Sequencer& sequencer,
    int velocity, bool isLoop) const
{
	if (isLoop)
		return ChannelStatus::WAIT;

	if (ch.samplePlayer->velocityAsVol)
		ch.volume_i = u::math::map(velocity, G_MAX_VELOCITY, G_MAX_VOLUME);

	if (sequencer.canQuantize())
	{
		sequencer.quantizer.trigger(Q_ACTION_PLAY + ch.id);
		return ChannelStatus::OFF;
	}
	else
		return ChannelStatus::PLAY;
}

/* -------------------------------------------------------------------------- */

ChannelStatus Data::pressWhilePlay(channel::Data& ch, Sequencer& sequencer,
    SamplePlayerMode mode, bool isLoop) const
{
	if (isLoop)
		return ChannelStatus::ENDING;

	switch (mode)
	{
	case SamplePlayerMode::SINGLE_RETRIG:
		if (sequencer.canQuantize())
			sequencer.quantizer.trigger(Q_ACTION_REWIND + ch.id);
		else
			rewind(ch, /*localFrame=*/0);
		return ChannelStatus::PLAY;

	case SamplePlayerMode::SINGLE_ENDLESS:
		return ChannelStatus::ENDING;

	case SamplePlayerMode::SINGLE_BASIC:
		reset(ch);
		return ChannelStatus::OFF;

	default:
		return ChannelStatus::OFF;
	}
}

/* -------------------------------------------------------------------------- */

void Data::press(channel::Data& ch, Sequencer& sequencer, int velocity) const
{
	const SamplePlayerMode mode   = ch.samplePlayer->mode;
	const bool             isLoop = ch.samplePlayer->isAnyLoopMode();

	ChannelStatus playStatus = ch.state->playStatus.load();

	switch (playStatus)
	{
	case ChannelStatus::OFF:
		playStatus = pressWhileOff(ch, sequencer, velocity, isLoop);
		break;

	case ChannelStatus::PLAY:
		playStatus = pressWhilePlay(ch, sequencer, mode, isLoop);
		break;

	case ChannelStatus::WAIT:
		playStatus = ChannelStatus::OFF;
		break;

	case ChannelStatus::ENDING:
		playStatus = ChannelStatus::PLAY;
		break;

	default:
		break;
	}

	ch.state->playStatus.store(playStatus);
}

/* -------------------------------------------------------------------------- */

void Data::kill(channel::Data& ch) const
{
	ch.state->playStatus.store(ChannelStatus::OFF);
	ch.state->tracker.store(ch.samplePlayer->begin);
}
/* -------------------------------------------------------------------------- */

void Data::release(channel::Data& ch, Sequencer& sequencer) const
{
	/* Key release is meaningful only for SINGLE_PRESS modes. */

	if (ch.samplePlayer->mode != SamplePlayerMode::SINGLE_PRESS)
		return;

	/* Kill it if it's SINGLE_PRESS is playing. Otherwise there might be a 
	quantization step in progress that would play the channel later on: 
	disable it. */

	if (ch.state->playStatus.load() == ChannelStatus::PLAY)
		kill(ch);
	else if (sequencer.quantizer.hasBeenTriggered())
		sequencer.quantizer.clear();
}

/* -------------------------------------------------------------------------- */

void Data::onStopBySeq(channel::Data& ch, bool chansStopOnSeqHalt) const
{
	G_DEBUG("onStopBySeq ch=" << ch.id);

	ChannelStatus playStatus       = ch.state->playStatus.load();
	bool          isReadingActions = ch.state->readActions.load();
	bool          isLoop           = ch.samplePlayer->isAnyLoopMode();

	switch (playStatus)
	{

	case ChannelStatus::WAIT:
		/* Loop-mode channels in wait status get stopped right away. */
		if (isLoop)
			ch.state->playStatus.store(ChannelStatus::OFF);
		break;

	case ChannelStatus::PLAY:
		if (chansStopOnSeqHalt && (isLoop || isReadingActions))
			kill(ch);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void Data::toggleReadActions(channel::Data& ch, bool isSequencerRunning, bool treatRecsAsLoops) const
{
	if (isSequencerRunning && ch.state->recStatus.load() == ChannelStatus::PLAY && !treatRecsAsLoops)
		kill(ch);
}
} // namespace giada::m::sampleReactor