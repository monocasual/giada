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

#ifndef G_CHANNEL_MIDI_CONTROLLER_H
#define G_CHANNEL_MIDI_CONTROLLER_H

#include "core/sequencer.h"

namespace giada::m::channel
{
struct Data;
}

namespace giada::m::midiController
{
struct Data
{
	void advance(const channel::Data& ch, const Sequencer::Event& e) const;
	void react(channel::Data& ch, const EventDispatcher::Event& e) const;

private:
	ChannelStatus onFirstBeat(const channel::Data& ch) const;
	ChannelStatus press(const channel::Data& ch) const;
};
} // namespace giada::m::midiController

#endif
