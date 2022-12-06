/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/eventDispatcher.h"
#include "core/const.h"
#include <cassert>

namespace giada::m
{
EventDispatcher::EventDispatcher()
: m_worker(G_EVENT_DISPATCHER_RATE_MS)
, m_eventQueue(G_MAX_DISPATCHER_EVENTS)
{
}

/* -------------------------------------------------------------------------- */

void EventDispatcher::start()
{
	m_worker.start([this]() { process(); });
}

/* -------------------------------------------------------------------------- */

bool EventDispatcher::pumpEvent(const std::function<void()>& f)
{
	return m_eventQueue.try_enqueue(f);
}

/* -------------------------------------------------------------------------- */

void EventDispatcher::process()
{
	Event e;
	while (m_eventQueue.try_dequeue(e))
		e();
}
} // namespace giada::m