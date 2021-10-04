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

#include "core/recorder.h"
#include "core/mixerHandler.h"
#include "core/model/model.h"
#include "core/sequencer.h"
#include "core/types.h"
#include "src/core/actions/actionRecorder.h"
#include "src/core/actions/actions.h"

namespace giada::m
{
Recorder::Recorder(model::Model& m, Sequencer& s, MixerHandler& mh)
: m_model(m)
, m_sequencer(s)
, m_mixerHandler(mh)
{
}

/* -------------------------------------------------------------------------- */

bool Recorder::isRecording() const
{
	return isRecordingAction() || isRecordingInput();
}

bool Recorder::isRecordingAction() const
{
	return m_model.get().recorder.a_isRecordingAction();
}

bool Recorder::isRecordingInput() const
{
	return m_model.get().recorder.a_isRecordingInput();
}

/* -------------------------------------------------------------------------- */

void Recorder::prepareActionRec(RecTriggerMode mode)
{
	if (mode == RecTriggerMode::NORMAL)
	{
		startActionRec();
		m_sequencer.setStatus(SeqStatus::RUNNING);
		G_DEBUG("Start action rec, NORMAL mode");
	}
	else
	{ // RecTriggerMode::SIGNAL
		m_sequencer.setStatus(SeqStatus::WAITING);
		G_DEBUG("Start action rec, SIGNAL mode (waiting for signal from Midi Dispatcher...)");
	}
}

/* -------------------------------------------------------------------------- */

void Recorder::stopActionRec(ActionRecorder& actionRecorder)
{
	setRecordingAction(false);

	/* If you stop the Action Recorder in SIGNAL mode before any actual 
	recording: just clean up everything and return. */

	if (m_sequencer.getStatus() == SeqStatus::WAITING)
	{
		m_sequencer.setStatus(SeqStatus::STOPPED);
		return;
	}

	std::unordered_set<ID> channels = actionRecorder.consolidate();

	/* Enable reading actions for Channels that have just been filled with 
	actions. Start reading right away, without checking whether 
	conf::treatRecsAsLoops is enabled or not. Same thing for MIDI channels.  */

	for (ID id : channels)
	{
		channel::Data& ch = m_model.get().getChannel(id);
		ch.state->readActions.store(true);
		ch.state->recStatus.store(ChannelStatus::PLAY);
		if (ch.type == ChannelType::MIDI)
			ch.state->playStatus.store(ChannelStatus::PLAY);
	}
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

bool Recorder::prepareInputRec(RecTriggerMode triggerMode, InputRecMode inputMode)
{
	if (inputMode == InputRecMode::FREE)
		m_sequencer.rewind();

	if (triggerMode == RecTriggerMode::NORMAL)
	{
		startInputRec();
		m_sequencer.setStatus(SeqStatus::RUNNING);
		G_DEBUG("Start input rec, NORMAL mode");
	}
	else
	{
		m_sequencer.setStatus(SeqStatus::WAITING);
		G_DEBUG("Start input rec, SIGNAL mode (waiting for signal from Mixer...)");
	}

	return true;
}

/* -------------------------------------------------------------------------- */

void Recorder::stopInputRec(InputRecMode recMode, int sampleRate)
{
	setRecordingInput(false);

	Frame recordedFrames = m_mixerHandler.stopInputRec();

	/* When recording in RIGID mode, the amount of recorded frames is always 
	equal to the current loop length. */

	if (recMode == InputRecMode::RIGID)
		recordedFrames = m_sequencer.getFramesInLoop();

	G_DEBUG("Stop input rec, recordedFrames=" << recordedFrames);

	/* If you stop the Input Recorder in SIGNAL mode before any actual 
	recording: just clean up everything and return. */

	if (m_sequencer.getStatus() == SeqStatus::WAITING)
	{
		m_sequencer.setStatus(SeqStatus::STOPPED);
		return;
	}

	/* Finalize recordings. InputRecMode::FREE requires some adjustments. */

	m_mixerHandler.finalizeInputRec(recordedFrames, m_sequencer.getCurrentFrame());

	if (recMode == InputRecMode::FREE)
	{
		m_sequencer.rewind();
		m_sequencer.setBpm(m_sequencer.calcBpmFromRec(recordedFrames, sampleRate), sampleRate);
	}
}

/* -------------------------------------------------------------------------- */

bool Recorder::canEnableRecOnSignal() const { return !m_sequencer.isRunning(); }
bool Recorder::canEnableFreeInputRec() const { return !m_mixerHandler.hasAudioData(); }

/* -------------------------------------------------------------------------- */

bool Recorder::canRecordActions() const
{
	return isRecordingAction() && m_sequencer.isRunning() && !isRecordingInput();
}

/* -------------------------------------------------------------------------- */

void Recorder::setRecordingAction(bool v)
{
	m_model.get().recorder.a_setRecordingAction(v);
}

void Recorder::setRecordingInput(bool v)
{
	m_model.get().recorder.a_setRecordingInput(v);
}

/* -------------------------------------------------------------------------- */

void Recorder::startActionRec()
{
	setRecordingAction(true);
}

/* -------------------------------------------------------------------------- */

void Recorder::startActionRecOnCallback()
{
	if (m_sequencer.getStatus() != SeqStatus::WAITING)
		return;
	startActionRec();
	m_sequencer.setStatus(SeqStatus::RUNNING);
}

/* -------------------------------------------------------------------------- */

void Recorder::startInputRec()
{
	/* Start recording from the current frame, not the beginning. */
	m_mixerHandler.startInputRec(m_sequencer.getCurrentFrame());
	setRecordingInput(true);
}

/* -------------------------------------------------------------------------- */

void Recorder::startInputRecOnCallback()
{
	if (m_sequencer.getStatus() != SeqStatus::WAITING)
		return;
	startInputRec();
	m_sequencer.setStatus(SeqStatus::RUNNING);
}
} // namespace giada::m