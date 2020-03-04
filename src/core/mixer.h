/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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
#include "deps/rtaudio/RtAudio.h"
#include "core/ringBuffer.h"
#include "core/recorder.h"
#include "core/types.h"
#include "core/queue.h"
#include "core/midiEvent.h"


namespace giada {
namespace m
{
struct Action;
class Channel;
class Channel;
class AudioBuffer;

namespace mixer
{
enum class EventType 
{ 
	KEY_PRESS, 
	KEY_RELEASE, 
	KEY_KILL, 
	SEQUENCER_FIRST_BEAT, 
	SEQUENCER_BAR, 
	SEQUENCER_STOP, 
	SEQUENCER_REWIND,
	MIDI, 
	ACTION, 
	CHANNEL_TOGGLE_READ_ACTIONS,
	CHANNEL_TOGGLE_ARM,
	CHANNEL_MUTE,
	CHANNEL_SOLO,
	CHANNEL_VOLUME,
	CHANNEL_PITCH,
	CHANNEL_PAN
};

struct Event
{
	EventType type;
	Frame     delta;
	Action    action;
};

/* EventBuffer
Alias for a RingBuffer containing events to be sent to channels. The double size
is due to the presence of two distinct Queues for collecting events coming from
other threads. See below. */

using EventBuffer = RingBuffer<Event, G_MAX_QUEUE_EVENTS * 2>;

constexpr int MASTER_OUT_CHANNEL_ID = 1;
constexpr int MASTER_IN_CHANNEL_ID  = 2;
constexpr int PREVIEW_CHANNEL_ID    = 3;

extern std::atomic<float> peakOut;
extern std::atomic<float> peakIn;

/* Channel Event queues
Collect events coming from the UI or MIDI devices to be sent to channels. Our 
poor's man Queue is a single-producer/single-consumer one, so we need two queues 
for two writers. TODO - let's add a multi-producer queue sooner or later! */

extern Queue<Event, G_MAX_QUEUE_EVENTS> UIevents;
extern Queue<Event, G_MAX_QUEUE_EVENTS> MidiEvents;

void init(Frame framesInSeq, Frame framesInBuffer);

/* enable, disable
Toggles master callback processing. Useful when loading a new patch. Mixer
will flush itself to wait for a processing cycle to finish when disable() is
called. */

void enable();
void disable();

/* allocRecBuffer
Allocates new memory for the virtual input channel. Call this whenever you 
shrink or resize the sequencer. */

void allocRecBuffer(Frame frames);

/* clearRecBuffer
Clears internal virtual channel. */

void clearRecBuffer();

/* getRecBuffer
Returns a read-only reference to the internal virtual channel. Use this to
merge data into channel after an input recording session. */

const AudioBuffer& getRecBuffer(); 

void close();

/* masterPlay
Core method (callback) */

int masterPlay(void* outBuf, void* inBuf, unsigned bufferSize, double streamTime,
	RtAudioStreamStatus status, void* userData);

/* startInputRec, stopInputRec
Starts/stops input recording on frame clock::getCurrentFrame(). */

void startInputRec();
void stopInputRec();

void setSignalCallback(std::function<void()> f);
}}} // giada::m::mixer::;


#endif
