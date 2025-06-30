/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geScroll
 * Custom scroll with nice scrollbars and something else.
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

#include "scroll.h"
#include "boxtypes.h"
#include "deps/geompp/src/rect.hpp"
#include "gui/const.h"

namespace giada::v
{
geScroll::geScroll(int x, int y, int w, int h, int t)
: Fl_Scroll(x, y, w, h)
, autoscroll(false)
{
	end();
	type(t);

	scrollbar.color(G_COLOR_GREY_2);
	scrollbar.selection_color(G_COLOR_GREY_4);
	scrollbar.labelcolor(G_COLOR_LIGHT_1);
	scrollbar.slider(G_CUSTOM_BORDER_BOX);
	scrollbar.callback(cb_onScrollV, this);

	hscrollbar.color(G_COLOR_GREY_2);
	hscrollbar.selection_color(G_COLOR_GREY_4);
	hscrollbar.labelcolor(G_COLOR_LIGHT_1);
	hscrollbar.slider(G_CUSTOM_BORDER_BOX);
	hscrollbar.callback(cb_onScrollH, this);
}

/* -------------------------------------------------------------------------- */

geScroll::geScroll(int type)
: geScroll(0, 0, 0, 0, type)
{
}

/* -------------------------------------------------------------------------- */

int geScroll::handle(int e)
{
	if (e == FL_DRAG && autoscroll)
	{
		doAutoscroll();
		return 1;
	}
	return Fl_Scroll::handle(e);
}

/* -------------------------------------------------------------------------- */

geompp::Rect<int> geScroll::getViewportBounds() const
{
	const int hScrollbarH = hscrollbar.visible() ? hscrollbar.h() : 0;
	const int vScrollbarW = scrollbar.visible() ? scrollbar.w() : 0;
	return {x(), y(), w() - vScrollbarW, h() - hScrollbarH};
}

/* -------------------------------------------------------------------------- */

void geScroll::cb_onScrollV(Fl_Widget* w, void* p)
{
	geScroll*     s = static_cast<geScroll*>(w->parent());
	Fl_Scrollbar* b = static_cast<Fl_Scrollbar*>(w);

	s->scroll_to(s->xposition(), b->value());

	(static_cast<geScroll*>(p))->cb_onScrollV();
}

void geScroll::cb_onScrollH(Fl_Widget* w, void* p)
{
	geScroll*     s = static_cast<geScroll*>(w->parent());
	Fl_Scrollbar* b = static_cast<Fl_Scrollbar*>(w);

	s->scroll_to(b->value(), s->yposition());

	(static_cast<geScroll*>(p))->cb_onScrollH();
}

/* -------------------------------------------------------------------------- */

void geScroll::cb_onScrollV()
{
	if (onScrollV != nullptr)
		onScrollV(yposition());
}

/* -------------------------------------------------------------------------- */

void geScroll::cb_onScrollH()
{
	if (onScrollH != nullptr)
		onScrollH(xposition());
}

/* -------------------------------------------------------------------------- */

int geScroll::countChildren() const
{
	return children() - 2; // Exclude scrollbars
}
/* -------------------------------------------------------------------------- */

void geScroll::doAutoscroll()
{
	constexpr int       BORDER = 30;
	constexpr int       DELTA  = 10;
	const geompp::Rect  bounds(x(), y(), w(), h());
	const geompp::Point cursor(Fl::event_x(), Fl::event_y());

	if (bounds.withTrimmedRight(w() - BORDER).contains(cursor) && xposition() > 0) // Avoid negative x
		scroll_to(xposition() - DELTA, yposition());
	else if (bounds.withTrimmedLeft(w() - BORDER).contains(cursor))
		scroll_to(xposition() + DELTA, yposition());
	else if (bounds.withTrimmedTop(h() - BORDER).contains(cursor))
		scroll_to(xposition(), yposition() + DELTA);
	else if (bounds.withTrimmedBottom(h() - BORDER).contains(cursor) && yposition() > 0) // Avoid negative y
		scroll_to(xposition(), yposition() - DELTA);
}
} // namespace giada::v