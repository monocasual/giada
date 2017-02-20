/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * channel
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include <vector>
#include <pthread.h>
#include "midiMapConf.h"
#include "recorder.h"

#ifdef WITH_VST
	#include "../deps/juce-config.h"
#endif


using std::vector;
using std::string;


class PluginHost;
class Plugin;
class MidiMapConf;
class Patch_DEPR_;
class Patch;
class Mixer;
class Clock;
class geChannel;


class Channel
{
protected:

#ifdef WITH_VST

	/* pluginHost
	 * Pointer to PluginHost class, which manages and processes plugins. */

	PluginHost *pluginHost;

	/* MidiBuffer contains MIDI events. When ready, events are sent to
	 * each plugin in the channel. This is available for any kind of
	 * channel, but it makes sense only for MIDI channels. */

	juce::MidiBuffer midiBuffer;

#endif

  /* clock
  Pointer to Clock class. It manages timing. */

  Clock *clock;

	/* MidiMapConf
	 * Pointer to MidiMapConf. It deals with Midi lightning operations. */

	MidiMapConf *midiMapConf;

	/* bufferSize
	 * size of every buffer in this channel (vChan, pChan) */

	int bufferSize;

	/* sendMidiLMessage
	 * compose a MIDI message by merging bytes from MidiMap conf class, and send
	 * it to KernelMidi. */

	void sendMidiLmessage(uint32_t learn, const MidiMapConf::message_t &msg);

public:

	Channel(int type, int status, int bufferSize, MidiMapConf *midiMapConf,
    Clock *clock);

	virtual ~Channel();

	/* copy
	 * Make a shallow copy (no vChan/pChan allocation) of another channel. */

	virtual void copy(const Channel *src, pthread_mutex_t *pluginMutex) = 0;

	/* readPatch
	 * Fill channel with data from patch. */

	virtual int readPatch_DEPR_(const char *file, int i, Patch_DEPR_ *patch,
			int samplerate, int rsmpQuality) = 0;
	virtual int readPatch(const string &basePath, int i, Patch *patch,
			pthread_mutex_t *pluginMutex, int samplerate, int rsmpQuality);

	/* process
	Merges vChannels into buffer, plus plugin processing (if any). Warning:
	inBuffer might be nullptr if no input devices are available for recording. */

	virtual void process(float *outBuffer, float *inBuffer) = 0;

	/* start
	Action to do when channel starts. doQuantize = false (don't quantize)
	when Mixer is reading actions from Recorder. If isUserGenerated means that
	the channel has been started by a human key press and not a pre-recorded
	action. */

	virtual void start(int frame, bool doQuantize, int quantize,
			bool mixerIsRunning, bool forceStart, bool isUserGenerated) = 0;

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

	virtual void stopBySeq(bool chansStopOnSeqHalt) = 0;

	/* quantize
	 * start channel according to quantizer. Index = array index of
	 * mixer::channels, used by recorder. LocalFrame = frame within the current
   * buffer.  */

	virtual void quantize(int index, int localFrame, Mixer *m) = 0;

	/* onZero
	 * action to do when frame goes to zero, i.e. sequencer restart. */

	virtual void onZero(int frame, bool recsStopOnChanHalt) = 0;

	/* onBar
	 * action to do when a bar has passed. */

	virtual void onBar(int frame) = 0;

	/* parseAction
	 * do something on a recorded action. Parameters:
	 * action *a   - action to parse
	 * localFrame  - frame number of the processed buffer
	 * globalFrame - actual frame in Mixer */

	 // TODO - quantize is useless!

	virtual void parseAction(Recorder::action *a, int localFrame, int globalFrame,
			int quantize, bool mixerIsRunning) = 0;

	/* rewind
	 * rewind channel when rewind button is pressed. */

	virtual void rewind() = 0;

	/* clear
	Clears all memory buffers. This is actually useful to sample channels only. */

	virtual void clear() = 0;

	/* canInputRec
	Tells whether a channel can accept and handle input audio. Always false for
	Midi channels, true for Sample channels only if they don't contain a
	sample yet.*/

	virtual bool canInputRec() = 0;

	/* writePatch
	 * Fill a patch with channel values. Returns the index of the last
	 * Patch::channel_t added. */

	virtual int writePatch(int i, bool isProject, Patch *patch);

	/* receiveMidi
	 * Receives and processes midi messages from external devices. */

	virtual void receiveMidi(uint32_t msg);

	/* ------------------------------------------------------------------------ */

	int     index;                 // unique id
	int     type;                  // midi or sample
	int     status;	               // status: see const.h
	int     key;                   // keyboard button
	float   volume;                // global volume
	float   volume_i;              // internal volume
	float   volume_d;              // delta volume (for envelope)
	float   panLeft;
	float   panRight;
	bool    mute_i;                // internal mute
	bool 	  mute_s;                // previous mute status after being solo'd
	bool    mute;                  // global mute
	bool    solo;
  bool    hasActions;            // has something recorded
  bool    readActions;           // read what's recorded
	bool    armed;							   // armed for recording
	int 	  recStatus;             // status of recordings (waiting, ending, ...)
	float  *vChan;	               // virtual channel
  geChannel *guiChannel;         // pointer to a gChannel object, part of the GUI

	// TODO - midi structs, please

  bool     midiIn;              // enable midi input
  uint32_t midiInKeyPress;
  uint32_t midiInKeyRel;
  uint32_t midiInKill;
  uint32_t midiInArm;
  uint32_t midiInVolume;
  uint32_t midiInMute;
  uint32_t midiInSolo;

	/*  midiOutL*
	 * Enable MIDI lightning output, plus a set of midi lighting event to be sent
	 * to a device. Those events basically contains the MIDI channel, everything
	 * else gets stripped out. */

	bool     midiOutL;
  uint32_t midiOutLplaying;
  uint32_t midiOutLmute;
  uint32_t midiOutLsolo;

#ifdef WITH_VST
  vector <Plugin *> plugins;
#endif


	/* ------------------------------------------------------------------------ */

	/* isPlaying
	 * tell wether the channel is playing or is stopped. */

	bool isPlaying();

	/* readPatchMidiIn
	 * read from patch all midi-related parameters such as keypress, mute
	 * and so on. */

	void readPatchMidiIn_DEPR_(int i, Patch_DEPR_ &patch);
	void readPatchMidiOut_DEPR_(int i, Patch_DEPR_ &patch);

	/* sendMidiL*
	 * send MIDI lightning events to a physical device. */

	void sendMidiLmute();
	void sendMidiLsolo();
	void sendMidiLplay();

#ifdef WITH_VST

	/* SetPluginHost
	 * A neat trick to avoid duplicated constructors (with and without pointer
	 * to PluginHost). */

	void setPluginHost(PluginHost *pluginHost);

	/* getPluginMidiEvents
	 * Return a reference to midiBuffer stack. This is available for any kind of
	 * channel, but it makes sense only for MIDI channels. */

	juce::MidiBuffer &getPluginMidiEvents();

	void clearMidiBuffer();

#endif
};


#endif
