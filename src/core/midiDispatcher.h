/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#ifndef G_MIDI_DISPATCHER_H
#define G_MIDI_DISPATCHER_H


#ifdef __APPLE__  // our Clang still doesn't know about cstdint (c++11 stuff)
	#include <stdint.h>
#else
	#include <cstdint>
#endif


namespace giada {
namespace m {
namespace midiDispatcher
{
typedef void (cb_midiLearn) (uint32_t, void*);

void startMidiLearn(cb_midiLearn* cb, void* data);
void stopMidiLearn();

void dispatch(int byte1, int byte2, int byte3);

}}}; // giada::m::midiDispatcher::


#endif
