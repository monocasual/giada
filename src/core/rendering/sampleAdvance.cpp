/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
 *
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

#include "src/core/rendering/sampleAdvance.h"
#include "src/core/channels/channel.h"
#include "src/core/channels/channelShared.h"
#include "src/core/rendering/sampleReactions.h"

namespace giada::m::rendering
{
namespace
{
void onFirstBeat_(ChannelShared& shared, Frame localFrame, bool isLoop)
{
	const ChannelStatus playStatus = shared.playStatus.load();
	const ChannelStatus recStatus  = shared.recStatus.load();

	switch (playStatus)
	{
	case ChannelStatus::PLAY:
		if (isLoop)
			rewindSampleChannel(shared, localFrame);
		break;

	case ChannelStatus::WAIT:
		playSampleChannel(shared, localFrame);
		break;

	case ChannelStatus::ENDING:
		if (isLoop)
			stopSampleChannel(shared, localFrame);
		break;

	default:
		break;
	}

	switch (recStatus)
	{
	case ChannelStatus::WAIT:
		shared.recStatus.store(ChannelStatus::PLAY);
		shared.readActions.store(true);
		break;

	case ChannelStatus::ENDING:
		shared.recStatus.store(ChannelStatus::OFF);
		shared.readActions.store(false);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void onBar_(ChannelShared& shared, Frame localFrame, SamplePlayerMode mode)
{
	const ChannelStatus playStatus = shared.playStatus.load();

	if (playStatus == ChannelStatus::PLAY &&
	    (mode == SamplePlayerMode::LOOP_REPEAT || mode == SamplePlayerMode::LOOP_ONCE_BAR))
		rewindSampleChannel(shared, localFrame);
	else if (playStatus == ChannelStatus::WAIT && mode == SamplePlayerMode::LOOP_ONCE_BAR)
		playSampleChannel(shared, localFrame);
}

/* -------------------------------------------------------------------------- */

void onNoteOn_(ChannelShared& shared, Frame localFrame, SamplePlayerMode mode, float velocity)
{
	shared.volumeInternal.store(velocity);

	switch (shared.playStatus.load())
	{
	case ChannelStatus::OFF:
		playSampleChannel(shared, localFrame);
		break;

	case ChannelStatus::PLAY:
		if (mode == SamplePlayerMode::SINGLE_RETRIG)
			rewindSampleChannel(shared, localFrame);
		else
			stopSampleChannel(shared, localFrame);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void parseActions_(ID channelId, std::size_t scene, ChannelShared& shared, const std::vector<Action>& as,
    Frame localFrame, SamplePlayerMode mode)
{
	for (const Action& a : as)
	{
		if (a.channelId != channelId || a.scene != scene)
			continue;

		switch (a.event.getStatus())
		{
		case MidiEvent::CHANNEL_NOTE_ON:
			onNoteOn_(shared, localFrame, mode, a.event.getVelocityFloat());
			break;

		case MidiEvent::CHANNEL_NOTE_OFF:
		case MidiEvent::CHANNEL_NOTE_KILL:
			if (shared.playStatus.load() == ChannelStatus::PLAY)
				stopSampleChannel(shared, localFrame);
			break;

		default:
			break;
		}
	}
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void advanceSampleChannel(const Channel& ch, std::size_t scene, const Sequencer::Event& e)
{
	const SamplePlayerMode mode   = ch.sampleChannel->mode;
	const bool             isLoop = ch.sampleChannel->isAnyLoopMode();

	switch (e.type)
	{
	case Sequencer::EventType::FIRST_BEAT:
		onFirstBeat_(*ch.shared, e.delta, isLoop);
		break;

	case Sequencer::EventType::BAR:
		onBar_(*ch.shared, e.delta, mode);
		break;

	case Sequencer::EventType::REWIND:
		if (isLoop)
			rewindSampleChannel(*ch.shared, e.delta);
		break;

	case Sequencer::EventType::ACTIONS:
		if (!isLoop && ch.shared->isReadingActions())
			parseActions_(ch.id, scene, *ch.shared, *e.actions, e.delta, mode);
		break;

	default:
		break;
	}
}
} // namespace giada::m::rendering
