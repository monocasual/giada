/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * channel
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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

public:

	Channel(int type, int status, char side);
	virtual ~Channel();

	/* loadByPatch
	 * load a sample inside a patch. */

	virtual int loadByPatch(const char *file, int i) = 0;

	/* process
	 * merge vChannels into buffer, plus plugin processing (if any). */

	virtual void process(float *buffer, int size) = 0;

	/* start
	 * action to do when channel starts. doQuantize = false (don't
	 * quantize) when Mixer is reading actions from Recorder::. */

	virtual void start(bool doQuantize) = 0;

	/* stop
	 * action to do when channel is stopped normally (via key or MIDI). */

	virtual void stop() = 0;

	/* kill
	 * action to do when channel stops abruptly. */

	virtual void kill() = 0;

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

	/* writePatch
	 * store values in patch, writing to *fp. */

	virtual void writePatch(FILE *fp, int i, bool isProject) = 0;

	/* quantize
	 * start channel according to quantizer. Index = array index of
	 * mixer::channels, used by recorder. Frame = actual frame from
	 * mixer. */

	virtual void quantize(int index, int frame) = 0;

	/* onZero
	 * action to do when frame goes to zero, i.e. sequencer restart. */

	virtual void onZero() = 0;

	/* onBar
	 * action to do when a bar has passed. */

	virtual void onBar() = 0;

	/* parseAction
	 * do something on a recorded action. Frame = actual frame in Mixer. */

	virtual void parseAction(recorder::action *a, int frame) = 0;

	/* rewind
	 * rewind channel when rewind button is pressed. */

	virtual void rewind() = 0;

	/* ---------------------------------------------------------------- */

	int     index;                // unique id
	int     type;                 // midi or sample
	int     status;	              // status: see const.h
	char    side;                 // left or right column
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

  bool     midiIn;              // enable midi output
  uint32_t midiInKeyPress;
  uint32_t midiInKeyRel;
  uint32_t midiInKill;
  uint32_t midiInVolume;
  uint32_t midiInMute;
  uint32_t midiInSolo;

#ifdef WITH_VST
  gVector <class Plugin *> plugins;
#endif



	/* ---------------------------------------------------------------- */

	/* isPlaying
	 * tell wether the channel is playing or is stopped. */

	bool isPlaying();

	/* clear
	 * call memset to empty each vChan available. Useless for MIDI. */

	void clear(int bufSize);

	/* read/writePatchMidiIn
	 * read and write to patch all midi-related parameters such as
	 * keypress, mute and so on. */

	void readPatchMidiIn(int i);
	void writePatchMidiIn(FILE *fp, int i);
};


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


class SampleChannel : public Channel {

private:

	/* calcFadeoutStep
	 * how many frames are left before the end of the sample? Is there
	 * enough room for a complete fadeout? Should we shorten it? */

	void calcFadeoutStep();

	/* calcVolumeEnv
	 * compute any changes in volume done via envelope tool */

	void calcVolumeEnv(int frame);

public:

	SampleChannel(char side);
	~SampleChannel();

	void  process    (float *buffer, int size);
	void  start      (bool doQuantize);
	void  kill       ();
	void  empty      ();
	void  stopBySeq  ();
	void  stop       ();
	void  rewind     ();
	void  setMute    (bool internal);
	void  unsetMute  (bool internal);
	void  reset      ();
	int   load       (const char *file);
	int   loadByPatch(const char *file, int i);
	void  writePatch (FILE *fp, int i, bool isProject);
	void  quantize   (int index, int frame);
	void  onZero     ();
	void  onBar      ();
	void  parseAction(recorder::action *a, int frame);

	/* fade methods
	 * prepare channel for fade, mixer will take care of the process
	 * during master play. */

	void  setFadeIn  (bool internal);
	void  setFadeOut (int actionPostFadeout);
	void  setXFade   ();

	/* pushWave
	 * add a new wave to an existing channel. */

	void pushWave(class Wave *w);

	/* getPosition
	 * returns the position of an active sample. If EMPTY o MISSING
	 * returns -1. */

	int getPosition();

	/* sum
	 * add sample frames to virtual channel. Frame = processed frame in
	 * Mixer. Running = is Mixer in play? */

	void sum(int frame, bool running);

	/* setPitch
	 * updates the pitch value and chanStart+chanEnd accordingly. */

	void setPitch(float v);

	/* setStart/end
	 * change begin/end read points in sample. */

	void setBegin(unsigned v);
	void setEnd  (unsigned v);

	/* save
	 * save sample to file. */

	int save(const char *path);

	/* hardStop
	 * stop the channel immediately, no further checks. */

	void hardStop();

	/* allocEmpty
	 * alloc an empty wave used in input recordings. */

	bool allocEmpty(int frames, int takeId);

	/* canInputRec
	 * true if channel can host a new wave from input recording. */

	bool  canInputRec();

	/* ---------------------------------------------------------------- */

	class  Wave *wave;
	int    tracker;         // chan position
	int    begin;
	int    end;
	int    beginTrue;	      // chanStart NOT pitch affected
	int    endTrue;	        // chanend   NOT pitch affected
  float  pitch;
	float  boost;
	int    mode;            // mode: see const.h
	bool   qWait;           // quantizer wait
	float  fadein;
	bool   fadeoutOn;
	float  fadeoutVol;      // fadeout volume
	int    fadeoutTracker;  // tracker fadeout, xfade only
	float  fadeoutStep;     // fadeout decrease
  int    fadeoutType;     // xfade or fadeout
  int		 fadeoutEnd;      // what to do when fadeout ends
  int    key;

	/* recorder:: stuff */

  bool   readActions;     // read actions or not

	/* const - what to do when a fadeout ends */

	enum {
		DO_STOP   = 0x01,
		DO_MUTE   = 0x02,
		DO_MUTE_I = 0x04
	};

	/*  const - fade types */

	enum {
		FADEOUT = 0x01,
		XFADE   = 0x02
	};
};


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


class MidiChannel : public Channel {

public:

	MidiChannel(char side);
	~MidiChannel();

  bool    midiOut;           // enable midi output
  uint8_t midiOutChan;       // midi output channel

	void  process    (float *buffer, int size);
	void  start      (bool doQuantize);
	void  kill       ();
	void  empty      ();
	void  stopBySeq  ();
	void  stop       ();
	void  rewind     ();
	void  setMute    (bool internal);
	void  unsetMute  (bool internal);
	int   loadByPatch(const char *file, int i);
	void  writePatch (FILE *fp, int i, bool isProject);
	void  quantize   (int index, int frame);
	void  onZero     ();
	void  onBar      ();
	void  parseAction(recorder::action *a, int frame);

	/* ---------------------------------------------------------------- */

	/* sendMidi
	 * send Midi event to the outside world. */

	void sendMidi(recorder::action *a);
	void sendMidi(uint32_t data);

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
    int       numEvents;
    int       reserved;
    VstEvent *events[MAX_VST_EVENTS];
	} events;

#endif

};


#endif
