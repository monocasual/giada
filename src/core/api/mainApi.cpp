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

#include "mainApi.h"
#include "core/channels/channelManager.h"
#include "core/engine.h"
#include "core/kernelAudio.h"
#include "core/midiSynchronizer.h"
#include "core/mixer.h"

namespace giada::m
{
MainApi::MainApi(Engine& e, Conf& c, KernelAudio& ka, Mixer& m, Sequencer& s, MidiSynchronizer& ms, ChannelManager& cm, Recorder& r)
: m_engine(e)
, m_conf(c)
, m_kernelAudio(ka)
, m_mixer(m)
, m_sequencer(s)
, m_midiSynchronizer(ms)
, m_channelManager(cm)
, m_recorder(r)
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

bool MainApi::isSequencerRunning() const
{
	return m_sequencer.isRunning();
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
	m_channelManager.setVolume(Mixer::MASTER_IN_CHANNEL_ID, v);
}

void MainApi::setMasterOutVolume(float v)
{
	m_channelManager.setVolume(Mixer::MASTER_OUT_CHANNEL_ID, v);
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

	const int sampleRate      = m_kernelAudio.getSampleRate();
	const int maxFramesInLoop = m_sequencer.getMaxFramesInLoop(sampleRate);

	m_sequencer.setBeats(beats, bars, sampleRate);
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
	m_channelManager.stopAll();
}

void MainApi::toggleSequencer()
{
	m_sequencer.isRunning() ? stopSequencer() : startSequencer();
}

void MainApi::rewindSequencer()
{
	m_sequencer.rewind();
	m_channelManager.rewindAll();
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
	if (m_mixer.isRecordingActions())
		m_recorder.stopActionRec();
}

void MainApi::toggleActionRecording()
{
	if (m_mixer.isRecordingActions())
		m_recorder.stopActionRec();
	else
		m_recorder.prepareActionRec(m_conf.recTriggerMode);
}

/* -------------------------------------------------------------------------- */

void MainApi::stopInputRecording()
{
	if (m_mixer.isRecordingInput())
		m_recorder.stopInputRec(m_conf.inputRecMode, m_kernelAudio.getSampleRate());
}

void MainApi::toggleInputRecording()
{
	if (!m_kernelAudio.isInputEnabled() || !m_channelManager.hasInputRecordableChannels())
		return;
	if (m_mixer.isRecordingInput())
		m_recorder.stopInputRec(m_conf.inputRecMode, m_kernelAudio.getSampleRate());
	else
		m_recorder.prepareInputRec(m_conf.recTriggerMode, m_conf.inputRecMode);
}

/* -------------------------------------------------------------------------- */

void MainApi::startActionRecOnCallback()
{
	m_recorder.startActionRecOnCallback();
}
} // namespace giada::m
