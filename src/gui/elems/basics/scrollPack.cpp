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


#include <cassert>
#include "core/const.h"
#include "boxtypes.h"
#include "scrollPack.h"


namespace giada {
namespace v 
{
geScrollPack::geScrollPack(int x, int y, int w, int h, int type, Direction dir,
    int gutter)
: geScroll   (x, y, w, h, type)
, m_direction(dir)
, m_gutter   (gutter)
{
    end();
}


/* -------------------------------------------------------------------------- */


std::size_t geScrollPack::countChildren() const
{
    return m_widgets.size();
}


/* -------------------------------------------------------------------------- */


void geScrollPack::add(Fl_Widget* w)
{
    if (countChildren() == 0)
        w->position(x(), y());
    else
    if (m_direction == Direction::HORIZONTAL)
        w->position((getLastChild()->x() + getLastChild()->w() + m_gutter), y());
    else
        w->position(x(), (getLastChild()->y() + getLastChild()->h() + m_gutter));

    geScroll::add(w);
    m_widgets.push_back(w);
}


/* -------------------------------------------------------------------------- */


Fl_Widget* geScrollPack::getChild(std::size_t i)
{
    return m_widgets.at(i); // Throws std::out_of_range in case
}

/* -------------------------------------------------------------------------- */


Fl_Widget* geScrollPack::getLastChild()
{
    return m_widgets.at(m_widgets.size() - 1); // Throws std::out_of_range in case
}
}}