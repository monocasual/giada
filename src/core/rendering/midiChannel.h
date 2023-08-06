/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_RENDERING_MIDI_CHANNEL_H
#define G_RENDERING_MIDI_CHANNEL_H

#include "core/midiEvent.h"

namespace giada::m
{
struct Action;
class KernelMidi;
} // namespace giada::m

namespace giada::m::rendering
{
/* registerOnSendMidiCb
Callback fired when a MIDI signal has been sent. */

void registerOnSendMidiCb(std::function<void()>);

/* sendMidiFromActions
Sends a corresponding MIDI event for each action in the action vector. */

void sendMidiFromActions(ID channelId, const std::vector<Action>&, int outputFilter, KernelMidi&);

/* sendMidiAllNotesOff
Sends a G_MIDI_ALL_NOTES_OFF event to the outside world. */

void sendMidiAllNotesOff(int outputFilter, KernelMidi&);

} // namespace giada::m::rendering

#endif
