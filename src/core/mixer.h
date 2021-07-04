/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "core/midiEvent.h"
#include "core/queue.h"
#include "core/recorder.h"
#include "core/ringBuffer.h"
#include "core/types.h"
#include "deps/rtaudio/RtAudio.h"
#include <functional>

namespace mcl
{
class AudioBuffer;
}
namespace giada::m
{
struct Action;
} // namespace giada::m
namespace giada::m::channel
{
struct Data;
}
namespace giada::m::mixer
{
constexpr int MASTER_OUT_CHANNEL_ID = 1;
constexpr int MASTER_IN_CHANNEL_ID  = 2;
constexpr int PREVIEW_CHANNEL_ID    = 3;

/* RenderInfo
Struct of parameters passed to Mixer for rendering. */

struct RenderInfo
{
	bool  isAudioReady;
	bool  hasInput;
	bool  isClockActive;
	bool  isClockRunning;
	bool  canLineInRec;
	bool  limitOutput;
	bool  inToOut;
	Frame maxFramesToRec;
	float outVol;
	float inVol;
	float recTriggerLevel;
};

/* RecordInfo
Information regarding the input recording progress. */

struct RecordInfo
{
	Frame position;
	Frame maxLength;
};

void init(Frame framesInLoop, Frame framesInBuffer);

/* enable, disable
Toggles master callback processing. Useful to suspend the rendering. */

void enable();
void disable();

/* allocRecBuffer
Allocates new memory for the virtual input channel. */

void allocRecBuffer(Frame frames);

/* clearRecBuffer
Clears internal virtual channel. */

void clearRecBuffer();

/* getRecBuffer
Returns a read-only reference to the internal virtual channel. Use this to
merge data into channel after an input recording session. */

const mcl::AudioBuffer& getRecBuffer();

/* render
Core rendering function. */

int render(mcl::AudioBuffer& out, const mcl::AudioBuffer& in, const RenderInfo& info);

/* startInputRec, stopInputRec
Starts/stops input recording on frame 'from'. The latter returns the number of
recorded frames. */

void  startInputRec(Frame from);
Frame stopInputRec();

/* setSignalCallback
Registers the function to be called when the audio signal reaches a certain
threshold (record-on-signal mode). */

void setSignalCallback(std::function<void()> f);

/* setEndOfRecCallback
Registers the function to be called when the end of the internal recording 
buffer has been reached. */

void setEndOfRecCallback(std::function<void()> f);

/* isChannelAudible
True if the channel 'c' is currently audible: not muted or not included in a 
solo session. */

bool isChannelAudible(const channel::Data& c);

Peak getPeakOut();
Peak getPeakIn();

RecordInfo getRecordInfo();
} // namespace giada::m::mixer

#endif
