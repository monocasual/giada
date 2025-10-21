/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/recorder.h"
#include "src/core/actions/actionRecorder.h"
#include "src/core/channels/channelManager.h"
#include "src/core/mixer.h"
#include "src/core/model/actions.h"
#include "src/core/model/model.h"
#include "src/core/sequencer.h"
#include "src/core/types.h"
#include "src/utils/log.h"

namespace giada::m
{
Recorder::Recorder(Sequencer& s, ChannelManager& cm, Mixer& mx, ActionRecorder& a)
: m_sequencer(s)
, m_channelManager(cm)
, m_mixer(mx)
, m_actionRecorder(a)
{
}

/* -------------------------------------------------------------------------- */

void Recorder::prepareActionRec(RecTriggerMode mode)
{
	if (mode == RecTriggerMode::NORMAL)
	{
		m_mixer.startActionRec();
		m_sequencer.setStatus(SeqStatus::RUNNING);
		G_DEBUG("Start action rec, NORMAL mode", );
	}
	else
	{ // RecTriggerMode::SIGNAL
		m_sequencer.setStatus(SeqStatus::WAITING);
		G_DEBUG("Start action rec, SIGNAL mode (waiting for signal from Midi Dispatcher...)", );
	}
}

/* -------------------------------------------------------------------------- */

void Recorder::stopActionRec()
{
	if (!m_mixer.isRecordingActions())
		return;

	m_mixer.stopActionRec();

	/* Restore record trigger mode to normal in case you want to record again
	while the sequencer is running - if in SIGNAL mode, the sequencer would
	pause otherwise (see https://github.com/monocasual/giada/issues/678). */

	if (m_mixer.getRecTriggerMode() == RecTriggerMode::SIGNAL && m_sequencer.getStatus() == SeqStatus::RUNNING)
		m_mixer.setRecTriggerMode(RecTriggerMode::NORMAL);

	/* If you stop the Action Recorder in SIGNAL mode before any actual
	recording: just clean up everything and return. */

	if (m_sequencer.getStatus() == SeqStatus::WAITING)
	{
		m_sequencer.setStatus(SeqStatus::STOPPED);
		return;
	}

	/* Enable reading actions for Channels that have just been filled with
	actions. This will start reading right away, without checking whether
	conf::treatRecsAsLoops is enabled or not. Same thing for MIDI channels.  */

	m_channelManager.finalizeActionRec(m_actionRecorder.consolidate(m_sequencer.getCurrentScene()));
}

/* -------------------------------------------------------------------------- */

void Recorder::toggleActionRec()
{
	if (m_mixer.isRecordingActions())
		stopActionRec();
	else if (m_sequencer.getStatus() == SeqStatus::WAITING)
		m_sequencer.setStatus(SeqStatus::STOPPED);
	else
		prepareActionRec(m_mixer.getRecTriggerMode());
}

/* -------------------------------------------------------------------------- */

bool Recorder::prepareInputRec(RecTriggerMode triggerMode, InputRecMode inputMode)
{
	if (inputMode == InputRecMode::FREE)
		m_sequencer.rewindForced();

	if (triggerMode == RecTriggerMode::NORMAL)
	{
		startInputRec();
		m_sequencer.setStatus(SeqStatus::RUNNING);
		G_DEBUG("Start input rec, NORMAL mode", );
	}
	else
	{
		m_sequencer.setStatus(SeqStatus::WAITING);
		G_DEBUG("Start input rec, SIGNAL mode (waiting for signal from Mixer...)", );
	}

	return true;
}

/* -------------------------------------------------------------------------- */

void Recorder::stopInputRec(int sampleRate)
{
	if (!m_mixer.isRecordingInput())
		return;

	const RecTriggerMode recTriggerMode = m_mixer.getRecTriggerMode();
	const InputRecMode   recMode        = m_mixer.getInputRecMode();
	Frame                recordedFrames = m_mixer.stopInputRec();
	const std::size_t    scene          = m_sequencer.getCurrentScene();

	/* Restore record trigger mode to normal in case you want to record again
	while the sequencer is running - if in SIGNAL mode, the sequencer would
	pause otherwise (see https://github.com/monocasual/giada/issues/678). */

	if (recTriggerMode == RecTriggerMode::SIGNAL && m_sequencer.getStatus() == SeqStatus::RUNNING)
		m_mixer.setRecTriggerMode(RecTriggerMode::NORMAL);

	/* When recording in RIGID mode, the amount of recorded frames is always
	equal to the current loop length. */

	if (recMode == InputRecMode::RIGID)
		recordedFrames = m_sequencer.getFramesInLoop();

	G_DEBUG("Stop input rec, recordedFrames={}", recordedFrames);

	/* If you stop the Input Recorder in SIGNAL mode before any actual
	recording: just clean up everything and return. */

	if (m_sequencer.getStatus() == SeqStatus::WAITING)
	{
		m_sequencer.setStatus(SeqStatus::STOPPED);
		return;
	}

	/* Finalize recordings. InputRecMode::FREE requires some adjustments. */

	m_channelManager.finalizeInputRec(m_mixer.getRecBuffer(), recordedFrames, m_sequencer.getCurrentFrame(), scene);
	m_mixer.clearRecBuffer();

	if (recMode == InputRecMode::FREE)
	{
		m_sequencer.rewindForced();
		m_sequencer.setBpm(m_sequencer.calcBpmFromRec(recordedFrames, sampleRate), sampleRate);
	}
}

/* -------------------------------------------------------------------------- */

void Recorder::toggleInputRec(int sampleRate)
{
	if (!m_channelManager.hasInputRecordableChannels(m_sequencer.getCurrentScene()))
		return;
	if (m_mixer.isRecordingInput())
		stopInputRec(sampleRate);
	else if (m_sequencer.getStatus() == SeqStatus::WAITING)
		m_sequencer.setStatus(SeqStatus::STOPPED);
	else
		prepareInputRec(m_mixer.getRecTriggerMode(), m_mixer.getInputRecMode());
}

/* -------------------------------------------------------------------------- */

void Recorder::toggleRecOnSignal()
{
	const RecTriggerMode currentMode = m_mixer.getRecTriggerMode();
	RecTriggerMode       newMode     = currentMode == RecTriggerMode::NORMAL ? RecTriggerMode::SIGNAL : RecTriggerMode::NORMAL;

	if (!canEnableRecOnSignal())
		newMode = RecTriggerMode::NORMAL;

	m_mixer.setRecTriggerMode(newMode);
}

/* -------------------------------------------------------------------------- */

void Recorder::toggleFreeInputRec()
{
	const InputRecMode currentMode = m_mixer.getInputRecMode();
	InputRecMode       newMode     = currentMode == InputRecMode::RIGID ? InputRecMode::FREE : InputRecMode::RIGID;

	if (!canEnableFreeInputRec())
		newMode = InputRecMode::RIGID;

	m_mixer.setInputRecMode(newMode);
}

/* -------------------------------------------------------------------------- */

bool Recorder::canEnableRecOnSignal() const { return !m_sequencer.isRunning(); }
bool Recorder::canEnableFreeInputRec() const { return !m_channelManager.hasAudioData(); }

/* -------------------------------------------------------------------------- */

bool Recorder::canRecordActions() const
{
	return m_mixer.isRecordingActions() && m_sequencer.isRunning() && !m_mixer.isRecordingInput();
}

/* -------------------------------------------------------------------------- */

void Recorder::startActionRecOnCallback()
{
	if (m_sequencer.getStatus() != SeqStatus::WAITING)
		return;
	m_mixer.startActionRec();
	m_sequencer.setStatus(SeqStatus::RUNNING);
}

/* -------------------------------------------------------------------------- */

void Recorder::startInputRec()
{
	/* Start recording from the current frame, not the beginning. */
	m_mixer.startInputRec(m_sequencer.getCurrentFrame());
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