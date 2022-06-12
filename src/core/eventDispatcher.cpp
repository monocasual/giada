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
: onMidiLearn(nullptr)
, onMidiProcess(nullptr)
, onProcessChannels(nullptr)
, onProcessSequencer(nullptr)
, onMixerSignalCallback(nullptr)
, onMixerEndOfRecCallback(nullptr)
{
}

/* -------------------------------------------------------------------------- */

void EventDispatcher::start()
{
	m_worker.start([this]() { process(); }, /*sleep=*/G_EVENT_DISPATCHER_RATE_MS);
}

/* -------------------------------------------------------------------------- */

void EventDispatcher::pumpUIevent(Event e) { UIevents.push(e); }
void EventDispatcher::pumpMidiEvent(Event e) { MidiEvents.push(e); }

/* -------------------------------------------------------------------------- */

void EventDispatcher::processFunctions()
{
	assert(onMidiLearn != nullptr);
	assert(onMidiProcess != nullptr);
	assert(onMixerSignalCallback != nullptr);
	assert(onMixerEndOfRecCallback != nullptr);

	for (const Event& e : m_eventBuffer)
	{
		switch (e.type)
		{
		case EventType::MIDI_DISPATCHER_LEARN:
			onMidiLearn(std::get<Action>(e.data).event);
			break;

		case EventType::MIDI_DISPATCHER_PROCESS:
			onMidiProcess(std::get<Action>(e.data).event);
			break;

		case EventType::MIXER_SIGNAL_CALLBACK:
			onMixerSignalCallback();
			break;

		case EventType::MIXER_END_OF_REC_CALLBACK:
			onMixerEndOfRecCallback();
			break;

		default:
			break;
		}
	}
}

/* -------------------------------------------------------------------------- */

void EventDispatcher::process()
{
	assert(onProcessChannels != nullptr);
	assert(onProcessSequencer != nullptr);

	m_eventBuffer.clear();

	Event e;
	while (UIevents.pop(e))
		m_eventBuffer.push_back(e);
	while (MidiEvents.pop(e))
		m_eventBuffer.push_back(e);

	if (m_eventBuffer.size() == 0)
		return;

	processFunctions();
	onProcessChannels(m_eventBuffer);
	onProcessSequencer(m_eventBuffer);
}
} // namespace giada::m