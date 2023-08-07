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

#include "core/rendering/sampleReactions.h"
#include "core/actions/actionRecorder.h"

namespace giada::m::rendering
{
namespace
{
void record_(ID channelId, int note, Frame currentFrameQuantized, ActionRecorder& actionRecorder)
{
	actionRecorder.liveRec(channelId, MidiEvent::makeFrom3Bytes(note, 0, 0), currentFrameQuantized);
}
/* -------------------------------------------------------------------------- */

void startReadActions_(ChannelShared& shared, bool treatRecsAsLoops)
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

void stopReadActions_(ChannelShared& shared, ChannelStatus curRecStatus,
    bool treatRecsAsLoops, bool seqIsRunning)
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

} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void recordSampleKeyPress(ID channelId, ChannelShared& shared, Frame currentFrameQuantized, SamplePlayerMode mode, ActionRecorder& actionRecorder)
{
	record_(channelId, MidiEvent::CHANNEL_NOTE_ON, currentFrameQuantized, actionRecorder);

	/* Skip reading actions when recording on ChannelMode::SINGLE_PRESS to 
	prevent	existing actions to interfere with the keypress/keyrel combo. */

	if (mode == SamplePlayerMode::SINGLE_PRESS)
		shared.readActions.store(false);
}

/* -------------------------------------------------------------------------- */

void recordSampleKeyRelease(ID channelId, Frame currentFrameQuantized, ActionRecorder& actionRecorder)
{
	record_(channelId, MidiEvent::CHANNEL_NOTE_OFF, currentFrameQuantized, actionRecorder);
}

/* -------------------------------------------------------------------------- */

void recordSampleKeyKill(ID channelId, Frame currentFrameQuantized, ActionRecorder& actionRecorder)
{
	record_(channelId, MidiEvent::CHANNEL_NOTE_KILL, currentFrameQuantized, actionRecorder);
}

/* -------------------------------------------------------------------------- */

void killSampleReadActions(ChannelShared& shared)
{
	shared.recStatus.store(ChannelStatus::OFF);
	shared.readActions.store(false);
}

/* -------------------------------------------------------------------------- */

void toggleSampleReadActions(ChannelShared& shared, bool treatRecsAsLoops, bool seqIsRunning)
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
		stopReadActions_(shared, recStatus, treatRecsAsLoops, seqIsRunning);
	else
		startReadActions_(shared, treatRecsAsLoops);
}
} // namespace giada::m::rendering
