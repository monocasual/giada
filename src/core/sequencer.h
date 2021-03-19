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

#ifndef G_SEQUENCER_H
#define G_SEQUENCER_H

#include "core/eventDispatcher.h"
#include "core/quantizer.h"
#include <vector>

namespace giada::m
{
class AudioBuffer;
}
namespace giada::m::sequencer
{
enum class EventType
{
	NONE,
	FIRST_BEAT,
	BAR,
	REWIND,
	ACTIONS
};

struct Event
{
	EventType                  type    = EventType::NONE;
	Frame                      global  = 0;
	Frame                      delta   = 0;
	const std::vector<Action>* actions = nullptr;
};

using EventBuffer = RingBuffer<Event, G_MAX_SEQUENCER_EVENTS>;

/* quantizer
Used by the sequencer itself and each sample channel. */

extern Quantizer quantizer;

void init();

/* react
Reacts to live events coming from the EventDispatcher (human events). */

void react(const eventDispatcher::EventBuffer& e);

/* advance
Parses sequencer events that might occur in a block and advances the internal 
quantizer. Returns a reference to the internal EventBuffer filled with events
(if any). Call this on each new audio block. */

const EventBuffer& advance(Frame bufferSize);

/* render
Renders audio coming out from the sequencer: that is, the metronome! */

void render(AudioBuffer& outBuf);

void start();
void stop();
void rewind();

bool isMetronomeOn();
void toggleMetronome();
void setMetronome(bool v);
} // namespace giada::m::sequencer

#endif
