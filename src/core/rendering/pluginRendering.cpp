/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/rendering/pluginRendering.h"
#include "core/channels/channel.h"
#include "core/plugins/pluginHost.h"

namespace giada::m::rendering
{
namespace
{
/* prepareMidiBuffer_
Fills the JUCE MIDI buffer with events previously enqueued in the MidiQueue.
Returns a reference to the JUCE MIDI buffer for convenience. */

const juce::MidiBuffer& prepareMidiBuffer_(ChannelShared& shared)
{
	shared.midiBuffer.clear();

	MidiEvent e;
	while (shared.midiQueue.try_dequeue(e))
	{
		juce::MidiMessage message = juce::MidiMessage(
		    e.getStatus(),
		    e.getNote(),
		    e.getVelocity());
		shared.midiBuffer.addEvent(message, e.getDelta());
	}

	return shared.midiBuffer;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void renderAudioAndMidiPlugins(const Channel& ch, PluginHost& pluginHost)
{
	pluginHost.processStack(ch.shared->audioBuffer, ch.plugins, &prepareMidiBuffer_(*ch.shared));
	ch.shared->midiBuffer.clear();
}

/* -------------------------------------------------------------------------- */

void renderAudioPlugins(const Channel& ch, PluginHost& pluginHost)
{
	pluginHost.processStack(ch.shared->audioBuffer, ch.plugins, nullptr);
}
} // namespace giada::m::rendering
