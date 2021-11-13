/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "core/channels/sampleActionRecorder.h"
#include "core/channels/channel.h"
#include "core/eventDispatcher.h"
#include "src/core/actions/action.h"
#include "src/core/actions/actionRecorder.h"
#include <cassert>

namespace giada::m
{
SampleActionRecorder::SampleActionRecorder(ActionRecorder& a, Sequencer& s)
: m_actionRecorder(&a)
, m_sequencer(&s)
{
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::react(Channel& ch, const EventDispatcher::Event& e, bool treatRecsAsLoops,
    bool seqIsRunning, bool canRecordActions) const
{
	if (!ch.hasWave())
		return;

	canRecordActions = canRecordActions && !ch.samplePlayer->isAnyLoopMode();

	switch (e.type)
	{
	case EventDispatcher::EventType::KEY_PRESS:
		if (canRecordActions)
			onKeyPress(ch);
		break;

	case EventDispatcher::EventType::KEY_RELEASE:
		/* Record a stop event only if channel is SINGLE_PRESS. For any other 
		mode the key release event is meaningless. */
		if (canRecordActions && ch.samplePlayer->mode == SamplePlayerMode::SINGLE_PRESS)
			record(ch, MidiEvent::NOTE_OFF);
		break;

	case EventDispatcher::EventType::KEY_KILL:
		if (canRecordActions)
			record(ch, MidiEvent::NOTE_KILL);
		break;

	case EventDispatcher::EventType::CHANNEL_TOGGLE_READ_ACTIONS:
		if (ch.hasActions)
			toggleReadActions(ch, treatRecsAsLoops, seqIsRunning);
		break;

	case EventDispatcher::EventType::CHANNEL_KILL_READ_ACTIONS:
		/* Killing Read Actions, i.e. shift + click on 'R' button is meaningful 
		only when the conf::treatRecsAsLoops is true. */
		if (treatRecsAsLoops)
			killReadActions(ch);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::record(Channel& ch, int note) const
{
	m_actionRecorder->liveRec(ch.id, MidiEvent(note, 0, 0), m_sequencer->getCurrentFrameQuantized());
	ch.hasActions = true;
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::onKeyPress(Channel& ch) const
{
	record(ch, MidiEvent::NOTE_ON);

	/* Skip reading actions when recording on ChannelMode::SINGLE_PRESS to 
	prevent	existing actions to interfere with the keypress/keyrel combo. */

	if (ch.samplePlayer->mode == SamplePlayerMode::SINGLE_PRESS)
		ch.state->readActions.store(false);
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::startReadActions(Channel& ch, bool treatRecsAsLoops) const
{
	if (treatRecsAsLoops)
		ch.state->recStatus.store(ChannelStatus::WAIT);
	else
	{
		ch.state->recStatus.store(ChannelStatus::PLAY);
		ch.state->readActions.store(true);
	}
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::stopReadActions(Channel& ch, ChannelStatus curRecStatus,
    bool treatRecsAsLoops, bool seqIsRunning) const
{
	/* First of all, if the sequencer is not running or treatRecsAsLoops is off, 
	just stop and disable everything. Otherwise make sure a channel with actions
	behave like a dynamic one. */

	if (!seqIsRunning || !treatRecsAsLoops)
	{
		ch.state->recStatus.store(ChannelStatus::OFF);
		ch.state->readActions.store(false);
	}
	else if (curRecStatus == ChannelStatus::WAIT)
		ch.state->recStatus.store(ChannelStatus::OFF);
	else if (curRecStatus == ChannelStatus::ENDING)
		ch.state->recStatus.store(ChannelStatus::PLAY);
	else
		ch.state->recStatus.store(ChannelStatus::ENDING);
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::toggleReadActions(Channel& ch, bool treatRecsAsLoops, bool seqIsRunning) const
{
	/* When you start reading actions while conf::treatRecsAsLoops is true, the
	value ch.state->readActions actually is not set to true immediately, because
	the channel is in wait mode (REC_WAITING). readActions will become true on
	the next first beat. So a 'stop rec' command should occur also when
	readActions is false but the channel is in wait mode; this check will
	handle the case of when you press 'R', the channel goes into REC_WAITING and
	then you press 'R' again to undo the status. */

	const bool          readActions = ch.state->readActions.load();
	const ChannelStatus recStatus   = ch.state->recStatus.load();

	if (readActions || (!readActions && recStatus == ChannelStatus::WAIT))
		stopReadActions(ch, recStatus, treatRecsAsLoops, seqIsRunning);
	else
		startReadActions(ch, treatRecsAsLoops);
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::killReadActions(Channel& ch) const
{
	ch.state->recStatus.store(ChannelStatus::OFF);
	ch.state->readActions.store(false);
}
} // namespace giada::m