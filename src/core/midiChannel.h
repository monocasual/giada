/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#ifndef G_MIDI_CHANNEL_H
#define G_MIDI_CHANNEL_H


#ifdef WITH_VST
	#include "../deps/juce-config.h"
#endif
#include "channel.h"


class MidiMapConf;
class Patch;


class MidiChannel : public Channel
{
public:

	MidiChannel(int bufferSize);

	void copy(const Channel* src, pthread_mutex_t* pluginMutex) override;
	void parseEvents(giada::m::mixer::FrameEvents fe) override;
	void process(giada::m::AudioBuffer& out, const giada::m::AudioBuffer& in, 
		bool audible, bool running) override;
	void start(int frame, bool doQuantize, int velocity) override;
	void kill(int localFrame) override;
	void empty() override;
	void stopBySeq(bool chansStopOnSeqHalt) override;
	void stop() override {};
	void rewindBySeq() override;
	void setMute(bool value) override;
	void setSolo(bool value) override;
	void readPatch(const std::string& basePath, int i) override;
	void writePatch(int i, bool isProject) override;
	void receiveMidi(const giada::m::MidiEvent& midiEvent) override;
	bool canInputRec() override;

	/* sendMidi
	 * send Midi event to the outside world. */

	void sendMidi(giada::m::recorder::action* a, int localFrame);
	void sendMidi(uint32_t data);

#ifdef WITH_VST

	/* addVstMidiEvent
	 * Add a new Midi event to the midiEvent stack fom a composite uint32_t raw
	 * Midi event. LocalFrame is the offset: it tells where to put the event
	 * inside the buffer. */

	void addVstMidiEvent(uint32_t msg, int localFrame);

#endif

	bool    midiOut;           // enable midi output
	uint8_t midiOutChan;       // midi output channel
};


#endif
