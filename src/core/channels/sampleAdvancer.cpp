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

#include "core/channels/sampleAdvancer.h"
#include "core/channels/channel.h"
#include <cassert>

namespace giada::m::sampleAdvancer
{
namespace
{
void rewind_(const channel::Data& ch, Frame localFrame)
{
	ch.state->rewinding = true;
	ch.state->offset    = localFrame;
}

/* -------------------------------------------------------------------------- */

void stop_(const channel::Data& ch, Frame localFrame)
{
	ch.state->playStatus.store(ChannelStatus::OFF);
	ch.state->tracker.store(ch.samplePlayer->begin);

	/*  Clear data in range [localFrame, (buffer.size)) if the event occurs in
    the middle of the buffer. TODO - samplePlayer should be responsible for this*/

	if (localFrame != 0)
		ch.buffer->audio.clear(localFrame);
}

/* -------------------------------------------------------------------------- */

void play_(const channel::Data& ch, Frame localFrame)
{
	ch.state->playStatus.store(ChannelStatus::PLAY);
	ch.state->offset = localFrame;
}

/* -------------------------------------------------------------------------- */

void onFirstBeat_(const channel::Data& ch, Frame localFrame)
{
	G_DEBUG("onFirstBeat ch=" << ch.id << ", localFrame=" << localFrame);

	const ChannelStatus playStatus = ch.state->playStatus.load();
	const ChannelStatus recStatus  = ch.state->recStatus.load();
	const bool          isLoop     = ch.samplePlayer->isAnyLoopMode();

	switch (playStatus)
	{
	case ChannelStatus::PLAY:
		if (isLoop)
			rewind_(ch, localFrame);
		break;

	case ChannelStatus::WAIT:
		play_(ch, localFrame);
		break;

	case ChannelStatus::ENDING:
		if (isLoop)
			stop_(ch, localFrame);
		break;

	default:
		break;
	}

	switch (recStatus)
	{
	case ChannelStatus::WAIT:
		ch.state->recStatus.store(ChannelStatus::PLAY);
		ch.state->readActions.store(true);
		break;

	case ChannelStatus::ENDING:
		ch.state->recStatus.store(ChannelStatus::OFF);
		ch.state->readActions.store(false);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void onBar_(const channel::Data& ch, Frame localFrame)
{
	G_DEBUG("onBar ch=" << ch.id << ", localFrame=" << localFrame);

	const ChannelStatus    playStatus = ch.state->playStatus.load();
	const SamplePlayerMode mode       = ch.samplePlayer->mode;

	if (playStatus == ChannelStatus::PLAY && (mode == SamplePlayerMode::LOOP_REPEAT ||
	                                             mode == SamplePlayerMode::LOOP_ONCE_BAR))
		rewind_(ch, localFrame);
	else if (playStatus == ChannelStatus::WAIT && mode == SamplePlayerMode::LOOP_ONCE_BAR)
		play_(ch, localFrame);
}

/* -------------------------------------------------------------------------- */

void onNoteOn_(const channel::Data& ch, Frame localFrame)
{
	switch (ch.state->playStatus.load())
	{
	case ChannelStatus::OFF:
		play_(ch, localFrame);
		break;

	case ChannelStatus::PLAY:
		if (ch.samplePlayer->mode == SamplePlayerMode::SINGLE_RETRIG)
			rewind_(ch, localFrame);
		else
			stop_(ch, localFrame);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void parseActions_(const channel::Data& ch, const std::vector<Action>& as, Frame localFrame)
{
	if (ch.samplePlayer->isAnyLoopMode() || !ch.isReadingActions())
		return;

	for (const Action& a : as)
	{
		if (a.channelId != ch.id)
			continue;

		switch (a.event.getStatus())
		{
		case MidiEvent::NOTE_ON:
			onNoteOn_(ch, localFrame);
			break;

		case MidiEvent::NOTE_OFF:
		case MidiEvent::NOTE_KILL:
			stop_(ch, localFrame);
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

void onLastFrame(const channel::Data& ch, bool seqIsRunning)
{
	const SamplePlayerMode mode   = ch.samplePlayer->mode;
	const bool             isLoop = ch.samplePlayer->isAnyLoopMode();

	ChannelStatus playStatus = ch.state->playStatus.load();

	if (playStatus == ChannelStatus::PLAY)
	{
		/* Stop LOOP_* when the sequencer is off, or SINGLE_* except for
		SINGLE_ENDLESS, which runs forever unless it's in ENDING mode. 
		Other loop once modes are put in wait mode. */
		if ((mode == SamplePlayerMode::SINGLE_BASIC ||
		        mode == SamplePlayerMode::SINGLE_BASIC_PAUSE ||
		        mode == SamplePlayerMode::SINGLE_PRESS ||
		        mode == SamplePlayerMode::SINGLE_RETRIG) ||
		    (isLoop && !seqIsRunning))
			playStatus = ChannelStatus::OFF;
		else if (mode == SamplePlayerMode::LOOP_ONCE || mode == SamplePlayerMode::LOOP_ONCE_BAR)
			playStatus = ChannelStatus::WAIT;
	}
	else if (playStatus == ChannelStatus::ENDING)
		playStatus = ChannelStatus::OFF;

	ch.state->playStatus.store(playStatus);
}

/* -------------------------------------------------------------------------- */

void advance(const channel::Data& ch, const Sequencer::Event& e)
{
	switch (e.type)
	{
	case Sequencer::EventType::FIRST_BEAT:
		onFirstBeat_(ch, e.delta);
		break;

	case Sequencer::EventType::BAR:
		onBar_(ch, e.delta);
		break;

	case Sequencer::EventType::REWIND:
		rewind_(ch, e.delta);
		break;

	case Sequencer::EventType::ACTIONS:
		if (ch.state->readActions.load() == true)
			parseActions_(ch, *e.actions, e.delta);
		break;

	default:
		break;
	}
}
} // namespace giada::m::sampleAdvancer