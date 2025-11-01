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

#include "src/core/rendering/midiAdvance.h"
#include "src/core/channels/channel.h"
#include "src/core/rendering/midiOutput.h"
#include "src/core/rendering/midiReactions.h"

namespace giada::m::rendering
{
void advanceMidiChannel(const Channel& ch, const Sequencer::Event& e, KernelMidi& kernelMidi)
{
	switch (e.type)
	{
	case Sequencer::EventType::FIRST_BEAT:
		rewindMidiChannel(ch.shared->playStatus);
		break;

	case Sequencer::EventType::ACTIONS:
		if (ch.isPlaying())
			sendMidiFromActions(ch, e.scene, *e.actions, e.delta, kernelMidi);
		break;

	default:
		break;
	}
}
} // namespace giada::m::rendering
