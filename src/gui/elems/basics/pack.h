/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geScroll
 * Custom scroll with nice scrollbars and something else.
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


#ifndef GE_PACK_H
#define GE_PACK_H


#include "core/const.h"
#include "gui/elems/basics/group.h"


namespace giada {
namespace v 
{
enum class Direction { HORIZONTAL, VERTICAL };

/* gePack
A stack of widgets that resize itself according to its content. */

class gePack : public geGroup
{
public:

	gePack(int x, int y, Direction d, int gutter=G_GUI_INNER_MARGIN);

    /* add
    Adds a Fl_Widget 'w' to this pack. Coordinates are relative to the group,
    so origin starts at (0, 0). */

    void add(Fl_Widget* w);

private:

    Direction m_direction;
    int       m_gutter;
};
}}


#endif
