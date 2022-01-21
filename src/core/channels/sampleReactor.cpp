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

namespace giada::m
{
namespace
{
constexpr int Q_ACTION_PLAY   = 0;
constexpr int Q_ACTION_REWIND = 10000; // Avoid clash with Q_ACTION_PLAY + channelId
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

SampleReactor::SampleReactor(Channel& ch, ID channelId)
{
	ch.shared->quantizer->schedule(Q_ACTION_PLAY + channelId, [this, shared = ch.shared](Frame delta) {
		play(*shared, delta);
	});

	ch.shared->quantizer->schedule(Q_ACTION_REWIND + channelId, [this, shared = ch.shared](Frame delta) {
		ChannelStatus status = shared->playStatus.load();
		if (status == ChannelStatus::OFF)
			play(*shared, delta);
		else if (status == ChannelStatus::PLAY || status == ChannelStatus::ENDING)
			rewind(*shared, delta);
	});
}

/* -------------------------------------------------------------------------- */

void SampleReactor::react(Channel& ch, const EventDispatcher::Event& e,
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
		release(ch);
		break;

	case EventDispatcher::EventType::KEY_KILL:
		if (ch.shared->playStatus.load() == ChannelStatus::PLAY)
			stop(*ch.shared);
		break;

	case EventDispatcher::EventType::SEQUENCER_STOP:
		onStopBySeq(ch, conf.chansStopOnSeqHalt);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void SampleReactor::rewind(ChannelShared& shared, Frame localFrame) const
{
	shared.renderQueue->push({SamplePlayer::Render::Mode::REWIND, localFrame});
}

/* -------------------------------------------------------------------------- */

void SampleReactor::play(ChannelShared& shared, Frame localFrame) const
{
	shared.playStatus.store(ChannelStatus::PLAY);
	shared.renderQueue->push({SamplePlayer::Render::Mode::NORMAL, localFrame});
}

/* -------------------------------------------------------------------------- */

void SampleReactor::stop(ChannelShared& shared) const
{
	shared.renderQueue->push({SamplePlayer::Render::Mode::STOP, 0});
}

/* -------------------------------------------------------------------------- */

ChannelStatus SampleReactor::pressWhileOff(Channel& ch, Sequencer& sequencer,
    int velocity, bool isLoop) const
{
	if (isLoop)
		return ChannelStatus::WAIT;

	if (ch.samplePlayer->velocityAsVol)
		ch.volume_i = u::math::map(velocity, G_MAX_VELOCITY, G_MAX_VOLUME);

	if (sequencer.canQuantize())
	{
		ch.shared->quantizer->trigger(Q_ACTION_PLAY + ch.id);
		return ChannelStatus::OFF;
	}
	else
		return ChannelStatus::PLAY;
}

/* -------------------------------------------------------------------------- */

ChannelStatus SampleReactor::pressWhilePlay(Channel& ch, Sequencer& sequencer,
    SamplePlayerMode mode, bool isLoop) const
{
	if (isLoop)
		return ChannelStatus::ENDING;

	switch (mode)
	{
	case SamplePlayerMode::SINGLE_RETRIG:
		if (sequencer.canQuantize())
			ch.shared->quantizer->trigger(Q_ACTION_REWIND + ch.id);
		else
			rewind(*ch.shared, /*localFrame=*/0);
		return ChannelStatus::PLAY;

	case SamplePlayerMode::SINGLE_ENDLESS:
		return ChannelStatus::ENDING;

	case SamplePlayerMode::SINGLE_BASIC:
		stop(*ch.shared);
		return ChannelStatus::PLAY; // Let SamplePlayer stop it once done

	default:
		return ChannelStatus::OFF;
	}
}

/* -------------------------------------------------------------------------- */

void SampleReactor::press(Channel& ch, Sequencer& sequencer, int velocity) const
{
	const SamplePlayerMode mode   = ch.samplePlayer->mode;
	const bool             isLoop = ch.samplePlayer->isAnyLoopMode();

	ChannelStatus playStatus = ch.shared->playStatus.load();

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

	ch.shared->playStatus.store(playStatus);
}

/* -------------------------------------------------------------------------- */

void SampleReactor::release(Channel& ch) const
{
	/* Key release is meaningful only for SINGLE_PRESS modes. */

	if (ch.samplePlayer->mode != SamplePlayerMode::SINGLE_PRESS)
		return;

	/* Kill it if it's SINGLE_PRESS is playing. Otherwise there might be a 
	quantization step in progress that would play the channel later on: 
	disable it. */

	if (ch.shared->playStatus.load() == ChannelStatus::PLAY)
		stop(*ch.shared); // Let SamplePlayer stop it once done
	else if (ch.shared->quantizer->hasBeenTriggered())
		ch.shared->quantizer->clear();
}

/* -------------------------------------------------------------------------- */

void SampleReactor::onStopBySeq(Channel& ch, bool chansStopOnSeqHalt) const
{
	G_DEBUG("onStopBySeq ch=" << ch.id);

	ChannelStatus playStatus       = ch.shared->playStatus.load();
	bool          isReadingActions = ch.shared->readActions.load();
	bool          isLoop           = ch.samplePlayer->isAnyLoopMode();

	switch (playStatus)
	{

	case ChannelStatus::WAIT:
		/* Loop-mode channels in wait status get stopped right away. */
		if (isLoop)
			ch.shared->playStatus.store(ChannelStatus::OFF);
		break;

	case ChannelStatus::PLAY:
		if (chansStopOnSeqHalt && (isLoop || isReadingActions))
			stop(*ch.shared);
		break;

	default:
		break;
	}
}
} // namespace giada::m