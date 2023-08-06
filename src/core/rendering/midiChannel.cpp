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

#include "core/rendering/midiChannel.h"
#include "core/kernelMidi.h"
#include <cassert>

namespace giada::m::rendering
{
namespace
{
std::function<void()> onSend_ = nullptr;

/* -------------------------------------------------------------------------- */

void sendMidiToOut_(MidiEvent e, int outputFilter, KernelMidi& kernelMidi)
{
	assert(onSend_ != nullptr);

	e.setChannel(outputFilter);
	kernelMidi.send(e);
	onSend_();
}

/* -------------------------------------------------------------------------- */

void sendMidiToPlugins_(ChannelShared::MidiQueue& midiQueue, const MidiEvent& e, Frame localFrame)
{
	MidiEvent eWithDelta(e);
	eWithDelta.setDelta(localFrame);
	midiQueue.push(eWithDelta);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void registerOnSendMidiCb(std::function<void()> f)
{
	onSend_ = f;
}

/* -------------------------------------------------------------------------- */

void sendMidiFromActions(const Channel& ch, const std::vector<Action>& actions, Frame delta, KernelMidi& kernelMidi)
{
	for (const Action& action : actions)
	{
		if (action.channelId != ch.id)
			continue;
		sendMidiToPlugins_(ch.shared->midiQueue, action.event, delta);
		if (ch.canSendMidi())
			sendMidiToOut_(action.event, ch.midiChannel->outputFilter, kernelMidi);
	}
}

/* -------------------------------------------------------------------------- */

void sendMidiAllNotesOff(int outputFilter, KernelMidi& kernelMidi)
{
	sendMidiToOut_(MidiEvent::makeFromRaw(G_MIDI_ALL_NOTES_OFF, /*numBytes=*/3), outputFilter, kernelMidi);
}

/* -------------------------------------------------------------------------- */

void sendMidiEventToPlugins(ChannelShared::MidiQueue& midiQueue, const MidiEvent& e)
{
	/* Now all messages are turned into Channel-0 messages. Giada doesn't care 
	about holding MIDI channel information. Moreover, having all internal 
	messages on channel 0 is way easier. Then send it to plug-ins. */

	MidiEvent flat(e);
	flat.setChannel(0);
	sendMidiToPlugins_(midiQueue, flat, /*delta=*/0);
}

/* -------------------------------------------------------------------------- */

void sendMidiAllNotesOffToPlugins(ChannelShared::MidiQueue& midiQueue)
{
	sendMidiToPlugins_(midiQueue, MidiEvent::makeFromRaw(G_MIDI_ALL_NOTES_OFF, /*numBytes=*/3), 0);
}

/* -------------------------------------------------------------------------- */

const juce::MidiBuffer& prepareMidiBuffer(ChannelShared& shared)
{
	shared.midiBuffer.clear();

	MidiEvent e;
	while (shared.midiQueue.pop(e))
	{
		juce::MidiMessage message = juce::MidiMessage(
		    e.getStatus(),
		    e.getNote(),
		    e.getVelocity());
		shared.midiBuffer.addEvent(message, e.getDelta());
	}

	return shared.midiBuffer;
}
} // namespace giada::m::rendering
