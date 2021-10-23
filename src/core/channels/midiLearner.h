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

#ifndef G_CHANNEL_MIDI_LEARNER_H
#define G_CHANNEL_MIDI_LEARNER_H

#include "core/midiLearnParam.h"
#include "core/patch.h"

namespace giada::m::midiLearner
{
struct Data
{
	Data();
	Data(const Patch::Channel&);
	Data(const Data&) = default;

	/* isAllowed
    Tells whether the MIDI channel 'c' is enabled to receive MIDI data. */

	bool isAllowed(int c) const;

	/* enabled
    Tells whether MIDI learning is enabled for the current channel. */

	bool enabled;

	/* filter
    Which MIDI channel should be filtered out when receiving MIDI messages. 
    If -1 means 'all'. */

	int filter;

	/* MIDI learning fields. */

	MidiLearnParam keyPress;
	MidiLearnParam keyRelease;
	MidiLearnParam kill;
	MidiLearnParam arm;
	MidiLearnParam volume;
	MidiLearnParam mute;
	MidiLearnParam solo;
	MidiLearnParam readActions; // Sample Channels only
	MidiLearnParam pitch;       // Sample Channels only
};
} // namespace giada::m::midiLearner

#endif
