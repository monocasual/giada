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

#include "core/recManager.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/kernelAudio.h"
#include "core/midiDispatcher.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/model/model.h"
#include "core/recorder.h"
#include "core/recorderHandler.h"
#include "core/sequencer.h"
#include "core/types.h"
#include "gui/dispatcher.h"

namespace giada::m::recManager
{
namespace
{
bool canRec_()
{
	return kernelAudio::isReady();
}

bool canInputRec_()
{
	return canRec_() && mh::hasInputRecordableChannels();
}

/* -------------------------------------------------------------------------- */

void setRecordingAction_(bool v)
{
	model::get().recorder.isRecordingAction = v;
	model::swap(model::SwapType::NONE);
}

void setRecordingInput_(bool v)
{
	model::get().recorder.isRecordingInput = v;
	model::swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

bool startActionRec_()
{
	clock::setStatus(ClockStatus::RUNNING);
	sequencer::start();
	conf::conf.recTriggerMode = RecTriggerMode::NORMAL;
	return true;
}

/* -------------------------------------------------------------------------- */

void startInputRec_()
{
	/* Start recording from the current frame, not the beginning. */
	mixer::startInputRec(clock::getCurrentFrame());
	sequencer::start();
	conf::conf.recTriggerMode = RecTriggerMode::NORMAL;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

bool isRecording()
{
	return isRecordingAction() || isRecordingInput();
}

bool isRecordingAction()
{
	return model::get().recorder.isRecordingAction;
}

bool isRecordingInput()
{
	return model::get().recorder.isRecordingInput;
}

/* -------------------------------------------------------------------------- */

void startActionRec(RecTriggerMode mode)
{
	if (!canRec_())
		return;

	if (mode == RecTriggerMode::NORMAL)
	{
		startActionRec_();
		setRecordingAction_(true);
	}
	else
	{ // RecTriggerMode::SIGNAL
		clock::setStatus(ClockStatus::WAITING);
		clock::rewind();
		midiDispatcher::setSignalCallback(startActionRec_);
		v::dispatcher::setSignalCallback(startActionRec_);
		setRecordingAction_(true);
	}
}

/* -------------------------------------------------------------------------- */

void stopActionRec()
{
	setRecordingAction_(false);

	/* If you stop the Action Recorder in SIGNAL mode before any actual 
	recording: just clean up everything and return. */

	if (clock::getStatus() == ClockStatus::WAITING)
	{
		clock::setStatus(ClockStatus::STOPPED);
		midiDispatcher::setSignalCallback(nullptr);
		v::dispatcher::setSignalCallback(nullptr);
		return;
	}

	std::unordered_set<ID> channels = recorderHandler::consolidate();

	/* Enable reading actions for Channels that have just been filled with 
	actions. Start reading right away, without checking whether 
	conf::treatRecsAsLoops is enabled or not. Same thing for MIDI channels.  */

	for (ID id : channels)
	{
		channel::Data& ch = model::get().getChannel(id);
		ch.readActions    = true;
		if (ch.type == ChannelType::MIDI)
			ch.state->playStatus.store(ChannelStatus::PLAY);
	}
	model::swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void toggleActionRec(RecTriggerMode m)
{
	isRecordingAction() ? stopActionRec() : startActionRec(m);
}

/* -------------------------------------------------------------------------- */

bool startInputRec(RecTriggerMode triggerMode, InputRecMode inputMode)
{
	if (!canInputRec_())
		return false;

	if (triggerMode == RecTriggerMode::SIGNAL || inputMode == InputRecMode::FREE)
		clock::rewind();

	if (inputMode == InputRecMode::FREE)
		mixer::setEndOfRecCallback([inputMode] { stopInputRec(inputMode); });

	if (triggerMode == RecTriggerMode::NORMAL)
	{
		startInputRec_();
		setRecordingInput_(true);
		G_DEBUG("Start input rec, NORMAL mode");
	}
	else
	{
		clock::setStatus(ClockStatus::WAITING);
		mixer::setSignalCallback([] {
			startInputRec_();
			setRecordingInput_(true);
		});
		G_DEBUG("Start input rec, SIGNAL mode");
	}

	return true;
}

/* -------------------------------------------------------------------------- */

void stopInputRec(InputRecMode recMode)
{
	setRecordingInput_(false);

	Frame recordedFrames = mixer::stopInputRec();

	/* When recording in RIGID mode, the amount of recorded frames is always 
	equal to the current loop length. */

	if (recMode == InputRecMode::RIGID)
		recordedFrames = clock::getFramesInLoop();

	G_DEBUG("Stop input rec, recordedFrames=" << recordedFrames);

	/* If you stop the Input Recorder in SIGNAL mode before any actual 
	recording: just clean up everything and return. */

	if (clock::getStatus() == ClockStatus::WAITING)
	{
		clock::setStatus(ClockStatus::STOPPED);
		mixer::setSignalCallback(nullptr);
		return;
	}

	/* Finalize recordings. InputRecMode::FREE requires some adjustments. */

	mh::finalizeInputRec(recordedFrames);

	if (recMode == InputRecMode::FREE)
	{
		clock::rewind();
		clock::setBpm(clock::calcBpmFromRec(recordedFrames));
		mixer::setEndOfRecCallback(nullptr);
		refreshInputRecMode();                       // Back to RIGID mode if necessary
		model::triggerSwapCb(model::SwapType::HARD); // Rebuild UI to show new BPM value
	}
}

/* -------------------------------------------------------------------------- */

bool toggleInputRec(RecTriggerMode m, InputRecMode i)
{
	if (isRecordingInput())
	{
		stopInputRec(i);
		return true;
	}
	return startInputRec(m, i);
}

/* -------------------------------------------------------------------------- */

bool canEnableRecOnSignal() { return !clock::isRunning(); }
bool canEnableFreeInputRec() { return !mh::hasAudioData(); }

void refreshInputRecMode()
{
	if (!canEnableFreeInputRec())
		conf::conf.inputRecMode = InputRecMode::RIGID;
}
} // namespace giada::m::recManager