/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories

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

#include "core/channels/sampleAdvancer.h"
#include "core/channels/channelShared.h"
#include "core/rendering/sampleRendering.h"

namespace giada::m
{
void SampleAdvancer::onLastFrame(ChannelShared& shared, bool seqIsRunning, bool natural,
    SamplePlayerMode mode, bool isLoop) const
{
	switch (shared.playStatus.load())
	{
	case ChannelStatus::PLAY:
		/* Stop LOOP_* when the sequencer is off, or SINGLE_* except for
		SINGLE_ENDLESS, which runs forever unless it's in ENDING mode. 
		Other loop once modes are put in wait mode. */
		if ((mode == SamplePlayerMode::SINGLE_BASIC ||
		        mode == SamplePlayerMode::SINGLE_BASIC_PAUSE ||
		        mode == SamplePlayerMode::SINGLE_PRESS ||
		        mode == SamplePlayerMode::SINGLE_RETRIG) ||
		    (isLoop && !seqIsRunning) || !natural)
			shared.playStatus.store(ChannelStatus::OFF);
		else if (mode == SamplePlayerMode::LOOP_ONCE || mode == SamplePlayerMode::LOOP_ONCE_BAR)
			shared.playStatus.store(ChannelStatus::WAIT);
		break;

	case ChannelStatus::ENDING:
		shared.playStatus.store(ChannelStatus::OFF);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void SampleAdvancer::advance(ID channelId, ChannelShared& shared,
    const Sequencer::Event& e, SamplePlayerMode mode, bool isLoop) const
{
	switch (e.type)
	{
	case Sequencer::EventType::FIRST_BEAT:
		onFirstBeat(shared, e.delta, isLoop);
		break;

	case Sequencer::EventType::BAR:
		onBar(shared, e.delta, mode);
		break;

	case Sequencer::EventType::REWIND:
		if (isLoop)
			rewind(shared, e.delta);
		break;

	case Sequencer::EventType::ACTIONS:
		if (!isLoop && shared.isReadingActions())
			parseActions(channelId, shared, *e.actions, e.delta, mode);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void SampleAdvancer::rewind(ChannelShared& shared, Frame localFrame) const
{
	shared.renderQueue->push({rendering::RenderInfo::Mode::REWIND, localFrame});
}

/* -------------------------------------------------------------------------- */

void SampleAdvancer::stop(ChannelShared& shared, Frame localFrame) const
{
	shared.renderQueue->push({rendering::RenderInfo::Mode::STOP, localFrame});
}

/* -------------------------------------------------------------------------- */

void SampleAdvancer::play(ChannelShared& shared, Frame localFrame) const
{
	shared.playStatus.store(ChannelStatus::PLAY);
	shared.renderQueue->push({rendering::RenderInfo::Mode::NORMAL, localFrame});
}

/* -------------------------------------------------------------------------- */

void SampleAdvancer::onFirstBeat(ChannelShared& shared, Frame localFrame, bool isLoop) const
{
	const ChannelStatus playStatus = shared.playStatus.load();
	const ChannelStatus recStatus  = shared.recStatus.load();

	switch (playStatus)
	{
	case ChannelStatus::PLAY:
		if (isLoop)
			rewind(shared, localFrame);
		break;

	case ChannelStatus::WAIT:
		play(shared, localFrame);
		break;

	case ChannelStatus::ENDING:
		if (isLoop)
			stop(shared, localFrame);
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

void SampleAdvancer::onBar(ChannelShared& shared, Frame localFrame, SamplePlayerMode mode) const
{
	const ChannelStatus playStatus = shared.playStatus.load();

	if (playStatus == ChannelStatus::PLAY &&
	    (mode == SamplePlayerMode::LOOP_REPEAT || mode == SamplePlayerMode::LOOP_ONCE_BAR))
		rewind(shared, localFrame);
	else if (playStatus == ChannelStatus::WAIT && mode == SamplePlayerMode::LOOP_ONCE_BAR)
		play(shared, localFrame);
}

/* -------------------------------------------------------------------------- */

void SampleAdvancer::onNoteOn(ChannelShared& shared, Frame localFrame, SamplePlayerMode mode) const
{
	switch (shared.playStatus.load())
	{
	case ChannelStatus::OFF:
		play(shared, localFrame);
		break;

	case ChannelStatus::PLAY:
		if (mode == SamplePlayerMode::SINGLE_RETRIG)
			rewind(shared, localFrame);
		else
			stop(shared, localFrame);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void SampleAdvancer::parseActions(ID channelId, ChannelShared& shared,
    const std::vector<Action>& as, Frame localFrame, SamplePlayerMode mode) const
{
	for (const Action& a : as)
	{
		if (a.channelId != channelId)
			continue;

		switch (a.event.getStatus())
		{
		case MidiEvent::CHANNEL_NOTE_ON:
			onNoteOn(shared, localFrame, mode);
			break;

		case MidiEvent::CHANNEL_NOTE_OFF:
		case MidiEvent::CHANNEL_NOTE_KILL:
			if (shared.playStatus.load() == ChannelStatus::PLAY)
				stop(shared, localFrame);
			break;

		default:
			break;
		}
	}
}
} // namespace giada::m