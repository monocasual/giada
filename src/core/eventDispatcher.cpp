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

#include "eventDispatcher.h"
#include "core/clock.h"
#include "core/model/model.h"
#include "core/sequencer.h"
#include "core/worker.h"
#include "utils/log.h"
#include <functional>

namespace giada::m::eventDispatcher
{
namespace
{
Worker worker_;

/* eventBuffer_
Buffer of events sent to channels for event parsing. This is filled with Events
coming from the two event queues.*/

EventBuffer eventBuffer_;

/* -------------------------------------------------------------------------- */

void processFuntions_()
{
	for (const Event& e : eventBuffer_)
	{
		if (e.type == EventType::FUNCTION)
			std::get<std::function<void()>>(e.data)();
		G_DEBUG("Event type=" << (int)e.type << ", delta=" << e.delta << ", frame=" << clock::getCurrentFrame());
	}
}

/* -------------------------------------------------------------------------- */

void processChannels_()
{
	for (channel::Data& ch : model::get().channels)
		channel::react(ch, eventBuffer_, mixer::isChannelAudible(ch));
	model::swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void processSequencer_()
{
	sequencer::react(eventBuffer_);
}

/* -------------------------------------------------------------------------- */

void process_()
{
	eventBuffer_.clear();

	Event e;
	while (UIevents.pop(e))
		eventBuffer_.push_back(e);
	while (MidiEvents.pop(e))
		eventBuffer_.push_back(e);

	if (eventBuffer_.size() == 0)
		return;

	processFuntions_();
	processChannels_();
	processSequencer_();
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Queue<Event, G_MAX_DISPATCHER_EVENTS> UIevents;
Queue<Event, G_MAX_DISPATCHER_EVENTS> MidiEvents;

/* -------------------------------------------------------------------------- */

void init()
{
	worker_.start(process_, /*sleep=*/5); // TODO - <= than audio thread speed
}

/* -------------------------------------------------------------------------- */

void pumpEvent(Event e)
{
	/* TODO - two threads push events here: mixer/rt-thread and main thread, so
	this breaks the 1-producer 1-consumer rule! */
	UIevents.push(e);
}
} // namespace giada::m::eventDispatcher