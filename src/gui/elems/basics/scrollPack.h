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

#ifndef GE_SCROLL_PACK_H
#define GE_SCROLL_PACK_H

#include "gui/elems/basics/pack.h"
#include "gui/elems/basics/scroll.h"

namespace giada
{
namespace v
{
/* geScrollPack
A scrollable viewport that contains packed widgets. */

class geScrollPack : public geScroll
{
  public:
	geScrollPack(int x, int y, int w, int h, int type = Fl_Scroll::BOTH,
	    Direction d = Direction::HORIZONTAL, int gutter = G_GUI_INNER_MARGIN);

	/* countChildren
    Returns the number of widgets contained in this group. */

	std::size_t countChildren() const;

	void add(Fl_Widget* w);

	Fl_Widget* getChild(std::size_t i);
	Fl_Widget* getLastChild();

  private:
	/* m_widgets 
    The internal Fl_Scroll::array_ is unreliable when inspected with the child()
    method. Let's keep track of widgets that belong to this group manually. */

	std::vector<Fl_Widget*> m_widgets;

	Direction m_direction;
	int       m_gutter;
};
} // namespace v
} // namespace giada

#endif
