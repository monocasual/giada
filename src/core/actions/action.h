/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_ACTION_H
#define G_ACTION_H

#include "src/core/midiEvent.h"
#include "src/core/types.h"

namespace giada::m
{
struct Action
{
	ID        id = 0; // Invalid
	ID        channelId;
	Frame     frame;
	MidiEvent event;
	ID        pluginId    = -1;
	int       pluginParam = -1;
	ID        prevId      = 0;
	ID        nextId      = 0;

	bool isValid() const
	{
		return id != 0;
	}

	bool isVolumeEnvelope() const
	{
		return event.getStatus() == MidiEvent::CHANNEL_CC && pluginId == -1;
	}
};
} // namespace giada::m

#endif