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

#include "core/mainEngine.h"
#include "core/channels/channelManager.h"
#include "core/engine.h"
#include "core/kernelAudio.h"
#include "core/midiSynchronizer.h"
#include "core/mixer.h"

namespace giada::m
{
MainEngine::MainEngine(Engine& e, KernelAudio& ka, Mixer& m, Sequencer& s, MidiSynchronizer& ms, ChannelManager& cm, Recorder& r)
: m_engine(e)
, m_kernelAudio(ka)
, m_mixer(m)
, m_sequencer(s)
, m_midiSynchronizer(ms)
, m_channelManager(cm)
, m_recorder(r)
{
}

/* -------------------------------------------------------------------------- */

bool MainEngine::isRecordingInput() const
{
	return m_mixer.isRecordingInput();
}

/* -------------------------------------------------------------------------- */

bool MainEngine::isRecordingActions() const
{
	return m_mixer.isRecordingActions();
}

/* -------------------------------------------------------------------------- */

bool MainEngine::isSequencerRunning() const
{
	return m_sequencer.isRunning();
}

/* -------------------------------------------------------------------------- */

bool MainEngine::isMetronomeOn() const
{
	return m_sequencer.isMetronomeOn();
}

/* -------------------------------------------------------------------------- */

Peak MainEngine::getPeakOut() const { return m_mixer.getPeakOut(); }
Peak MainEngine::getPeakIn() const { return m_mixer.getPeakIn(); }

/* -------------------------------------------------------------------------- */

Mixer::RecordInfo MainEngine::getRecordInfo() const
{
	return m_mixer.getRecordInfo();
}

/* -------------------------------------------------------------------------- */

int MainEngine::getBeats() const
{
	return m_sequencer.getBeats();
}

int MainEngine::getBars() const
{
	return m_sequencer.getBars();
}

int MainEngine::getCurrentBeat() const
{
	return m_sequencer.getCurrentBeat();
}

/* -------------------------------------------------------------------------- */

SeqStatus MainEngine::getSequencerStatus() const
{
	return m_sequencer.getStatus();
}

/* -------------------------------------------------------------------------- */

void MainEngine::toggleMetronome()
{
	m_sequencer.toggleMetronome();
}

/* -------------------------------------------------------------------------- */

void MainEngine::setMasterInVolume(float v)
{
	m_channelManager.setVolume(Mixer::MASTER_IN_CHANNEL_ID, v);
}

void MainEngine::setMasterOutVolume(float v)
{
	m_channelManager.setVolume(Mixer::MASTER_OUT_CHANNEL_ID, v);
}

/* -------------------------------------------------------------------------- */

void MainEngine::multiplyBeats()
{
	setBeats(m_sequencer.getBeats() * 2, m_sequencer.getBars());
}

void MainEngine::divideBeats()
{
	setBeats(m_sequencer.getBeats() / 2, m_sequencer.getBars());
}

/* -------------------------------------------------------------------------- */

void MainEngine::setBpm(float bpm)
{
	if (m_mixer.isRecordingInput())
		return;
	m_sequencer.setBpm(bpm, m_kernelAudio.getSampleRate());
	m_midiSynchronizer.setClockBpm(bpm);
	m_engine.updateMixerModel();
}

/* -------------------------------------------------------------------------- */

void MainEngine::setBeats(int beats, int bars)
{
	if (m_mixer.isRecordingInput())
		return;

	const int sampleRate      = m_kernelAudio.getSampleRate();
	const int maxFramesInLoop = m_sequencer.getMaxFramesInLoop(sampleRate);

	m_sequencer.setBeats(beats, bars, sampleRate);
	m_mixer.allocRecBuffer(maxFramesInLoop);
	m_engine.updateMixerModel();
}

/* -------------------------------------------------------------------------- */

void MainEngine::goToBeat(int beat)
{
	m_sequencer.goToBeat(beat, m_kernelAudio.getSampleRate());
}

/* -------------------------------------------------------------------------- */

void MainEngine::startSequencer()
{
	m_sequencer.start();
}

void MainEngine::stopSequencer()
{
	m_sequencer.stop();
	m_channelManager.stopAll();
}

void MainEngine::toggleSequencer()
{
	m_sequencer.isRunning() ? stopSequencer() : startSequencer();
}

void MainEngine::rewindSequencer()
{
	m_sequencer.rewind();
	m_channelManager.rewindAll();
}

/* -------------------------------------------------------------------------- */

void MainEngine::setQuantize(int v)
{
	m_sequencer.setQuantize(v, m_kernelAudio.getSampleRate());
}
/* -------------------------------------------------------------------------- */

void MainEngine::setInToOut(bool v)
{
	m_mixer.setInToOut(v);
}

/* -------------------------------------------------------------------------- */

void MainEngine::toggleRecOnSignal()
{
	if (!m_recorder.canEnableRecOnSignal())
		m_engine.conf.data.recTriggerMode = RecTriggerMode::NORMAL;
	else
		m_engine.conf.data.recTriggerMode = m_engine.conf.data.recTriggerMode == RecTriggerMode::NORMAL ? RecTriggerMode::SIGNAL : RecTriggerMode::NORMAL;
	m_engine.updateMixerModel();
}

/* -------------------------------------------------------------------------- */

void MainEngine::toggleFreeInputRec()
{
	if (!m_recorder.canEnableFreeInputRec())
		m_engine.conf.data.inputRecMode = InputRecMode::RIGID;
	else
		m_engine.conf.data.inputRecMode = m_engine.conf.data.inputRecMode == InputRecMode::FREE ? InputRecMode::RIGID : InputRecMode::FREE;
	m_engine.updateMixerModel();
}

/* -------------------------------------------------------------------------- */

void MainEngine::stopActionRecording()
{
	if (m_mixer.isRecordingActions())
		m_recorder.stopActionRec();
}

void MainEngine::toggleActionRecording()
{
	if (m_mixer.isRecordingActions())
		m_recorder.stopActionRec();
	else
		m_recorder.prepareActionRec(m_engine.conf.data.recTriggerMode);
}

/* -------------------------------------------------------------------------- */

void MainEngine::stopInputRecording()
{
	if (m_mixer.isRecordingInput())
		m_recorder.stopInputRec(m_engine.conf.data.inputRecMode, m_kernelAudio.getSampleRate());
}

void MainEngine::toggleInputRecording()
{
	if (!m_kernelAudio.isInputEnabled() || !m_channelManager.hasInputRecordableChannels())
		return;
	if (m_mixer.isRecordingInput())
		m_recorder.stopInputRec(m_engine.conf.data.inputRecMode, m_kernelAudio.getSampleRate());
	else
		m_recorder.prepareInputRec(m_engine.conf.data.recTriggerMode, m_engine.conf.data.inputRecMode);
}

/* -------------------------------------------------------------------------- */

void MainEngine::startActionRecOnCallback()
{
	m_recorder.startActionRecOnCallback();
}
} // namespace giada::m
