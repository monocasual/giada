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

#ifndef G_EVENT_DISPATCHER_H
#define G_EVENT_DISPATCHER_H

#include "core/action.h"
#include "core/const.h"
#include "core/queue.h"
#include "core/ringBuffer.h"
#include "core/types.h"
#include <atomic>
#include <functional>
#include <thread>
#include <variant>

/* giada::m::eventDispatcher
Takes events from the two queues (MIDI and UI) filled by c::events and turns 
them into actual changes in the data model. The EventDispatcher runs in a
separate worker thread. */

namespace giada::m::eventDispatcher
{
enum class EventType
{
	KEY_PRESS,
	KEY_RELEASE,
	KEY_KILL,
	SEQUENCER_START,
	SEQUENCER_STOP,
	SEQUENCER_REWIND,
	MIDI,
	CHANNEL_TOGGLE_READ_ACTIONS,
	CHANNEL_KILL_READ_ACTIONS,
	CHANNEL_TOGGLE_ARM,
	CHANNEL_MUTE,
	CHANNEL_SOLO,
	CHANNEL_VOLUME,
	CHANNEL_PITCH,
	CHANNEL_PAN,
	FUNCTION
};

using EventData = std::variant<int, float, Action, std::function<void()>>;

struct Event
{
	EventType type;
	Frame     delta     = 0;
	ID        channelId = 0;
	EventData data;
};

/* EventBuffer
Alias for a RingBuffer containing events to be sent to engine. The double size
is due to the presence of two distinct Queues for collecting events coming from
other threads. See below. */

using EventBuffer = RingBuffer<Event, G_MAX_DISPATCHER_EVENTS * 2>;

/* Event queues
Collect events coming from the UI or MIDI devices. Our poor's man Queue is a 
single-producer/single-consumer one, so we need two queues for two writers. 
TODO - let's add a multi-producer queue sooner or later! */

extern Queue<Event, G_MAX_DISPATCHER_EVENTS> UIevents;
extern Queue<Event, G_MAX_DISPATCHER_EVENTS> MidiEvents;

void init();

void pumpUIevent(Event e);
void pumpMidiEvent(Event e);
} // namespace giada::m::eventDispatcher

#endif
