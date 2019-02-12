/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include "types.h"
#include "patch.h"
#include "mixer.h"
#include "midiMapConf.h"
#include "midiEvent.h"
#include "recorder.h"
#include "audioBuffer.h"

#ifdef WITH_VST
	#include "../deps/juce-config.h"
	#include "plugin.h"
#endif


class geChannel;


namespace giada {
namespace m
{
class Channel
{
public:

	virtual ~Channel() {};

	/* copy
	Makes a shallow copy (no internal buffers allocation) of another channel. */

	virtual void copy(const Channel* src, pthread_mutex_t* pluginMutex) = 0;

	/* parseEvents
	Prepares channel for rendering. This is called on each frame. */

	virtual void parseEvents(mixer::FrameEvents fe) = 0;

	/* process
	Merges working buffers into 'out', plus plugin processing (if any). Warning:
	inBuffer might be unallocated if no input devices are available for 
	recording. */

	virtual void process(AudioBuffer& out, const AudioBuffer& in, bool audible, 
		bool running) = 0;

	/* start
	Action to do when channel starts. doQuantize = false (don't quantize)
	when Mixer is reading actions from Recorder. */

	virtual void start(int localFrame, bool doQuantize, int velocity) = 0;

	/* stop
	What to do when channel is stopped normally (via key or MIDI). */

	virtual void stop() = 0;

	/* kill
	What to do when channel stops abruptly. */

	virtual void kill(int localFrame) = 0;

	/* set mute
	What to do when channel is un/muted. */

	virtual void setMute(bool value) = 0;

	/* set solo
	What to do when channel is un/soloed. */

	virtual void setSolo(bool value) = 0;

	/* empty
	Frees any associated resources (e.g. waveform for SAMPLE). */

	virtual void empty() = 0;

	/* stopBySeq
	What to do when channel is stopped by sequencer. */

	virtual void stopBySeq(bool chansStopOnSeqHalt) = 0;

	/* rewind
	Rewinds channel when rewind button is pressed. */

	virtual void rewindBySeq() = 0;

	/* canInputRec
	Tells whether a channel can accept and handle input audio. Always false for
	Midi channels, true for Sample channels only if they don't contain a
	sample yet.*/

	virtual bool canInputRec()    const { return false; };
	virtual bool hasLogicalData() const { return false; };
	virtual bool hasEditedData()  const { return false; };
	virtual bool hasData()        const { return false; };

	virtual bool recordStart(bool canQuantize) { return true; };
	virtual bool recordKill() { return true; };
	virtual void recordStop() {};

	/* prepareBuffer
	Fill audio buffer with audio data from the internal source. This is actually 
	useful to sample channels only. */

	virtual void prepareBuffer(bool running) {};

	virtual void startReadingActions(bool treatRecsAsLoops, 
		bool recsStopOnChanHalt) {};
	virtual void stopReadingActions(bool running, bool treatRecsAsLoops, 
		bool recsStopOnChanHalt) {};

	virtual void stopInputRec(int globalFrame) {};
	
	virtual void readPatch(const std::string& basePath, const patch::channel_t& pch);
	virtual void writePatch(int i, bool isProject);

	/* receiveMidi
	Receives and processes midi messages from external devices. */

	virtual void receiveMidi(const MidiEvent& midiEvent) {};

	/* calcPanning
	Given an audio channel (stereo: 0 or 1) computes the current panning value. */

	float calcPanning(int ch) const;

	bool isPlaying() const;
	float getPan() const;
	bool isPreview() const;

	/* isMidiInAllowed
	Given a MIDI channel 'c' tells whether this channel should be allowed to 
	receive and process MIDI events on MIDI channel 'c'. */

	bool isMidiInAllowed(int c) const;

	/* isReadingActions
	Tells whether the channel as actions and it is currently reading them. */

	bool isReadingActions() const;

	/* sendMidiL*
	Sends MIDI lightning events to a physical device. */

	void sendMidiLmute();
	void sendMidiLsolo();
	void sendMidiLstatus();

	void setPan(float v);

	void calcVolumeEnvelope();

#ifdef WITH_VST

	/* getPluginMidiEvents
	Returns a reference to midiBuffer stack. This is available for any kind of
	channel, but it makes sense only for MIDI channels. */

	const juce::MidiBuffer& getPluginMidiEvents() const;

	void clearMidiBuffer();

#endif

	/* guiChannel
	Pointer to a gChannel object, part of the GUI. TODO - remove this and send
	signals instead. */

	geChannel* guiChannel;

	/* buffer
	Working buffer for internal processing. */
	
	AudioBuffer buffer;

	ChannelType   type;
	ChannelStatus status;
	ChannelStatus recStatus;

	/* previewMode
	Whether the channel is in audio preview mode or not. */

	PreviewMode previewMode;

	float       pan;
	float       volume;   // global volume
	bool        armed;
	std::string name;
	int         index;    // unique id
	int         key;      // keyboard button
	bool        mute;     // global mute
	bool        solo;

	/* volume_*
	Internal volume variables: volume_i for envelopes, volume_d keeps track of
	the delta during volume changes (or the line slope between two volume 
	points). */
	
	double volume_i;
	double volume_d;
	
	bool hasActions;      // If has some actions recorded
	bool readActions;     // If should read recorded actions

	bool      midiIn;               // enable midi input
	uint32_t  midiInKeyPress;
	uint32_t  midiInKeyRel;
	uint32_t  midiInKill;
	uint32_t  midiInArm;
	uint32_t  midiInVolume;
	uint32_t  midiInMute;
	uint32_t  midiInSolo;

	/* midiInFilter
	Which MIDI channel should be filtered out when receiving MIDI messages. -1
	means 'all'. */

	int midiInFilter;

	/*  midiOutL*
	 * Enable MIDI lightning output, plus a set of midi lighting event to be sent
	 * to a device. Those events basically contains the MIDI channel, everything
	 * else gets stripped out. */

	bool     midiOutL;
	uint32_t midiOutLplaying;
	uint32_t midiOutLmute;
	uint32_t midiOutLsolo;

#ifdef WITH_VST
	std::vector<std::unique_ptr<Plugin>> plugins;
#endif

protected:

	Channel(ChannelType type, ChannelStatus status, int bufferSize);

#ifdef WITH_VST

	/* MidiBuffer contains MIDI events. When ready, events are sent to each plugin 
	in the channel. This is available for any kind of channel, but it makes sense 
	only for MIDI channels. */

	juce::MidiBuffer midiBuffer;

#endif
};

}} // giada::m::


#endif
