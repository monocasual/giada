/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * channel
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
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


#ifndef CHANNEL_H
#define CHANNEL_H


#include "utils.h"
#include "const.h"
#include "recorder.h"


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


class Channel {

protected:

	/* bufferSize
	 * size of every buffer in this channel (vChan, pChan) */

	int bufferSize;

public:

	Channel(int type, int status, int bufferSize);
	virtual ~Channel();

	/* writePatch
	 * store values in patch, writing to *fp. */

	virtual void writePatch(FILE *fp, int i, bool isProject);

	/* loadByPatch
	 * load a sample inside a patch. */

	virtual int loadByPatch(const char *file, int i) = 0;

	/* process
	 * merge vChannels into buffer, plus plugin processing (if any). */

	virtual void process(float *buffer) = 0;

	/* start
	 * action to do when channel starts. doQuantize = false (don't
	 * quantize) when Mixer is reading actions from Recorder::. */

	virtual void start(int frame, bool doQuantize) = 0;

	/* stop
	 * action to do when channel is stopped normally (via key or MIDI). */

	virtual void stop() = 0;

	/* kill
	 * action to do when channel stops abruptly. */

	virtual void kill(int frame) = 0;

	/* mute
	 * action to do when channel is muted. If internal == true, set
	 * internal mute without altering main mute. */

	virtual void setMute  (bool internal) = 0;
	virtual void unsetMute(bool internal) = 0;

	/* empty
	 * free any associated resources (e.g. waveform for SAMPLE). */

	virtual void empty() = 0;

	/* stopBySeq
	 * action to do when channel is stopped by sequencer. */

	virtual void stopBySeq() = 0;

	/* quantize
	 * start channel according to quantizer. Index = array index of
	 * mixer::channels, used by recorder. LocalFrame = frame within buffer.
	 * GloalFrame = actual frame from mixer. */

	virtual void quantize(int index, int localFrame, int globalFrame) = 0;

	/* onZero
	 * action to do when frame goes to zero, i.e. sequencer restart. */

	virtual void onZero(int frame) = 0;

	/* onBar
	 * action to do when a bar has passed. */

	virtual void onBar(int frame) = 0;

	/* parseAction
	 * do something on a recorded action. Parameters:
	 * action *a   - action to parse
	 * localFrame  - frame number of the processed buffer
	 * globalFrame - actual frame in Mixer */

	virtual void parseAction(recorder::action *a, int localFrame, int globalFrame) = 0;

	/* rewind
	 * rewind channel when rewind button is pressed. */

	virtual void rewind() = 0;

	/* ------------------------------------------------------------------------ */

	int     index;                // unique id
	int     type;                 // midi or sample
	int     status;	              // status: see const.h
	int     key;                  // keyboard button
	float   volume;               // global volume
	float   volume_i;             // internal volume
	float   volume_d;             // delta volume (for envelope)
	float   panLeft;
	float   panRight;
	bool    mute_i;               // internal mute
	bool 	  mute_s;               // previous mute status after being solo'd
	bool    mute;                 // global mute
	bool    solo;
  bool    hasActions;           // has something recorded
	int 	  recStatus;            // status of recordings (waiting, ending, ...)
	float  *vChan;	              // virtual channel
  class   gChannel *guiChannel; // pointer to a gChannel object, part of the GUI

  bool     midiIn;              // enable midi input
  uint32_t midiInKeyPress;
  uint32_t midiInKeyRel;
  uint32_t midiInKill;
  uint32_t midiInVolume;
  uint32_t midiInMute;
  uint32_t midiInSolo;

  bool     midiOut;              // enable midi output
	bool     midiOutL;             // enable midi lightning output

  uint32_t midiOutLplaying;      // set of midi lighting event sent to a device
  uint32_t midiOutLmute;
  uint32_t midiOutLsolo;

#ifdef WITH_VST
  gVector <class Plugin *> plugins;
#endif


	/* ------------------------------------------------------------------------ */

	/* isPlaying
	 * tell wether the channel is playing or is stopped. */

	bool isPlaying();

	/* readPatchMidiIn
	 * read from patch all midi-related parameters such as keypress, mute
	 * and so on. */

	void readPatchMidiIn(int i);
	void readPatchMidiOut(int i);

	void refreshMidiMuteLed();
	void refreshMidiSoloLed();
	void refreshMidiPlayLed();
};


#endif
