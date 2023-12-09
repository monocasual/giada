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
#include "utils/math.h"

namespace giada::m::rendering
{
namespace
{
constexpr int Q_ACTION_PLAY   = 0;
constexpr int Q_ACTION_REWIND = 10000; // Avoid clash with Q_ACTION_PLAY + channelId

/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */

ChannelStatus pressWhileOff_(ID channelId, ChannelShared& shared, float velocity,
    bool canQuantize, bool velocityAsVol)
{
	if (velocityAsVol)
		shared.volumeInternal.store(velocity); // No need for mapping: velocityFloat has same range of volume

	if (canQuantize)
	{
		shared.quantizer->trigger(Q_ACTION_PLAY + channelId);
		return ChannelStatus::OFF;
	}
	else
		return ChannelStatus::PLAY;
}

/* -------------------------------------------------------------------------- */

ChannelStatus pressWhilePlay_(ID channelId, ChannelShared& shared, SamplePlayerMode mode,
    bool canQuantize)
{
	switch (mode)
	{
	case SamplePlayerMode::SINGLE_RETRIG:
		if (canQuantize)
			shared.quantizer->trigger(Q_ACTION_REWIND + channelId);
		else
			rewindSampleChannel(shared, /*localFrame=*/0);
		return ChannelStatus::PLAY;

	case SamplePlayerMode::SINGLE_ENDLESS:
		return ChannelStatus::ENDING;

	case SamplePlayerMode::SINGLE_BASIC:
		stopSampleChannel(shared, 0);
		return ChannelStatus::PLAY; // Let SamplePlayer stop it once done

	default:
		return ChannelStatus::OFF;
	}
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

/* -------------------------------------------------------------------------- */

void stopSampleChannelBySeq(ChannelShared& shared, bool chansStopOnSeqHalt, bool isLoop)
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
			stopSampleChannel(shared, 0);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void stopSampleChannel(ChannelShared& shared, Frame localFrame)
{
	shared.renderQueue->push({RenderInfo::Mode::STOP, localFrame});
}

/* -------------------------------------------------------------------------- */

void pressSampleChannel(ID channelId, ChannelShared& shared, SamplePlayerMode mode, float velocity, bool canQuantize, bool isLoop, bool velocityAsVol)
{
	ChannelStatus playStatus = shared.playStatus.load();

	switch (playStatus)
	{
	case ChannelStatus::OFF:
		if (isLoop)
			playStatus = ChannelStatus::WAIT;
		else
			playStatus = pressWhileOff_(channelId, shared, velocity, canQuantize, velocityAsVol);
		break;

	case ChannelStatus::PLAY:
		if (isLoop)
			playStatus = ChannelStatus::ENDING;
		else
			playStatus = pressWhilePlay_(channelId, shared, mode, canQuantize);
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

void releaseSampleChannel(ChannelShared& shared, SamplePlayerMode mode)
{
	/* Key release is meaningful only for SINGLE_PRESS modes. */

	if (mode != SamplePlayerMode::SINGLE_PRESS)
		return;

	/* Kill it if it's SINGLE_PRESS is playing. Otherwise there might be a 
	quantization step in progress that would play the channel later on: 
	disable it. */

	if (shared.playStatus.load() == ChannelStatus::PLAY)
		stopSampleChannel(shared, 0); // Let SamplePlayer stop it once done
	else if (shared.quantizer->hasBeenTriggered())
		shared.quantizer->clear();
}

/* -------------------------------------------------------------------------- */

void killSampleChannel(ChannelShared& shared, SamplePlayerMode mode)
{
	const ChannelStatus playStatus = shared.playStatus.load();
	if (playStatus == ChannelStatus::PLAY || playStatus == ChannelStatus::ENDING)
		stopSampleChannel(shared, 0);
	if (mode == SamplePlayerMode::SINGLE_BASIC_PAUSE)
		shared.tracker.store(0); // Hard rewind
}

/* -------------------------------------------------------------------------- */

void rewindSampleChannel(ChannelShared& shared, Frame localFrame)
{
	shared.renderQueue->push({RenderInfo::Mode::REWIND, localFrame});
}

/* -------------------------------------------------------------------------- */

void playSampleChannel(ChannelShared& shared, Frame localFrame)
{
	shared.playStatus.store(ChannelStatus::PLAY);
	shared.renderQueue->push({RenderInfo::Mode::NORMAL, localFrame});
}
} // namespace giada::m::rendering
