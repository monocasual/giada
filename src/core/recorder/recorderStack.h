/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#ifndef G_RECORDER_STACK_H
#define G_RECORDER_STACK_H


#include <array>
#include "../midiEvent.h"
#include "../const.h"


namespace giada {
namespace m 
{
class Action;

namespace recorder
{
class Stack
{
public: 

    /* Push
    Pushes the element 'noteOn' in the stack. Return false if the stack is 
    full. */

    bool push(const Action* noteOn);

    const Action* pop(MidiEvent eventOff);

    void clear(const Action* noteOn);

    bool isFull();

private:

    using Array = std::array<const Action*, G_MAX_POLYPHONY>;

    Array m_array = {};
    bool  m_full  = false;

    Array::iterator find(const Action* a);
    int indexOf(const Action* a);
};
}}}; // giada::m::recorder::


#endif
