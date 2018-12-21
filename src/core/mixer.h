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


#ifndef G_MIXER_H
#define G_MIXER_H


#include <pthread.h>
#include <vector>
#include "recorder.h"
#include "types.h"
#include "../deps/rtaudio-mod/RtAudio.h"


namespace giada {
namespace m
{
class Action;
class Channel;

namespace mixer
{
struct FrameEvents
{
	Frame frameLocal;
	Frame frameGlobal;
	bool  doQuantize;
	bool  onBar;
	bool  onFirstBeat;
	bool  quantoPassed;
	std::vector<const Action*> actions;
};

extern std::vector<Channel*> channels;

extern bool   recording;         // is recording something?
extern bool   ready;
extern float  outVol;
extern float  inVol;
extern float  peakOut;
extern float  peakIn;
extern bool	  metronome;
extern int    waitRec;       // delayComp guard
extern bool   rewindWait;	   // rewind guard, if quantized
extern bool   hasSolos;      // more than 0 channels soloed

/* inToOut
Copy, process and paste the input into the output, in order to obtain a "hear 
what you're playing" feature. */

extern bool inToOut;

extern pthread_mutex_t mutex;

void init(Frame framesInSeq, Frame framesInBuffer);

/* allocVirtualInput
Allocates new memory for the virtual input channel. Call this whenever you 
shrink or resize the sequencer. */

void allocVirtualInput(Frame frames);

void close();

/* masterPlay
Core method (callback) */

int masterPlay(void* outBuf, void* inBuf, unsigned bufferSize, double streamTime,
	RtAudioStreamStatus status, void* userData);

/* isSilent
Is mixer silent? */

bool isSilent();

bool isChannelAudible(Channel* ch);

/* rewind
Rewinds sequencer to frame 0. */

void rewind();

/* startInputRec
Starts input recording on frame clock::getCurrentFrame(). */

void startInputRec();

/* mergeVirtualInput
Copies the virtual channel input in the channels designed for input recording. 
Called by mixerHandler on stopInputRec(). */

void mergeVirtualInput();
}}} // giada::m::mixer::;


#endif
