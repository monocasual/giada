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


#ifndef G_MIXER_H
#define G_MIXER_H


#include <atomic>
#include <functional>
#include <vector>
#include "deps/rtaudio-mod/RtAudio.h"
#include "core/recorder.h"
#include "core/types.h"


namespace giada {
namespace m
{
struct Action;
class Channel;
class AudioBuffer;

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
	const std::vector<Action>* actions;
};

constexpr int MASTER_OUT_CHANNEL_ID = 1;
constexpr int MASTER_IN_CHANNEL_ID  = 2;
constexpr int PREVIEW_CHANNEL_ID    = 3;

extern std::atomic<bool>  rewindWait;    // rewind guard, if quantized
extern std::atomic<float> peakOut;
extern std::atomic<float> peakIn;

void init(Frame framesInSeq, Frame framesInBuffer);

/* enable, disable
Toggles master callback processing. Useful when loading a new patch. Mixer
will flush itself to wait for a processing cycle to finish when disable() is
called. */

void enable();
void disable();

/* allocVirtualInput
Allocates new memory for the virtual input channel. Call this whenever you 
shrink or resize the sequencer. */

void allocVirtualInput(Frame frames);

/* clearVirtualInput
Clears internal virtual channel. */

void clearVirtualInput();

/* getVirtualInput
Returns a read-only reference to the internal virtual channel. Use this to
merge data into channel after an input recording session. */

const AudioBuffer& getVirtualInput(); 

void close();

/* masterPlay
Core method (callback) */

int masterPlay(void* outBuf, void* inBuf, unsigned bufferSize, double streamTime,
	RtAudioStreamStatus status, void* userData);

bool isChannelAudible(const Channel* ch);

/* startInputRec, stopInputRec
Starts/stops input recording on frame clock::getCurrentFrame(). */

void startInputRec();
void stopInputRec();

void toggleMetronome();
bool isMetronomeOn();
void setMetronome(bool v);

void setSignalCallback(std::function<void()> f);
}}} // giada::m::mixer::;


#endif
