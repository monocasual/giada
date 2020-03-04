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


#include "core/mixer.h"
#include "core/kernelMidi.h"
#include "core/channels/state.h"
#include "midiSender.h"


namespace giada {
namespace m 
{
MidiSender::MidiSender(ChannelState* c)
: state(std::make_unique<MidiSenderState>())
, m_channelState(c)
{
}


/* -------------------------------------------------------------------------- */


MidiSender::MidiSender(const patch::Channel& p, ChannelState* c)
: state(std::make_unique<MidiSenderState>(p))
, m_channelState(c)
{
}


/* -------------------------------------------------------------------------- */


MidiSender::MidiSender(const MidiSender& o, ChannelState* c)
: state(std::make_unique<MidiSenderState>(*o.state))
, m_channelState(c)
{
}


/* -------------------------------------------------------------------------- */


void MidiSender::parse(const mixer::Event& e) const
{
	bool isPlaying = m_channelState->isPlaying();
	bool isEnabled = state->enabled.load();

	if (!isPlaying || !isEnabled)
		return;

	if (e.type == mixer::EventType::KEY_KILL || 
	    e.type == mixer::EventType::SEQUENCER_STOP)
		send(MidiEvent(G_MIDI_ALL_NOTES_OFF));
	else
	if (e.type == mixer::EventType::ACTION)
		send(e.action.event);
}


/* -------------------------------------------------------------------------- */


void MidiSender::send(MidiEvent e) const
{
	e.setChannel(state->filter.load());
	kernelMidi::send(e.getRaw());
}
}} // giada::m::
