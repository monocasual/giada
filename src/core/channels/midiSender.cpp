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

#include "midiSender.h"
#include "core/channels/channel.h"
#include "core/kernelMidi.h"
#include "core/mixer.h"

namespace giada::m::midiSender
{
namespace
{
void send_(const channel::Data& ch, MidiEvent e)
{
	e.setChannel(ch.midiSender->filter);
	kernelMidi::send(e.getRaw());
}

/* -------------------------------------------------------------------------- */

void parseActions_(const channel::Data& ch, const std::vector<Action>& as)
{
	for (const Action& a : as)
		if (a.channelId == ch.id)
			send_(ch, a.event);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data::Data(const patch::Channel& p)
: enabled(p.midiOut)
, filter(p.midiOutChan)
{
}

/* -------------------------------------------------------------------------- */

void react(const channel::Data& ch, const eventDispatcher::Event& e)
{
	if (!ch.isPlaying() || !ch.midiSender->enabled)
		return;

	if (e.type == eventDispatcher::EventType::KEY_KILL ||
	    e.type == eventDispatcher::EventType::SEQUENCER_STOP)
		send_(ch, MidiEvent(G_MIDI_ALL_NOTES_OFF));
}

/* -------------------------------------------------------------------------- */

void advance(const channel::Data& ch, const sequencer::Event& e)
{
	if (!ch.isPlaying() || !ch.midiSender->enabled)
		return;
	if (e.type == sequencer::EventType::ACTIONS)
		parseActions_(ch, *e.actions);
}
} // namespace giada::m::midiSender
