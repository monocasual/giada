/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * channel
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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

/* before including aeffetx(x).h we must define __cdecl, otherwise VST
 * headers can't be compiled correctly. In windows __cdecl is already
 * defined. */

	#ifdef __GNUC__
		#ifndef _WIN32
			#define __cdecl
		#endif
	#endif
	#include "vst/aeffectx.h"

#endif


class MidiChannel : public Channel {

public:

	MidiChannel(int bufferSize);
	~MidiChannel();

  bool    midiOut;           // enable midi output
  uint8_t midiOutChan;       // midi output channel

	void  process    (float *buffer);
	void  start      (int frame, bool doQuantize);
	void  kill       (int frame);
	void  empty      ();
	void  stopBySeq  ();
	void  stop       ();
	void  rewind     ();
	void  setMute    (bool internal);
	void  unsetMute  (bool internal);
	int   loadByPatch(const char *file, int i);
	void  writePatch (FILE *fp, int i, bool isProject);
	void  quantize   (int index, int localFrame, int globalFrame);
	void  onZero     (int frame);
	void  onBar      (int frame);
	void  parseAction(recorder::action *a, int localFrame, int globalFrame);

	/* ---------------------------------------------------------------- */

	/* sendMidi
	 * send Midi event to the outside world. */

	void sendMidi(recorder::action *a, int localFrame);
	void sendMidi(uint32_t data);
	void recvMidi(uint32_t data);

#ifdef WITH_VST

	/* getVstEvents
	 * return a pointer to gVstEvents. */

	VstEvents *getVstEvents();

	/* freeVstMidiEvents
	 * empty vstEvents structure. Init: use the method for channel
	 * initialization. */

	void freeVstMidiEvents(bool init=false);

	/* addVstMidiEvent
	 * take a composite MIDI event, decompose it and add it to channel. The
	 * other version creates a VstMidiEvent on the fly. */

	void addVstMidiEvent(struct VstMidiEvent *e);
	void addVstMidiEvent(uint32_t msg);

#endif

	/* ---------------------------------------------------------------- */

#ifdef WITH_VST

	/* VST struct containing MIDI events. When ready, events are sent to
	 * each plugin in the channel.
	 *
	 * Anatomy of VstEvents
	 * --------------------
	 *
	 * VstInt32  numEvents = number of Events in array
	 * VstIntPtr reserved  = zero (Reserved for future use)
	 * VstEvent *events[n] = event pointer array, variable size
	 *
	 * Note that by default VstEvents only holds three events- if you want
	 * it to hold more, create an equivalent struct with a larger array,
	 * and then cast it to a VstEvents object when you've populated it.
	 * That's what we do with gVstEvents! */

	struct gVstEvents {
    VstInt32  numEvents;
    VstIntPtr reserved;
    VstEvent *events[MAX_VST_EVENTS];
	} events;

#endif

};


#endif
