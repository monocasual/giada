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

#ifndef G_CHANNEL_MIDI_LIGHTER_H
#define G_CHANNEL_MIDI_LIGHTER_H

#include "core/midiLearnParam.h"

namespace giada::m::channel
{
struct Data;
}
namespace giada::m::patch
{
struct Channel;
}
namespace giada::m::eventDispatcher
{
struct Event;
}
namespace giada::m::midiLighter
{
struct Data
{
	Data() = default;
	Data(const patch::Channel& p);
	Data(const Data& o) = default;

	/* enabled
    Tells whether MIDI lighting is enabled or not. */

	bool enabled;

	/* MIDI learning fields for MIDI lighting. */

	MidiLearnParam playing;
	MidiLearnParam mute;
	MidiLearnParam solo;
};

void react(channel::Data& ch, const eventDispatcher::Event& e, bool audible);
} // namespace giada::m::midiLighter

#endif
