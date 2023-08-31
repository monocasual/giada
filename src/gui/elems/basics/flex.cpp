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

geFlex::geFlex(int x, int y, int w, int h, Direction d, int gutter, geompp::Border<int> pad)
: Fl_Flex(x, y, w, h, toFlDirection_(d))
{
	Fl_Flex::end();
	Fl_Flex::gap(gutter);
	Fl_Flex::margin(pad.left, pad.top, pad.right, pad.bottom);
}

/* -------------------------------------------------------------------------- */

geFlex::geFlex(geompp::Rect<int> r, Direction d, int gutter, geompp::Border<int> pad)
: geFlex(r.x, r.y, r.w, r.h, d, gutter, pad)
{
}

/* -------------------------------------------------------------------------- */

geFlex::geFlex(Direction d, int gutter, geompp::Border<int> pad)
: geFlex(0, 0, 0, 0, d, gutter, pad)
{
}

/* -------------------------------------------------------------------------- */

void geFlex::addWidget(Fl_Widget& w, int size)
{
	Fl_Flex::add(w);
	Fl_Flex::fixed(w, size);
}

void geFlex::addWidget(Fl_Widget* w, int size)
{
	geFlex::addWidget(*w, size);
}
} // namespace giada::v
