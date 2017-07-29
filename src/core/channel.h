/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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


#ifndef G_CHANNEL_H
#define G_CHANNEL_H


#include <vector>
#include <string>
#include <pthread.h>
#include "midiMapConf.h"
#include "recorder.h"

#ifdef WITH_VST
	#include "../deps/juce-config.h"
#endif


class Plugin;
class MidiMapConf;
class geChannel;


class Channel
{
protected:

#ifdef WITH_VST

	/* MidiBuffer contains MIDI events. When ready, events are sent to
	 * each plugin in the channel. This is available for any kind of
	 * channel, but it makes sense only for MIDI channels. */

	juce::MidiBuffer midiBuffer;

#endif

	/* bufferSize
	 * size of every buffer in this channel (vChan, pChan) */

	int bufferSize;

  /* midiFilter
  Which MIDI channel should be filtered out when receiving MIDI messages. -1
  means 'all'. */

  int midiFilter;

	float pan;

	/* sendMidiLMessage
	Composes a MIDI message by merging bytes from MidiMap conf class, and sends it 
	to KernelMidi. */

	void sendMidiLmessage(uint32_t learn, const giada::m::midimap::message_t& msg);

	/* calcPanning
	Given an audio channel (stereo: 0 or 1) computes the current panning value. */

	float calcPanning(int ch);

public:

	Channel(int type, int status, int bufferSize);

	virtual ~Channel();

	/* copy
	 * Make a shallow copy (no vChan/pChan allocation) of another channel. */

	virtual void copy(const Channel* src, pthread_mutex_t* pluginMutex) = 0;

	/* readPatch
	 * Fill channel with data from patch. */

	virtual int readPatch(const std::string& basePath, int i,
    pthread_mutex_t* pluginMutex, int samplerate, int rsmpQuality);

	/* process
	Merges vChannels into buffer, plus plugin processing (if any). Warning:
	inBuffer might be nullptr if no input devices are available for recording. */

	virtual void process(float* outBuffer, float* inBuffer) = 0;

	/* Preview
	Makes itself audibile for audio preview, such as Sample Editor or other
	tools. */

	virtual void preview(float* outBuffer) = 0;

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

	virtual void quantize(int index, int localFrame) = 0;

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

	virtual void parseAction(giada::m::recorder::action* a, int localFrame,
    int globalFrame, int quantize, bool mixerIsRunning) = 0;

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

	virtual int writePatch(int i, bool isProject);

	/* receiveMidi
	 * Receives and processes midi messages from external devices. */

	virtual void receiveMidi(uint32_t msg);

	/* allocBuffers
	Mandatory method to allocate memory for internal buffers. Call it after the
	object has been constructed. */
	
	virtual bool allocBuffers();

	/* ------------------------------------------------------------------------ */

	int    index;                 // unique id
	int    type;                  // midi or sample
	int    status;                // status: see const.h
	int    key;                   // keyboard button
	float  volume;                // global volume
	float  volume_i;              // internal volume
	float  volume_d;              // delta volume (for envelope)
	bool   mute_i;                // internal mute
	bool 	 mute_s;                // previous mute status after being solo'd
	bool   mute;                  // global mute
	bool   solo;
  bool   hasActions;            // has something recorded
  bool   readActions;           // read what's recorded
	bool   armed;                 // armed for recording
	int 	 recStatus;             // status of recordings (waiting, ending, ...)
	float* vChan;                 // virtual channel
  geChannel* guiChannel;        // pointer to a gChannel object, part of the GUI

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
  std::vector <Plugin*> plugins;
#endif


	/* ------------------------------------------------------------------------ */

	/* isPlaying
	 * tell wether the channel is playing or is stopped. */

	bool isPlaying();

	/* sendMidiL*
	 * send MIDI lightning events to a physical device. */

	void sendMidiLmute();
	void sendMidiLsolo();
	void sendMidiLplay();

	void setPan(float v);
	float getPan();

#ifdef WITH_VST

	/* getPluginMidiEvents
	 * Return a reference to midiBuffer stack. This is available for any kind of
	 * channel, but it makes sense only for MIDI channels. */

	juce::MidiBuffer& getPluginMidiEvents();

	void clearMidiBuffer();

#endif
};


#endif
