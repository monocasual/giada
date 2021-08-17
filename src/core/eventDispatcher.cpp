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
#include "core/const.h"
#include "core/midiDispatcher.h"
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
		switch (e.type)
		{
		case EventType::MIDI_DISPATCHER_LEARN:
			midiDispatcher::learn(std::get<Action>(e.data).event);
			break;

		case EventType::MIDI_DISPATCHER_PROCESS:
			midiDispatcher::process(std::get<Action>(e.data).event);
			break;

		case EventType::MIXER_SIGNAL_CALLBACK:
			mixer::execSignalCb();
			break;

		case EventType::MIXER_END_OF_REC_CALLBACK:
			mixer::execEndOfRecCb();
			break;

		default:
			break;
		}
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
	worker_.start(process_, /*sleep=*/G_EVENT_DISPATCHER_RATE_MS);
}

/* -------------------------------------------------------------------------- */

void pumpUIevent(Event e) { UIevents.push(e); }
void pumpMidiEvent(Event e) { MidiEvents.push(e); }
} // namespace giada::m::eventDispatcher