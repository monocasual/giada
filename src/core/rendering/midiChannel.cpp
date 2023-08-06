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

void sendMidi_(MidiEvent e, int outputFilter, KernelMidi& kernelMidi)
{
	assert(onSend_ != nullptr);

	e.setChannel(outputFilter);
	kernelMidi.send(e);
	onSend_();
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

void sendMidiFromActions(ID channelId, const std::vector<Action>& actions, int outputFilter, KernelMidi& kernelMidi)
{
	for (const Action& a : actions)
		if (a.channelId == channelId)
			sendMidi_(a.event, outputFilter, kernelMidi);
}

/* -------------------------------------------------------------------------- */

void sendMidiAllNotesOff(int outputFilter, KernelMidi& kernelMidi)
{
	sendMidi_(MidiEvent::makeFromRaw(G_MIDI_ALL_NOTES_OFF, /*numBytes=*/3), outputFilter, kernelMidi);
}
} // namespace giada::m::rendering
