/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "core/channels/channelShared.h"
#include "core/eventDispatcher.h"
#include "src/core/actions/action.h"
#include "src/core/actions/actionRecorder.h"
#include <cassert>

namespace giada::m
{
SampleActionRecorder::SampleActionRecorder(ActionRecorder& a)
: m_actionRecorder(&a)
{
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::record(ID channelId, int note, Frame currentFrameQuantized, bool& hasActions) const
{
	m_actionRecorder->liveRec(channelId, MidiEvent::makeFrom3Bytes(note, 0, 0), currentFrameQuantized);
	hasActions = true;
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::keyPress(ID channelId, ChannelShared& shared,
    Frame currentFrameQuantized, SamplePlayerMode mode, bool& hasActions) const
{
	record(channelId, MidiEvent::CHANNEL_NOTE_ON, currentFrameQuantized, hasActions);

	/* Skip reading actions when recording on ChannelMode::SINGLE_PRESS to 
	prevent	existing actions to interfere with the keypress/keyrel combo. */

	if (mode == SamplePlayerMode::SINGLE_PRESS)
		shared.readActions.store(false);
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::keyRelease(ID channelId, Frame currentFrameQuantized, bool& hasActions) const
{
	record(channelId, MidiEvent::CHANNEL_NOTE_OFF, currentFrameQuantized, hasActions);
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::keyKill(ID channelId, Frame currentFrameQuantized, bool& hasActions) const
{
	record(channelId, MidiEvent::CHANNEL_NOTE_KILL, currentFrameQuantized, hasActions);
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::startReadActions(ChannelShared& shared, bool treatRecsAsLoops) const
{
	if (treatRecsAsLoops)
		shared.recStatus.store(ChannelStatus::WAIT);
	else
	{
		shared.recStatus.store(ChannelStatus::PLAY);
		shared.readActions.store(true);
	}
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::stopReadActions(ChannelShared& shared, ChannelStatus curRecStatus,
    bool treatRecsAsLoops, bool seqIsRunning) const
{
	/* First of all, if the sequencer is not running or treatRecsAsLoops is off, 
	just stop and disable everything. Otherwise make sure a channel with actions
	behave like a dynamic one. */

	if (!seqIsRunning || !treatRecsAsLoops)
	{
		shared.recStatus.store(ChannelStatus::OFF);
		shared.readActions.store(false);
	}
	else if (curRecStatus == ChannelStatus::WAIT)
		shared.recStatus.store(ChannelStatus::OFF);
	else if (curRecStatus == ChannelStatus::ENDING)
		shared.recStatus.store(ChannelStatus::PLAY);
	else
		shared.recStatus.store(ChannelStatus::ENDING);
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::toggleReadActions(ChannelShared& shared, bool treatRecsAsLoops, bool seqIsRunning) const
{
	/* When you start reading actions while conf::treatRecsAsLoops is true, the
	value ch.shared->readActions actually is not set to true immediately, because
	the channel is in wait mode (REC_WAITING). readActions will become true on
	the next first beat. So a 'stop rec' command should occur also when
	readActions is false but the channel is in wait mode; this check will
	handle the case of when you press 'R', the channel goes into REC_WAITING and
	then you press 'R' again to undo the status. */

	const bool          readActions = shared.readActions.load();
	const ChannelStatus recStatus   = shared.recStatus.load();

	if (readActions || (!readActions && recStatus == ChannelStatus::WAIT))
		stopReadActions(shared, recStatus, treatRecsAsLoops, seqIsRunning);
	else
		startReadActions(shared, treatRecsAsLoops);
}

/* -------------------------------------------------------------------------- */

void SampleActionRecorder::killReadActions(ChannelShared& shared) const
{
	shared.recStatus.store(ChannelStatus::OFF);
	shared.readActions.store(false);
}
} // namespace giada::m