/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gLiquidScroll
 * custom scroll that tells children to follow scroll's width when
 * resized. Thanks to Greg Ercolano from FLTK dev team.
 * http://seriss.com/people/erco/fltk/
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/elems/basics/liquidScroll.h"
#include "src/gui/const.h"
#include "src/gui/elems/basics/boxtypes.h"

namespace giada::v
{
geLiquidScroll::geLiquidScroll(int x, int y, int w, int h, Direction d, int scrollbarType)
: geScroll(x, y, w, h, d == Direction::VERTICAL ? Fl_Scroll::VERTICAL_ALWAYS : Fl_Scroll::HORIZONTAL_ALWAYS)
, m_direction(d)
{
	type(scrollbarType);
}

/* -------------------------------------------------------------------------- */

geLiquidScroll::geLiquidScroll(geompp::Rect<int> r, Direction d, int scrollbarType)
: geLiquidScroll(r.x, r.y, r.w, r.h, d, scrollbarType)
{
}

/* -------------------------------------------------------------------------- */

geLiquidScroll::geLiquidScroll(Direction d, int scrollbarType)
: geLiquidScroll(0, 0, 0, 0, d, scrollbarType)
{
}

/* -------------------------------------------------------------------------- */

void geLiquidScroll::resize(int X, int Y, int W, int H)
{
	const int scrollbarSpace = getScrollbarSpace();
	const int nc             = children() - 2; // skip hscrollbar and vscrollbar

	for (int t = 0; t < nc; t++) // tell children to resize to our new width
	{
		Fl_Widget* c = child(t);
		if (m_direction == Direction::VERTICAL)
			c->resize(c->x(), c->y(), W - scrollbarSpace, c->h());
		else
			c->resize(c->x(), c->y(), c->w(), H - scrollbarSpace);
	}
	init_sizes(); // tell scroll children changed in size
	Fl_Scroll::resize(X, Y, W, H);
}

/* -------------------------------------------------------------------------- */

void geLiquidScroll::addWidget(Fl_Widget* wg)
{
	const int numChildren = countChildren();

	const int wx = x();
	const int wy = y() - yposition() + (numChildren * (wg->h() + G_GUI_INNER_MARGIN));
	const int ww = w() - getScrollbarSpace();
	const int wh = wg->h();

	wg->resize(wx, wy, ww, wh);
	geScroll::add(wg);
	redraw();
}

/* -------------------------------------------------------------------------- */

int geLiquidScroll::getScrollbarSpace() const
{
	if (scrollbar.visible())
		return scrollbar.w() + G_GUI_OUTER_MARGIN;
	if (hscrollbar.visible())
		return scrollbar.h() + G_GUI_OUTER_MARGIN;
	return 0;
}
} // namespace giada::v