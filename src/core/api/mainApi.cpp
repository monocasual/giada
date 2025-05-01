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

#include "src/core/api/mainApi.h"
#include "src/core/channels/channelManager.h"
#include "src/core/engine.h"
#include "src/core/kernelAudio.h"
#include "src/core/midiSynchronizer.h"
#include "src/core/mixer.h"

namespace giada::m
{
MainApi::MainApi(KernelAudio& ka, Mixer& m, Sequencer& s, MidiSynchronizer& ms,
    ChannelManager& cm, Recorder& r, rendering::Reactor& re)
: m_kernelAudio(ka)
, m_mixer(m)
, m_sequencer(s)
, m_midiSynchronizer(ms)
, m_channelManager(cm)
, m_recorder(r)
, m_reactor(re)
{
}

/* -------------------------------------------------------------------------- */

bool MainApi::isRecordingInput() const
{
	return m_mixer.isRecordingInput();
}

/* -------------------------------------------------------------------------- */

bool MainApi::isRecordingActions() const
{
	return m_mixer.isRecordingActions();
}

/* -------------------------------------------------------------------------- */

bool MainApi::canRecordInput() const
{
	return m_kernelAudio.isInputEnabled() && m_channelManager.hasInputRecordableChannels();
}

/* -------------------------------------------------------------------------- */

bool MainApi::canEnableFreeInputRec() const
{
	return m_recorder.canEnableFreeInputRec();
}

/* -------------------------------------------------------------------------- */

bool MainApi::canEnableRecOnSignal() const
{
	return m_recorder.canEnableRecOnSignal();
}

/* -------------------------------------------------------------------------- */

bool MainApi::isSequencerRunning() const
{
	return m_sequencer.isRunning();
}

/* -------------------------------------------------------------------------- */

RecTriggerMode MainApi::getRecTriggerMode() const
{
	return m_mixer.getRecTriggerMode();
}

/* -------------------------------------------------------------------------- */

InputRecMode MainApi::getInputRecMode() const
{
	return m_mixer.getInputRecMode();
}

/* -------------------------------------------------------------------------- */

bool MainApi::isMetronomeOn() const
{
	return m_sequencer.isMetronomeOn();
}

/* -------------------------------------------------------------------------- */

bool MainApi::getInToOut() const
{
	return m_mixer.getInToOut();
}

/* -------------------------------------------------------------------------- */

Peak MainApi::getPeakOut() const { return m_mixer.getPeakOut(); }
Peak MainApi::getPeakIn() const { return m_mixer.getPeakIn(); }

/* -------------------------------------------------------------------------- */

double MainApi::getCpuLoad() const { return m_kernelAudio.getCpuLoad(); }

/* -------------------------------------------------------------------------- */

Mixer::RecordInfo MainApi::getRecordInfo() const
{
	return m_mixer.getRecordInfo();
}

/* -------------------------------------------------------------------------- */

int MainApi::getBeats() const
{
	return m_sequencer.getBeats();
}

int MainApi::getBars() const
{
	return m_sequencer.getBars();
}

float MainApi::getBpm() const
{
	return m_sequencer.getBpm();
}

int MainApi::getQuantizerValue() const
{
	return m_sequencer.getQuantizerValue();
}

int MainApi::getCurrentBeat() const
{
	return m_sequencer.getCurrentBeat();
}

Frame MainApi::getCurrentFrame() const
{
	return m_sequencer.getCurrentFrame();
}

int MainApi::getFramesInBar() const
{
	return m_sequencer.getFramesInBar();
}

int MainApi::getFramesInLoop() const
{
	return m_sequencer.getFramesInLoop();
}

int MainApi::getFramesInSeq() const
{
	return m_sequencer.getFramesInSeq();
}

int MainApi::getFramesInBeat() const
{
	return m_sequencer.getFramesInBeat();
}

/* -------------------------------------------------------------------------- */

SeqStatus MainApi::getSequencerStatus() const
{
	return m_sequencer.getStatus();
}

/* -------------------------------------------------------------------------- */

void MainApi::toggleMetronome()
{
	m_sequencer.toggleMetronome();
}

/* -------------------------------------------------------------------------- */

void MainApi::setMasterInVolume(float v)
{
	m_channelManager.setVolume(MASTER_IN_CHANNEL_ID, v);
}

void MainApi::setMasterOutVolume(float v)
{
	m_channelManager.setVolume(MASTER_OUT_CHANNEL_ID, v);
}

/* -------------------------------------------------------------------------- */

void MainApi::multiplyBeats()
{
	setBeats(m_sequencer.getBeats() * 2, m_sequencer.getBars());
}

void MainApi::divideBeats()
{
	setBeats(m_sequencer.getBeats() / 2, m_sequencer.getBars());
}

/* -------------------------------------------------------------------------- */

void MainApi::setBpm(float bpm)
{
	if (m_mixer.isRecordingInput())
		return;
	m_sequencer.setBpm(bpm, m_kernelAudio.getSampleRate());
	m_midiSynchronizer.setClockBpm(bpm);
}

/* -------------------------------------------------------------------------- */

void MainApi::setBeats(int beats, int bars)
{
	if (m_mixer.isRecordingInput())
		return;

	const int sampleRate = m_kernelAudio.getSampleRate();
	m_sequencer.setBeats(beats, bars, sampleRate);

	const int maxFramesInLoop = m_sequencer.getMaxFramesInLoop(sampleRate);
	m_mixer.allocRecBuffer(maxFramesInLoop);
}

/* -------------------------------------------------------------------------- */

void MainApi::goToBeat(int beat)
{
	m_sequencer.goToBeat(beat, m_kernelAudio.getSampleRate());
}

/* -------------------------------------------------------------------------- */

void MainApi::startSequencer()
{
	m_sequencer.start();
}

void MainApi::stopSequencer()
{
	m_sequencer.stop();
	m_reactor.stopAll();
}

void MainApi::toggleSequencer()
{
	m_sequencer.isRunning() ? stopSequencer() : startSequencer();
}

void MainApi::rewindSequencer()
{
	m_sequencer.rewind();
	m_reactor.rewindAll();
}

/* -------------------------------------------------------------------------- */

void MainApi::setQuantize(int v)
{
	m_sequencer.setQuantize(v, m_kernelAudio.getSampleRate());
}
/* -------------------------------------------------------------------------- */

void MainApi::setInToOut(bool v)
{
	m_mixer.setInToOut(v);
}

/* -------------------------------------------------------------------------- */

void MainApi::toggleRecOnSignal() { m_recorder.toggleRecOnSignal(); }
void MainApi::toggleFreeInputRec() { m_recorder.toggleFreeInputRec(); }

/* -------------------------------------------------------------------------- */

void MainApi::stopActionRecording()
{
	m_recorder.stopActionRec();
}

void MainApi::toggleActionRecording()
{
	m_recorder.toggleActionRec();
}

/* -------------------------------------------------------------------------- */

void MainApi::stopInputRecording()
{
	m_recorder.stopInputRec(m_kernelAudio.getSampleRate());
}

void MainApi::toggleInputRecording()
{
	if (!m_kernelAudio.isInputEnabled())
		return;
	m_recorder.toggleInputRec(m_kernelAudio.getSampleRate());

	if (m_mixer.isRecordingInput() && m_mixer.getInputRecMode() == InputRecMode::FREE && m_sequencer.isMetronomeOn())
		m_sequencer.setMetronome(false);
}

/* -------------------------------------------------------------------------- */

void MainApi::startActionRecOnCallback()
{
	m_recorder.startActionRecOnCallback();
}
} // namespace giada::m
