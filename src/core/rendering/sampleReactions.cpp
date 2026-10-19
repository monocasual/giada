/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/rendering/sampleReactions.h"
#include "src/core/actions/ActionManager.h"
#include "src/core/model/channel.h"

namespace giada::m::rendering
{
namespace
{
constexpr int Q_ACTION_PLAY   = 0;
constexpr int Q_ACTION_REWIND = 10000; // Avoid clash with Q_ACTION_PLAY + channelId

/* -------------------------------------------------------------------------- */

void record_(ID channelId, Scene scene, int note, Tick currentTickQuantized, ActionManager& actionRecorder)
{
	actionRecorder.liveRec(channelId, scene, MidiEvent::makeFrom3Bytes(note, 0, 0), currentTickQuantized);
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

ChannelStatus pressWhileOff_(
    ID                                channelId,
    const model::Channel::Parameters& params,
    float                             velocity,
    bool                              canQuantize,
    bool                              velocityAsVol)
{
	/* Reset internal volume to default (1.0) if no velocity as volume. This is
	important in case the channel has actions and some of them have velocity
	less than 1.0: without reset you would play the channel with that velocity
	value. */

	// TODO
	// shared.volumeInternal.store(velocityAsVol ? velocity : 1.0f);

	if (canQuantize)
	{
		// TODO
		// shared.quantizer->trigger(Q_ACTION_PLAY + channelId.getValue());
		// return ChannelStatus::OFF;
	}
	else
		return ChannelStatus::PLAY;
}

/* -------------------------------------------------------------------------- */

ChannelStatus pressWhilePlay_(
    ID                                channelId,
    const model::Channel::Parameters& params,
    const model::Channel::Rendering&  rendering,
    SamplePlayerMode                  mode,
    bool                              canQuantize)
{
	switch (mode)
	{
	case SamplePlayerMode::SINGLE_RETRIG:
		// TODO
		// if (canQuantize)
		//	shared.quantizer->trigger(Q_ACTION_REWIND + channelId.getValue());
		// else
		rewindSampleChannel(rendering, /*localFrame=*/0);
		return ChannelStatus::PLAY;

	case SamplePlayerMode::SINGLE_ENDLESS:
		return ChannelStatus::ENDING;

	case SamplePlayerMode::SINGLE_BASIC:
		stopSampleChannel(rendering, 0);
		return ChannelStatus::PLAY; // Let SamplePlayer stop it once done

	default:
		return ChannelStatus::OFF;
	}
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void recordSampleKeyPress(ID channelId, Scene scene, ChannelShared& shared, Tick currentTickQuantized,
    SamplePlayerMode mode, ActionManager& actionRecorder)
{
	record_(channelId, scene, MidiEvent::CHANNEL_NOTE_ON, currentTickQuantized, actionRecorder);

	/* Skip reading actions when recording on ChannelMode::SINGLE_PRESS to
	prevent	existing actions to interfere with the keypress/keyrel combo. */

	if (mode == SamplePlayerMode::SINGLE_PRESS)
		shared.readActions.store(false);
}

/* -------------------------------------------------------------------------- */

void recordSampleKeyRelease(ID channelId, Scene scene, Tick currentTickQuantized, ActionManager& actionRecorder)
{
	record_(channelId, scene, MidiEvent::CHANNEL_NOTE_OFF, currentTickQuantized, actionRecorder);
}

/* -------------------------------------------------------------------------- */

void recordSampleKeyKill(ID channelId, Scene scene, Tick currentTickQuantized, ActionManager& actionRecorder)
{
	record_(channelId, scene, MidiEvent::CHANNEL_NOTE_KILL, currentTickQuantized, actionRecorder);
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

void stopSampleChannelBySeq(
    const model::Channel::Parameters& params,
    const model::Channel::Rendering&  rendering,
    bool                              chansStopOnSeqHalt,
    bool                              isLoop)
{
	const ChannelStatus playStatus       = params.getPlayStatus();
	const bool          isReadingActions = params.isReadingActions();

	switch (playStatus)
	{
	case ChannelStatus::WAIT:
		/* Loop-mode channels in wait status get stopped right away. */
		if (isLoop)
			params.setPlayStatus(ChannelStatus::OFF);
		break;

	case ChannelStatus::PLAY:
		if (chansStopOnSeqHalt && (isLoop || isReadingActions))
			stopSampleChannel(rendering, 0);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void stopSampleChannel(const model::Channel::Rendering& rendering, Frame localFrame)
{
	rendering.setPendingRenderMode(RenderInfo::Mode::STOP);
}

/* -------------------------------------------------------------------------- */

void pressSampleChannel(
    ID                                channelId,
    const model::Channel::Parameters& params,
    const model::Channel::Rendering&  rendering,
    SamplePlayerMode                  mode,
    float                             velocity,
    bool                              canQuantize,
    bool                              isLoop,
    bool                              velocityAsVol)
{
	ChannelStatus playStatus = params.getPlayStatus();

	switch (playStatus)
	{
	case ChannelStatus::OFF:
		if (isLoop)
			playStatus = ChannelStatus::WAIT;
		else
			playStatus = pressWhileOff_(channelId, params, velocity, canQuantize, velocityAsVol);
		break;

	case ChannelStatus::PLAY:
		if (isLoop)
			playStatus = ChannelStatus::ENDING;
		else
			playStatus = pressWhilePlay_(channelId, params, rendering, mode, canQuantize);
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

	params.setPlayStatus(playStatus);
}

/* -------------------------------------------------------------------------- */

void releaseSampleChannel(
    const model::Channel::Parameters& params,
    const model::Channel::Rendering&  rendering,
    SamplePlayerMode                  mode)
{
	/* Key release is meaningful only for SINGLE_PRESS modes. */

	if (mode != SamplePlayerMode::SINGLE_PRESS)
		return;

	/* Kill it if it's SINGLE_PRESS is playing. Otherwise there might be a
	quantization step in progress that would play the channel later on:
	disable it. */

	if (params.getPlayStatus() == ChannelStatus::PLAY)
		stopSampleChannel(rendering, 0); // Let SamplePlayer stop it once done

	// TODO
	// else if (shared.quantizer->hasBeenTriggered())
	//	shared.quantizer->clear();
}

/* -------------------------------------------------------------------------- */

void killSampleChannel(
    const model::Channel::Parameters& params,
    const model::Channel::Rendering&  rendering,
    SamplePlayerMode                  mode)
{
	const ChannelStatus playStatus = params.getPlayStatus();
	if (playStatus == ChannelStatus::PLAY || playStatus == ChannelStatus::ENDING)
		stopSampleChannel(rendering, 0);
	if (mode == SamplePlayerMode::SINGLE_BASIC_PAUSE)
		params.setTracker(0); // Hard rewind
}

/* -------------------------------------------------------------------------- */

void rewindSampleChannel(const model::Channel::Rendering& rendering, Frame localFrame)
{
	rendering.setPendingRenderMode(RenderInfo::Mode::REWIND);
}

/* -------------------------------------------------------------------------- */

void playSampleChannel(
    const model::Channel::Parameters& params,
    const model::Channel::Rendering&  rendering)
{
	params.setPlayStatus(ChannelStatus::PLAY);
	rendering.setPendingRenderMode(RenderInfo::Mode::NORMAL);
}
} // namespace giada::m::rendering
