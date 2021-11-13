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

namespace giada::m
{
void MidiReceiver::react(const Channel& ch, const EventDispatcher::Event& e) const
{
	switch (e.type)
	{
	case EventDispatcher::EventType::MIDI:
		parseMidi(ch, std::get<Action>(e.data).event);
		break;

	case EventDispatcher::EventType::KEY_KILL:
	case EventDispatcher::EventType::SEQUENCER_STOP:
	case EventDispatcher::EventType::SEQUENCER_REWIND:
		sendToPlugins(ch, MidiEvent(G_MIDI_ALL_NOTES_OFF), 0);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void MidiReceiver::advance(const Channel& ch, const Sequencer::Event& e) const
{
	if (e.type == Sequencer::EventType::ACTIONS && ch.isPlaying())
		for (const Action& action : *e.actions)
			if (action.channelId == ch.id)
				sendToPlugins(ch, action.event, e.delta);
}

/* -------------------------------------------------------------------------- */

void MidiReceiver::render(const Channel& ch, PluginHost& pluginHost) const
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

	pluginHost.processStack(ch.buffer->audio, ch.plugins, &ch.buffer->midi);
}

/* -------------------------------------------------------------------------- */

void MidiReceiver::sendToPlugins(const Channel& ch, const MidiEvent& e, Frame localFrame) const
{
	ch.buffer->midiQueue.push(MidiEvent(e.getRaw(), localFrame));
}

/* -------------------------------------------------------------------------- */

void MidiReceiver::parseMidi(const Channel& ch, const MidiEvent& e) const
{
	/* Now all messages are turned into Channel-0 messages. Giada doesn't care 
	about holding MIDI channel information. Moreover, having all internal 
	messages on channel 0 is way easier. Then send it to plug-ins. */

	MidiEvent flat(e);
	flat.setChannel(0);
	sendToPlugins(ch, flat, /*delta=*/0);
}
} // namespace giada::m

#endif // WITH_VST
