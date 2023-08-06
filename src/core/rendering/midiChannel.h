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

#include "core/channels/channelShared.h"
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

void sendMidiFromActions(const Channel&, const std::vector<Action>&, Frame delta, KernelMidi&);

/* sendMidiAllNotesOff
Sends a G_MIDI_ALL_NOTES_OFF event to the outside world and plug-ins. */

void sendMidiAllNotesOff(const Channel&, KernelMidi&);

/* sendMidiEventToPlugins
Enqueue MIDI event to to the MIDI queue, so that it will be processed later
on by the PluginHost. */

void sendMidiEventToPlugins(ChannelShared::MidiQueue&, const MidiEvent&);

/* prepareMidiBuffer 
Fills the JUCE MIDI buffer with events previously enqueued in the MidiQueue.
Returns a reference to the JUCE MIDI buffer for convenience. */

const juce::MidiBuffer& prepareMidiBuffer(ChannelShared&);
} // namespace giada::m::rendering

#endif
