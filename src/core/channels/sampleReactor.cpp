/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories

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
#include "core/channels/channelShared.h"
#include "utils/math.h"

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

SampleReactor::SampleReactor(ChannelShared& shared, ID channelId)
{
	shared.quantizer->schedule(Q_ACTION_PLAY + channelId, [this, &shared](Frame delta) {
		play(shared, delta);
	});

	shared.quantizer->schedule(Q_ACTION_REWIND + channelId, [this, &shared](Frame delta) {
		const ChannelStatus status = shared.playStatus.load();
		if (status == ChannelStatus::OFF)
			play(shared, delta);
		else if (status == ChannelStatus::PLAY || status == ChannelStatus::ENDING)
			rewind(shared, delta);
	});
}

/* -------------------------------------------------------------------------- */

void SampleReactor::react(ID channelId, ChannelShared& shared, const EventDispatcher::Event& e,
    SamplePlayerMode mode, bool velocityAsVol, bool chansStopOnSeqHalt, bool canQuantize,
    bool isLoop, float& volume_i) const
{
	switch (e.type)
	{
	case EventDispatcher::EventType::KEY_PRESS:
		press(channelId, shared, mode, std::get<int>(e.data), canQuantize, isLoop, velocityAsVol, volume_i);
		break;

	case EventDispatcher::EventType::KEY_RELEASE:
		if (mode == SamplePlayerMode::SINGLE_PRESS) // Key release is meaningful only for SINGLE_PRESS modes
			release(shared);
		break;

	case EventDispatcher::EventType::KEY_KILL:
		if (shared.playStatus.load() == ChannelStatus::PLAY)
			stop(shared);
		break;

	case EventDispatcher::EventType::SEQUENCER_STOP:
		onStopBySeq(shared, chansStopOnSeqHalt, isLoop);
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

ChannelStatus SampleReactor::pressWhileOff(ID channelId, ChannelShared& shared,
    int velocity, bool canQuantize, bool velocityAsVol, float& volume_i) const
{
	if (velocityAsVol)
		volume_i = u::math::map(velocity, G_MAX_VELOCITY, G_MAX_VOLUME);

	if (canQuantize)
	{
		shared.quantizer->trigger(Q_ACTION_PLAY + channelId);
		return ChannelStatus::OFF;
	}
	else
		return ChannelStatus::PLAY;
}

/* -------------------------------------------------------------------------- */

ChannelStatus SampleReactor::pressWhilePlay(ID channelId, ChannelShared& shared,
    SamplePlayerMode mode, bool canQuantize) const
{
	switch (mode)
	{
	case SamplePlayerMode::SINGLE_RETRIG:
		if (canQuantize)
			shared.quantizer->trigger(Q_ACTION_REWIND + channelId);
		else
			rewind(shared, /*localFrame=*/0);
		return ChannelStatus::PLAY;

	case SamplePlayerMode::SINGLE_ENDLESS:
		return ChannelStatus::ENDING;

	case SamplePlayerMode::SINGLE_BASIC:
		stop(shared);
		return ChannelStatus::PLAY; // Let SamplePlayer stop it once done

	default:
		return ChannelStatus::OFF;
	}
}

/* -------------------------------------------------------------------------- */

void SampleReactor::press(ID channelId, ChannelShared& shared, SamplePlayerMode mode,
    int velocity, bool canQuantize, bool isLoop, bool velocityAsVol, float& volume_i) const
{
	ChannelStatus playStatus = shared.playStatus.load();

	switch (playStatus)
	{
	case ChannelStatus::OFF:
		if (isLoop)
			playStatus = ChannelStatus::WAIT;
		else
			playStatus = pressWhileOff(channelId, shared, velocity, canQuantize, velocityAsVol, volume_i);
		break;

	case ChannelStatus::PLAY:
		if (isLoop)
			playStatus = ChannelStatus::ENDING;
		else
			playStatus = pressWhilePlay(channelId, shared, mode, canQuantize);
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

	shared.playStatus.store(playStatus);
}

/* -------------------------------------------------------------------------- */

void SampleReactor::release(ChannelShared& shared) const
{
	/* Kill it if it's SINGLE_PRESS is playing. Otherwise there might be a 
	quantization step in progress that would play the channel later on: 
	disable it. */

	if (shared.playStatus.load() == ChannelStatus::PLAY)
		stop(shared); // Let SamplePlayer stop it once done
	else if (shared.quantizer->hasBeenTriggered())
		shared.quantizer->clear();
}

/* -------------------------------------------------------------------------- */

void SampleReactor::onStopBySeq(ChannelShared& shared, bool chansStopOnSeqHalt, bool isLoop) const
{
	const ChannelStatus playStatus       = shared.playStatus.load();
	const bool          isReadingActions = shared.isReadingActions();

	switch (playStatus)
	{

	case ChannelStatus::WAIT:
		/* Loop-mode channels in wait status get stopped right away. */
		if (isLoop)
			shared.playStatus.store(ChannelStatus::OFF);
		break;

	case ChannelStatus::PLAY:
		if (chansStopOnSeqHalt && (isLoop || isReadingActions))
			stop(shared);
		break;

	default:
		break;
	}
}
} // namespace giada::m