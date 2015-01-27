/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * mixer
 *
 * ---------------------------------------------------------------------
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
 * ------------------------------------------------------------------ */


#ifndef MIXER_H
#define MIXER_H

#include <stdlib.h>
#include <pthread.h>
#include "const.h"
#include "kernelAudio.h"
#include "utils.h"


class Mixer {

public:

	Mixer();
	~Mixer();

	void init();
	int  close();

	/* addChannel
	 * add a new channel without any wave inside of it. */

	class Channel *addChannel(int type);

	/* deleteChannel
	 * completely remove a channel from the stack. */

	int deleteChannel(class Channel *ch);

	/* masterPlay
	 * core method (callback) */

	static int masterPlay(
		void *out_buf, void *in_buf, unsigned n_frames,
		double streamTime, RtAudioStreamStatus status, void *userData
	);
	int __masterPlay(void *out_buf, void *in_buf, unsigned n_frames);

	/* updateFrameBars
	 * updates bpm, frames, beats and so on. */

	void updateFrameBars();

	/* isSilent
	 * is mixer silent? */

	bool isSilent();

	/* rewind
	 * rewind sequencer to sample 0. */

	void rewind();

	/* updateQuanto
	 * recomputes the quanto between two quantizations */

	void updateQuanto();

	/* hasLogicalSamples
	 * true if 1 or more samples are logical (memory only, such as takes) */

	bool hasLogicalSamples();

	/* hasEditedSamples
	 * true if 1 or more samples was edited via gEditor */

	bool hasEditedSamples();

	/* mergeVirtualInput
	 * memcpy the virtual channel input in the channel designed for input
	 * recording. Called by mixerHandler on stopInputRec() */

	bool mergeVirtualInput();

	/* getChannelByIndex
	 * return channel with given index 'i'. */

	Channel *getChannelByIndex(int i);

	inline Channel* getLastChannel() { return channels.at(channels.size-1); }


	/* ---------------------------------------------------------------- */


	enum {    // const - what to do when a fadeout ends
		DO_STOP   = 0x01,
		DO_MUTE   = 0x02,
		DO_MUTE_I = 0x04
	};

	enum {    // const - fade types
		FADEOUT = 0x01,
		XFADE   = 0x02
	};

	gVector<class Channel*> channels;

	bool   running;
	bool   ready;
	float *vChanInput;        // virtual channel for recording
	float *vChanInToOut;      // virtual channel in->out bridge (hear what you're playin)
	int    frameSize;
	float  outVol;
	float  inVol;
	float  peakOut;
	float  peakIn;
	int    quanto;
	char   quantize;
	bool	 metronome;
	float  bpm;
	int    bars;
	int    beats;
	int    waitRec;      // delayComp guard

	bool docross;			   // crossfade guard
	bool rewindWait;	   // rewind guard, if quantized

	int framesPerBar;      // frames in one bar
	int framesPerBeat;     // frames in one beat
	int framesInSequencer; // frames in the whole sequencer
	int totalFrames;       // frames in the selected range (e.g. 4/4)
	int actualFrame;
	int actualBeat;

#define TICKSIZE 38
	static float tock[TICKSIZE];
	static float tick[TICKSIZE];
	int  tickTracker, tockTracker;
	bool tickPlay, tockPlay; // 1 = play, 0 = stop

	/* chanInput
	 * the active channel during a recording. NULL = no channels active */

	class SampleChannel *chanInput;

	/* inputTracker
	 * position of the sample in the input side (recording) */

	int inputTracker;

	/* inToOut
	 * copy, process and paste the input into the output, in order to
	 * obtain a "hear what you're playing" feature. */

	bool inToOut;

	pthread_mutex_t mutex_recs;
	pthread_mutex_t mutex_chans;
	pthread_mutex_t mutex_plugins;


private:

	int midiTCstep;      // part of MTC to send (0 to 7)
	int midiTCrate;      // send MTC data every midiTCrate frames
	int midiTCframes;
	int midiTCseconds;
	int midiTCminutes;
	int midiTChours;

	/* getNewIndex
	 * compute new index value for new channels */

	int getNewIndex();

	/* sendMIDIsync
	 * generate MIDI sync output data */

	void sendMIDIsync();

	/* sendMIDIrewind
	 * rewind timecode to beat 0 and also send a MTC full frame to cue
	 * the slave */

	void sendMIDIrewind();
};

#endif
