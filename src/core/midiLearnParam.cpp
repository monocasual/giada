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


#include "midiLearnParam.h"


namespace giada::m
{
MidiLearnParam::MidiLearnParam()
: m_param(0)
, m_index(0) 
{
}


MidiLearnParam::MidiLearnParam(uint32_t v, std::size_t index)
: m_param(v)
, m_index(index) 
{
}


/* -------------------------------------------------------------------------- */


uint32_t MidiLearnParam::getValue() const
{
    return m_param.load();
}


void MidiLearnParam::setValue(uint32_t v)
{
    m_param.store(v);
}


std::size_t MidiLearnParam::getIndex() const
{
    return m_index;
}
} // giada::m::