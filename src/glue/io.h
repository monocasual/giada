/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#ifndef G_GLUE_IO_H
#define G_GLUE_IO_H


#include <atomic>
#include "core/types.h"
#include "core/midiEvent.h"


namespace giada {
namespace m
{
class Channel;
}
namespace c {
namespace io 
{
/* keyPress / keyRelease
Handle the key pressure, either via mouse/keyboard or MIDI. */

void keyPress  (ID channelId, bool ctrl, bool shift, int velocity);
void keyRelease(ID channelId, bool ctrl, bool shift);

/* setSampleChannelKey
Set key 'k' to Sample Channel 'channelId'. Used for keyboard bindings. */

void setSampleChannelKey(ID channelId, int k);

void midiLearn(m::MidiEvent e, std::atomic<uint32_t>& param, ID channelId);
}}} // giada::c::io::

#endif
