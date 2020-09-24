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


#ifdef WITH_VST


#include "core/mixer.h"
#include "core/pluginHost.h"
#include "core/channels/state.h"
#include "midiReceiver.h"


namespace giada {
namespace m 
{
MidiReceiver::MidiReceiver(ChannelState* c)
: state           (std::make_unique<MidiReceiverState>())
, m_channelState  (c)
{
}


/* -------------------------------------------------------------------------- */


MidiReceiver::MidiReceiver(const patch::Channel& /*p*/, ChannelState* c)
: state           (std::make_unique<MidiReceiverState>())
, m_channelState  (c)
{
}


/* -------------------------------------------------------------------------- */


MidiReceiver::MidiReceiver(const MidiReceiver& /*o*/, ChannelState* c)
: state           (std::make_unique<MidiReceiverState>())
, m_channelState  (c)
{
}


/* -------------------------------------------------------------------------- */


void MidiReceiver::parse(const mixer::Event& e) const
{
	switch (e.type) {

		case mixer::EventType::MIDI:
			parseMidi(e.action.event); break;

		case mixer::EventType::ACTION:
			if (m_channelState->isPlaying())
				sendToPlugins(e.action.event, e.delta);
			break;
		
		case mixer::EventType::KEY_KILL:
		case mixer::EventType::SEQUENCER_STOP:
		case mixer::EventType::SEQUENCER_REWIND:
			sendToPlugins(MidiEvent(G_MIDI_ALL_NOTES_OFF), 0); break;
		
		default: break;
	}
}


/* -------------------------------------------------------------------------- */


void MidiReceiver::render(const std::vector<ID>& pluginIds) const
{
	pluginHost::processStack(m_channelState->buffer, pluginIds, &state->midiBuffer);
	state->midiBuffer.clear();
}


/* -------------------------------------------------------------------------- */


void MidiReceiver::parseMidi(const MidiEvent& e) const
{
	/* Now all messages are turned into Channel-0 messages. Giada doesn't care 
	about holding MIDI channel information. Moreover, having all internal 
	messages on channel 0 is way easier. Then send it to plug-ins. */

	MidiEvent flat(e);
	flat.setChannel(0);
	sendToPlugins(flat, /*delta=*/0); 
}


/* -------------------------------------------------------------------------- */


void MidiReceiver::sendToPlugins(const MidiEvent& e, Frame localFrame) const
{
	juce::MidiMessage message = juce::MidiMessage(
		e.getStatus(), 
		e.getNote(), 
		e.getVelocity());
	state->midiBuffer.addEvent(message, localFrame);
}
}} // giada::m::


#endif // WITH_VST