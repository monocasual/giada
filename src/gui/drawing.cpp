/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "drawing.h"
#include <FL/Fl.H>

namespace giada::v
{
void drawRectf(geompp::Rect<int> r, Fl_Color c)
{
	fl_rectf(r.x, r.y, r.w, r.h, c);
}

/* -------------------------------------------------------------------------- */

void drawRect(geompp::Rect<int> r, Fl_Color c)
{
	fl_rect(r.x, r.y, r.w, r.h, c);
}

/* -------------------------------------------------------------------------- */

void drawLine(geompp::Line<int> l, Fl_Color c)
{
	fl_color(c);
	fl_line(l.x1, l.y1, l.x2, l.y2);
}
} // namespace giada::v
