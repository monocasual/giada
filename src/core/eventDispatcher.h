/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/worker.h"
#include "deps/concurrentqueue/concurrentqueue.h"
#include <functional>

/* giada::m::EventDispatcher
Performs Events (a.k.a. function callbacks) in a separate worker thread. Used by
the realtime thread (via Engine) to talk to other non-realtime threads. */

namespace giada::m
{
class EventDispatcher
{
public:
	/* Event
	Alias for a std::function type. All events are just functions to be performed
	by the EventDispatcher. */

	using Event = std::function<void()>;

	EventDispatcher();

	/* start
	Starts the internal worker on a separate thread. Call this on startup. */

	void start();

	/* pumpEvent
	Inserts a new event in the event queue. Returns false if the queue is full. */

	bool pumpEvent(const Event&);

private:
	void process();

	/* m_worker
	A separate thread responsible for the event processing. */

	Worker m_worker;

	/* m_eventQueue
	Collects events coming from the UI or MIDI devices. */

	moodycamel::ConcurrentQueue<Event> m_eventQueue;
};
} // namespace giada::m

#endif
