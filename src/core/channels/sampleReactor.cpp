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
#include "src/core/model/model.h"
#include "utils/math.h"
#include <cassert>

namespace giada::m::sampleReactor
{
namespace
{
constexpr int Q_ACTION_PLAY   = 0;
constexpr int Q_ACTION_REWIND = 1;

/* -------------------------------------------------------------------------- */

void rewind_(channel::Data& ch, Frame localFrame)
{
	ch.state->rewinding = true;
	ch.state->offset    = localFrame;
}

/* -------------------------------------------------------------------------- */

void reset_(channel::Data& ch)
{
	ch.state->tracker.store(ch.samplePlayer->begin);
}

/* -------------------------------------------------------------------------- */

ChannelStatus pressWhileOff_(channel::Data& ch, Sequencer& sequencer, int velocity, bool isLoop)
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

ChannelStatus pressWhilePlay_(channel::Data& ch, Sequencer& sequencer, SamplePlayerMode mode, bool isLoop)
{
	if (isLoop)
		return ChannelStatus::ENDING;

	switch (mode)
	{
	case SamplePlayerMode::SINGLE_RETRIG:
		if (sequencer.canQuantize())
			sequencer.quantizer.trigger(Q_ACTION_REWIND + ch.id);
		else
			rewind_(ch, /*localFrame=*/0);
		return ChannelStatus::PLAY;

	case SamplePlayerMode::SINGLE_ENDLESS:
		return ChannelStatus::ENDING;

	case SamplePlayerMode::SINGLE_BASIC:
		reset_(ch);
		return ChannelStatus::OFF;

	default:
		return ChannelStatus::OFF;
	}
}

/* -------------------------------------------------------------------------- */

void press_(channel::Data& ch, Sequencer& sequencer, int velocity)
{
	const SamplePlayerMode mode   = ch.samplePlayer->mode;
	const bool             isLoop = ch.samplePlayer->isAnyLoopMode();

	ChannelStatus playStatus = ch.state->playStatus.load();

	switch (playStatus)
	{
	case ChannelStatus::OFF:
		playStatus = pressWhileOff_(ch, sequencer, velocity, isLoop);
		break;

	case ChannelStatus::PLAY:
		playStatus = pressWhilePlay_(ch, sequencer, mode, isLoop);
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

void kill_(channel::Data& ch)
{
	ch.state->playStatus.store(ChannelStatus::OFF);
	ch.state->tracker.store(ch.samplePlayer->begin);
}
/* -------------------------------------------------------------------------- */

void release_(channel::Data& ch, Sequencer& sequencer)
{
	/* Key release is meaningful only for SINGLE_PRESS modes. */

	if (ch.samplePlayer->mode != SamplePlayerMode::SINGLE_PRESS)
		return;

	/* Kill it if it's SINGLE_PRESS is playing. Otherwise there might be a 
	quantization step in progress that would play the channel later on: 
	disable it. */

	if (ch.state->playStatus.load() == ChannelStatus::PLAY)
		kill_(ch);
	else if (sequencer.quantizer.hasBeenTriggered())
		sequencer.quantizer.clear();
}

/* -------------------------------------------------------------------------- */

void onStopBySeq_(channel::Data& ch, bool chansStopOnSeqHalt)
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
			kill_(ch);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void toggleReadActions_(channel::Data& ch, bool isSequencerRunning, bool treatRecsAsLoops)
{
	if (isSequencerRunning && ch.state->recStatus.load() == ChannelStatus::PLAY && !treatRecsAsLoops)
		kill_(ch);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data::Data(ID channelId, Sequencer& sequencer, channel::Data& ch)
{
	sequencer.quantizer.schedule(Q_ACTION_PLAY + channelId, [&ch](Frame delta) {
		ch.state->offset = delta;
		ch.state->playStatus.store(ChannelStatus::PLAY);
	});

	sequencer.quantizer.schedule(Q_ACTION_REWIND + channelId, [&ch](Frame delta) {
		ch.isPlaying() ? rewind_(ch, delta) : reset_(ch);
	});
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void react(channel::Data& ch, const EventDispatcher::Event& e, Sequencer& sequencer, const Conf::Data& conf)
{
	if (!ch.hasWave())
		return;

	switch (e.type)
	{
	case EventDispatcher::EventType::KEY_PRESS:
		press_(ch, sequencer, std::get<int>(e.data));
		break;

	case EventDispatcher::EventType::KEY_RELEASE:
		release_(ch, sequencer);
		break;

	case EventDispatcher::EventType::KEY_KILL:
		kill_(ch);
		break;

	case EventDispatcher::EventType::SEQUENCER_STOP:
		onStopBySeq_(ch, conf.chansStopOnSeqHalt);
		break;

	case EventDispatcher::EventType::CHANNEL_TOGGLE_READ_ACTIONS:
		toggleReadActions_(ch, sequencer.isRunning(), conf.treatRecsAsLoops);
		break;

	default:
		break;
	}
}
} // namespace giada::m::sampleReactor