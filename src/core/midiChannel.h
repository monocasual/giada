/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * channel
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#ifndef MIDI_CHANNEL_H
#define MIDI_CHANNEL_H


#ifdef WITH_VST
	#include "../deps/juce/config.h"
#endif

#include "channel.h"


class MidiChannel : public Channel
{
public:

	MidiChannel(int bufferSize, class MidiMapConf *midiMapConf);
	~MidiChannel();

  bool    midiOut;           // enable midi output
  uint8_t midiOutChan;       // midi output channel

	void copy(const Channel *src, pthread_mutex_t *pluginMutex);

	void clear      ();
	void process    (float *buffer);
	void start      (int frame, bool doQuantize, int quantize, bool mixerIsRunning,
		bool isUserGenerated=false);
	void kill       (int frame);
	void empty      ();
	void stopBySeq  (bool chansStopOnSeqHalt);
	void stop       ();
	void rewind     ();
	void setMute    (bool internal);
	void unsetMute  (bool internal);
	int  readPatch_DEPR_  (const char *file, int i, class Patch_DEPR_ *patch,
			int samplerate, int rsmpQuality);
	int  readPatch  (const string &basePath, int i, class Patch *patch,
			pthread_mutex_t *pluginMutex, int samplerate, int rsmpQuality);
	int  writePatch (int i, bool isProject, class Patch *patch);
	void quantize   (int index, int localFrame, int globalFrame);
	void onZero     (int frame, bool recsStopOnChanHalt);
	void onBar      (int frame);
	void parseAction(Recorder::action *a, int localFrame, int globalFrame,
			int quantize, bool mixerIsRunning);

	/* ---------------------------------------------------------------- */

	/* sendMidi
	 * send Midi event to the outside world. */

	void sendMidi(Recorder::action *a, int localFrame);
	void sendMidi(uint32_t data);

#ifdef WITH_VST

	/* freeVstMidiEvents
	 * empty vstEvents structure. Init: use the method for channel
	 * initialization. */

	void freeVstMidiEvents(bool init=false);

	/* addVstMidiEvent
	 * Add a new Midi event to the midiEvent stack fom a composite uint32_t raw
	 * Midi event. LocalFrame is the offset: it tells where to put the event
	 * inside the buffer. */

	void addVstMidiEvent(uint32_t msg, int localFrame);

#endif
};


#endif
