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


#ifndef G_MIXER_H
#define G_MIXER_H


#include <pthread.h>
#include <vector>
#include "../deps/rtaudio-mod/RtAudio.h"


class Channel;


namespace giada {
namespace m {
namespace mixer
{
void init(int framesInSeq, int audioBufferSize);
int  close();

/* masterPlay
 * core method (callback) */

int masterPlay(void *outBuf, void *inBuf, unsigned bufferSize, double streamTime,
  RtAudioStreamStatus status, void *userData);

/* isSilent
 * is mixer silent? */

bool isSilent();

/* rewind
 * rewind sequencer to sample 0. */

void rewind();

/* mergeVirtualInput
 * memcpy the virtual channel input in the channel designed for input
 * recording. Called by mixerHandler on stopInputRec() */

void mergeVirtualInput();

enum {    // const - what to do when a fadeout ends
	DO_STOP   = 0x01,
	DO_MUTE   = 0x02,
	DO_MUTE_I = 0x04
};

enum {    // const - fade types
	FADEOUT = 0x01,
	XFADE   = 0x02
};

extern std::vector<Channel*> channels;

extern bool   recording;         // is recording something?
extern bool   ready;
extern float *vChanInput;        // virtual channel for recording
extern float *vChanInToOut;      // virtual channel in->out bridge (hear what you're playin)
extern int    frameSize;
extern float  outVol;
extern float  inVol;
extern float  peakOut;
extern float  peakIn;
extern bool	 metronome;
extern int    waitRec;      // delayComp guard
extern bool  docross;			 // crossfade guard
extern bool  rewindWait;	   // rewind guard, if quantized

extern int  tickTracker, tockTracker;
extern bool tickPlay, tockPlay; // 1 = play, 0 = stop

/* inputTracker
 * position of the sample in the input side (recording) */

extern int inputTracker;

/* inToOut
 * copy, process and paste the input into the output, in order to
 * obtain a "hear what you're playing" feature. */

extern bool inToOut;

extern pthread_mutex_t mutex_recs;
extern pthread_mutex_t mutex_chans;
extern pthread_mutex_t mutex_plugins;

}}} // giada::m::mixer::;


#endif
