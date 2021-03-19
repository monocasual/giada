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

#include "sampleActionRecorder.h"
#include "core/action.h"
#include "core/channels/channel.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/eventDispatcher.h"
#include "core/mixer.h"
#include "core/recManager.h"
#include "core/recorderHandler.h"
#include <cassert>

namespace giada::m::sampleActionRecorder
{
namespace
{
void record_(channel::Data& ch, int note);
void onKeyPress_(channel::Data& ch);
void toggleReadActions_(channel::Data& ch);
void startReadActions_(channel::Data& ch);
void stopReadActions_(channel::Data& ch, ChannelStatus curRecStatus);
void killReadActions_(channel::Data& ch);
bool canRecord_(const channel::Data& ch);

/* -------------------------------------------------------------------------- */

bool canRecord_(const channel::Data& ch)
{
	return recManager::isRecordingAction() &&
	       clock::isRunning() &&
	       !recManager::isRecordingInput() &&
	       !ch.samplePlayer->isAnyLoopMode();
}

/* -------------------------------------------------------------------------- */

void onKeyPress_(channel::Data& ch)
{
	if (!canRecord_(ch))
		return;
	record_(ch, MidiEvent::NOTE_ON);

	/* Skip reading actions when recording on ChannelMode::SINGLE_PRESS to 
	prevent	existing actions to interfere with the keypress/keyrel combo. */

	if (ch.samplePlayer->mode == SamplePlayerMode::SINGLE_PRESS)
		ch.readActions = false;
}

/* -------------------------------------------------------------------------- */

void record_(channel::Data& ch, int note)
{
	recorderHandler::liveRec(ch.id, MidiEvent(note, 0, 0),
	    clock::quantize(clock::getCurrentFrame()));

	ch.hasActions = true;
}

/* -------------------------------------------------------------------------- */

void toggleReadActions_(channel::Data& ch)
{
	/* When you start reading actions while conf::treatRecsAsLoops is true, the
	value ch.state->readActions actually is not set to true immediately, because
	the channel is in wait mode (REC_WAITING). readActions will become true on
	the next first beat. So a 'stop rec' command should occur also when
	readActions is false but the channel is in wait mode; this check will
	handle the case of when you press 'R', the channel goes into REC_WAITING and
	then you press 'R' again to undo the status. */

	if (!ch.hasActions)
		return;

	bool          readActions = ch.readActions;
	ChannelStatus recStatus   = ch.state->recStatus.load();

	if (readActions || (!readActions && recStatus == ChannelStatus::WAIT))
		stopReadActions_(ch, recStatus);
	else
		startReadActions_(ch);
}

/* -------------------------------------------------------------------------- */

void startReadActions_(channel::Data& ch)
{
	if (conf::conf.treatRecsAsLoops)
		ch.state->recStatus.store(ChannelStatus::WAIT);
	else
	{
		ch.state->recStatus.store(ChannelStatus::PLAY);
		ch.readActions = true;
	}
}

/* -------------------------------------------------------------------------- */

void stopReadActions_(channel::Data& ch, ChannelStatus curRecStatus)
{
	/* First of all, if the clock is not running or treatRecsAsLoops is off, 
	just stop and disable everything. Otherwise make sure a channel with actions
	behave like a dynamic one. */

	if (!clock::isRunning() || !conf::conf.treatRecsAsLoops)
	{
		ch.state->recStatus.store(ChannelStatus::OFF);
		ch.readActions = false;
	}
	else if (curRecStatus == ChannelStatus::WAIT)
		ch.state->recStatus.store(ChannelStatus::OFF);
	else if (curRecStatus == ChannelStatus::ENDING)
		ch.state->recStatus.store(ChannelStatus::PLAY);
	else
		ch.state->recStatus.store(ChannelStatus::ENDING);
}

/* -------------------------------------------------------------------------- */

void killReadActions_(channel::Data& ch)
{
	/* Killing Read Actions, i.e. shift + click on 'R' button is meaninful only 
	when the conf::treatRecsAsLoops is true. */

	if (!conf::conf.treatRecsAsLoops)
		return;
	ch.state->recStatus.store(ChannelStatus::OFF);
	ch.readActions = false;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void react(channel::Data& ch, const eventDispatcher::Event& e)
{
	if (!ch.hasWave())
		return;

	switch (e.type)
	{

	case eventDispatcher::EventType::KEY_PRESS:
		onKeyPress_(ch);
		break;

		/* Record a stop event only if channel is SINGLE_PRESS. For any other 
		mode the key release event is meaningless. */

	case eventDispatcher::EventType::KEY_RELEASE:
		if (canRecord_(ch) && ch.samplePlayer->mode == SamplePlayerMode::SINGLE_PRESS)
			record_(ch, MidiEvent::NOTE_OFF);
		break;

	case eventDispatcher::EventType::KEY_KILL:
		if (canRecord_(ch))
			record_(ch, MidiEvent::NOTE_KILL);
		break;

	case eventDispatcher::EventType::CHANNEL_TOGGLE_READ_ACTIONS:
		toggleReadActions_(ch);
		break;

	case eventDispatcher::EventType::CHANNEL_KILL_READ_ACTIONS:
		killReadActions_(ch);
		break;

	default:
		break;
	}
}
} // namespace giada::m::sampleActionRecorder