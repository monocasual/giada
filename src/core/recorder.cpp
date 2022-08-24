/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "core/channels/channelManager.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/sequencer.h"
#include "core/types.h"
#include "src/core/actions/actionRecorder.h"
#include "src/core/actions/actions.h"
#include "utils/log.h"

namespace giada::m
{
Recorder::Recorder(model::Model& m, Sequencer& s, ChannelManager& cm, Mixer& mx, ActionRecorder& a)
: m_model(m)
, m_sequencer(s)
, m_channelManager(cm)
, m_mixer(mx)
, m_actionRecorder(a)
{
}

/* -------------------------------------------------------------------------- */

bool Recorder::isRecordingActions() const
{
	return m_isRecordingActions.load();
}

bool Recorder::isRecordingInput() const
{
	return m_isRecordingInput.load();
}

/* -------------------------------------------------------------------------- */

void Recorder::react(const EventDispatcher::EventBuffer& events, int sampleRate)
{
	for (const EventDispatcher::Event& e : events)
	{
		switch (e.type)
		{
		case EventDispatcher::EventType::RECORDER_PREPARE_ACTION_REC:
		{
			const RecTriggerMode mode = std::any_cast<RecTriggerMode>(e.data);
			prepareActionRec(mode);
			break;
		}
		case EventDispatcher::EventType::RECORDER_PREPARE_INPUT_REC:
		{
			const InputRecData data = std::any_cast<InputRecData>(e.data);
			prepareInputRec(data.recTriggerMode, data.inputMode);
			break;
		}
		case EventDispatcher::EventType::RECORDER_STOP_ACTION_REC:
		{
			if (isRecordingActions())
				stopActionRec();
			break;
		}
		case EventDispatcher::EventType::RECORDER_STOP_INPUT_REC:
		{
			if (!isRecordingInput())
				return;
			const InputRecMode mode = std::any_cast<InputRecMode>(e.data);
			stopInputRec(mode, sampleRate);
			break;
		}
		default:
			break;
		}
	}
}

/* -------------------------------------------------------------------------- */

void Recorder::prepareActionRec(RecTriggerMode mode)
{
	if (mode == RecTriggerMode::NORMAL)
	{
		m_isRecordingActions.store(true);
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
	m_isRecordingActions.store(false);

	/* If you stop the Action Recorder in SIGNAL mode before any actual 
	recording: just clean up everything and return. */

	if (m_sequencer.getStatus() == SeqStatus::WAITING)
	{
		m_sequencer.setStatus(SeqStatus::STOPPED);
		return;
	}

	std::unordered_set<ID> channels = m_actionRecorder.consolidate();

	/* Enable reading actions for Channels that have just been filled with 
	actions. Start reading right away, without checking whether 
	conf::treatRecsAsLoops is enabled or not. Same thing for MIDI channels.  */

	for (ID id : channels)
	{
		Channel& ch = m_model.get().getChannel(id);
		ch.shared->readActions.store(true);
		ch.shared->recStatus.store(ChannelStatus::PLAY);
		if (ch.type == ChannelType::MIDI)
			ch.shared->playStatus.store(ChannelStatus::PLAY);
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

void Recorder::stopInputRec(InputRecMode recMode, int sampleRate)
{
	m_isRecordingInput.store(false);

	Frame recordedFrames = m_mixer.stopInputRec();

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

	m_channelManager.finalizeInputRec(m_mixer.getRecBuffer(), recordedFrames, m_sequencer.getCurrentFrame());
	m_mixer.clearRecBuffer();

	if (recMode == InputRecMode::FREE)
	{
		m_sequencer.rewind();
		m_sequencer.setBpm(m_sequencer.calcBpmFromRec(recordedFrames, sampleRate), sampleRate);
	}
}

/* -------------------------------------------------------------------------- */

bool Recorder::canEnableRecOnSignal() const { return !m_sequencer.isRunning(); }
bool Recorder::canEnableFreeInputRec() const { return !m_channelManager.hasAudioData(); }

/* -------------------------------------------------------------------------- */

bool Recorder::canRecordActions() const
{
	return isRecordingActions() && m_sequencer.isRunning() && !isRecordingInput();
}

/* -------------------------------------------------------------------------- */

void Recorder::startActionRecOnCallback()
{
	if (m_sequencer.getStatus() != SeqStatus::WAITING)
		return;
	m_isRecordingActions.store(true);
	m_sequencer.setStatus(SeqStatus::RUNNING);
}

/* -------------------------------------------------------------------------- */

void Recorder::startInputRec()
{
	/* Start recording from the current frame, not the beginning. */
	m_mixer.startInputRec(m_sequencer.getCurrentFrame());
	m_isRecordingInput.store(true);
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