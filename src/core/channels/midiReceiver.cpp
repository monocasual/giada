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

#ifdef WITH_VST

#include "midiReceiver.h"
#include "core/channels/channel.h"
#include "core/eventDispatcher.h"
#include "core/mixer.h"
#include "core/plugins/pluginHost.h"

namespace giada::m::midiReceiver
{
namespace
{
void sendToPlugins_(const channel::Data& ch, const MidiEvent& e, Frame localFrame)
{
	ch.buffer->midiQueue.push(MidiEvent(e.getRaw(), localFrame));
}

/* -------------------------------------------------------------------------- */

void parseMidi_(const channel::Data& ch, const MidiEvent& e)
{
	/* Now all messages are turned into Channel-0 messages. Giada doesn't care 
	about holding MIDI channel information. Moreover, having all internal 
	messages on channel 0 is way easier. Then send it to plug-ins. */

	MidiEvent flat(e);
	flat.setChannel(0);
	sendToPlugins_(ch, flat, /*delta=*/0);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void react(const channel::Data& ch, const eventDispatcher::Event& e)
{
	switch (e.type)
	{
	case eventDispatcher::EventType::MIDI:
		parseMidi_(ch, std::get<Action>(e.data).event);
		break;

	case eventDispatcher::EventType::KEY_KILL:
	case eventDispatcher::EventType::SEQUENCER_STOP:
	case eventDispatcher::EventType::SEQUENCER_REWIND:
		sendToPlugins_(ch, MidiEvent(G_MIDI_ALL_NOTES_OFF), 0);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void advance(const channel::Data& ch, const sequencer::Event& e)
{
	if (e.type == sequencer::EventType::ACTIONS && ch.isPlaying())
		for (const Action& action : *e.actions)
			if (action.channelId == ch.id)
				sendToPlugins_(ch, action.event, e.delta);
}

/* -------------------------------------------------------------------------- */

void render(const channel::Data& ch)
{
	ch.buffer->midi.clear();

	MidiEvent e;
	while (ch.buffer->midiQueue.pop(e))
	{
		juce::MidiMessage message = juce::MidiMessage(
		    e.getStatus(),
		    e.getNote(),
		    e.getVelocity());
		ch.buffer->midi.addEvent(message, e.getDelta());
	}

	pluginHost::processStack(ch.buffer->audio, ch.plugins, &ch.buffer->midi);
}
} // namespace giada::m::midiReceiver

#endif // WITH_VST
