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


#include <algorithm>
#include "../action.h"
#include "recorderStack.h"


namespace giada {
namespace m {
namespace recorder
{
bool Stack::push(const Action* noteOn)
{
    Stack::Array::iterator it = find(nullptr);
    if (it == m_array.end()) {  // Stack full!
        m_full = true;
        return false;
    }
    *it = noteOn;
    return true;
}


/* -------------------------------------------------------------------------- */


const Action* Stack::pop(MidiEvent eventOff)
{
    const Action* out = nullptr;
    for (const Action* a : m_array)
        if (a != nullptr && eventOff.getNote() == a->event.getNote()) {
            out = a;
            clear(a);
            break;
        }
    return out;
}


/* -------------------------------------------------------------------------- */


void Stack::clear(const Action* noteOn)
{
    int i = indexOf(noteOn);
    if (i != -1) {
        m_array[i] = nullptr;
        m_full     = false;
    }
}


/* -------------------------------------------------------------------------- */


bool Stack::isFull() { return m_full; }


/* -------------------------------------------------------------------------- */


Stack::Array::iterator Stack::find(const Action* a)
{
    return std::find(m_array.begin(), m_array.end(), a);
}


/* -------------------------------------------------------------------------- */


int Stack::indexOf(const Action* a)
{
    Array::iterator it = find(a);
    if (it == m_array.end()) return -1;
    return std::distance(m_array.begin(), it);
}

}}}; // giada::m::recorder::


