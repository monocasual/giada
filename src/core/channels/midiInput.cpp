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

#include "src/core/channels/midiInput.h"
#include "src/core/patch.h"

namespace giada::m
{
MidiInput::MidiInput()
: enabled(false)
, filter(-1)
{
}

/* -------------------------------------------------------------------------- */

MidiInput::MidiInput(const Patch::Channel& p)
: enabled(p.midiIn)
, filter(p.midiInFilter)
, keyPress(p.midiInKeyPress)
, keyRelease(p.midiInKeyRel)
, kill(p.midiInKill)
, arm(p.midiInArm)
, volume(p.midiInVolume)
, mute(p.midiInMute)
, solo(p.midiInSolo)
, readActions(p.midiInReadActions)
, pitch(p.midiInPitch)
{
}

/* -------------------------------------------------------------------------- */

bool MidiInput::isAllowed(int c) const
{
	return enabled && (filter == -1 || filter == c);
}
} // namespace giada::m