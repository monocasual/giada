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

#include "core/channels/midiSender.h"
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
	ch.midiSender->kernelMidi->send(e.getRaw());
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

Data::Data(KernelMidi& k)
: kernelMidi(&k)
, enabled(false)
, filter(0)
{
}

/* -------------------------------------------------------------------------- */

Data::Data(const Patch::Channel& p, KernelMidi& k)
: kernelMidi(&k)
, enabled(p.midiOut)
, filter(p.midiOutChan)
{
}

/* -------------------------------------------------------------------------- */

void react(const channel::Data& ch, const EventDispatcher::Event& e)
{
	if (!ch.isPlaying() || !ch.midiSender->enabled || ch.isMuted())
		return;

	if (e.type == EventDispatcher::EventType::KEY_KILL ||
	    e.type == EventDispatcher::EventType::SEQUENCER_STOP)
		send_(ch, MidiEvent(G_MIDI_ALL_NOTES_OFF));
}

/* -------------------------------------------------------------------------- */

void advance(const channel::Data& ch, const Sequencer::Event& e)
{
	if (!ch.isPlaying() || !ch.midiSender->enabled || ch.isMuted())
		return;
	if (e.type == Sequencer::EventType::ACTIONS)
		parseActions_(ch, *e.actions);
}
} // namespace giada::m::midiSender