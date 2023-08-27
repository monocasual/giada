/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "flex.h"
#include <algorithm>
#include <cstddef>
#include <numeric>

namespace giada::v
{
int toFlDirection_(Direction d)
{
	return d == Direction::HORIZONTAL ? Fl_Flex::ROW : Fl_Flex::COLUMN;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geFlex::geFlex(int x, int y, int w, int h, Direction d, int gutter)
: Fl_Flex(x, y, w, h, toFlDirection_(d))
{
	Fl_Flex::end();
	Fl_Flex::gap(gutter);
}

/* -------------------------------------------------------------------------- */

geFlex::geFlex(geompp::Rect<int> r, Direction d, int gutter)
: geFlex(r.x, r.y, r.w, r.h, d, gutter)
{
}

/* -------------------------------------------------------------------------- */

geFlex::geFlex(Direction d, int gutter)
: geFlex(0, 0, 0, 0, d, gutter)
{
}

/* -------------------------------------------------------------------------- */

void geFlex::add(Fl_Widget& w, int size, geompp::Border<int> pad)
{
	Fl_Flex::add(w);
	Fl_Flex::fixed(w, size);
	Fl_Flex::margin(pad.left, pad.top, pad.right, pad.bottom); // TODO - this property is no longer per-child with Fl_Flex
}

void geFlex::add(Fl_Widget* w, int size, geompp::Border<int> pad)
{
	geFlex::add(*w, size, pad);
}

/* -------------------------------------------------------------------------- */

void geFlex::end()
{
	Fl_Flex::end();
	resize(x(), y(), w(), h());
}
} // namespace giada::v
