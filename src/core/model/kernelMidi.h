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

#ifndef G_MODEL_KERNEL_MIDI_H
#define G_MODEL_KERNEL_MIDI_H

#include "core/kernelMidi.h"
#include "core/types.h"

namespace giada::m::model
{
struct KernelMidi
{
	RtMidi::Api system      = G_DEFAULT_MIDI_SYSTEM;
	int         portOut     = G_DEFAULT_MIDI_PORT_OUT;
	int         portIn      = G_DEFAULT_MIDI_PORT_IN;
	std::string midiMapPath = "";
	int         sync        = G_MIDI_SYNC_NONE;
};
} // namespace giada::m::model

#endif
