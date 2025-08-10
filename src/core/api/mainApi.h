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

#ifndef G_MAIN_API_H
#define G_MAIN_API_H

#include "src/core/mixer.h"

namespace giada::m::rendering
{
class Reactor;
}

namespace giada::m
{
class Engine;
class KernelAudio;
class Sequencer;
class MidiSynchronizer;
class ChannelManager;
class Recorder;
class MainApi
{
public:
	MainApi(KernelAudio&, Mixer&, Sequencer&, MidiSynchronizer&, ChannelManager&, Recorder&,
	    rendering::Reactor&);

	bool              isRecordingInput() const;
	bool              isRecordingActions() const;
	bool              canRecordInput() const;
	bool              canEnableFreeInputRec() const;
	bool              canEnableRecOnSignal() const;
	bool              isSequencerRunning() const;
	RecTriggerMode    getRecTriggerMode() const;
	InputRecMode      getInputRecMode() const;
	bool              isMetronomeOn() const;
	bool              getInToOut() const;
	Peak              getPeakOut() const;
	Peak              getPeakIn() const;
	Mixer::RecordInfo getRecordInfo() const;
	int               getBeats() const;
	int               getBars() const;
	float             getBpm() const;
	int               getQuantizerValue() const;
	int               getCurrentBeat() const;
	Frame             getCurrentFrame() const;
	int               getFramesInBar() const;
	int               getFramesInLoop() const;
	int               getFramesInSeq() const;
	int               getFramesInBeat() const;
	SeqStatus         getSequencerStatus() const;

	void toggleMetronome();
	void setMasterInVolume(float);
	void setMasterOutVolume(float);
	void setBpm(float);
	void setBeats(int beats, int bars);
	void multiplyBeats();
	void divideBeats();
	void goToBeat(int);
	void startSequencer();
	void stopSequencer();
	void toggleSequencer();
	void rewindSequencer();
	void setQuantize(int);
	void setInToOut(bool);
	void toggleRecOnSignal();
	void toggleFreeInputRec();
	void stopActionRecording();
	void toggleActionRecording();
	void stopInputRecording();
	void toggleInputRecording();
	void startActionRecOnCallback();
	void setScene(std::size_t);

private:
	KernelAudio&        m_kernelAudio;
	Mixer&              m_mixer;
	Sequencer&          m_sequencer;
	MidiSynchronizer&   m_midiSynchronizer;
	ChannelManager&     m_channelManager;
	Recorder&           m_recorder;
	rendering::Reactor& m_reactor;
};
} // namespace giada::m

#endif
