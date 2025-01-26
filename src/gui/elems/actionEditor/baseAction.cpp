/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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

#include "baseAction.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

namespace giada::v
{
geBaseAction::geBaseAction(Pixel X, Pixel Y, Pixel W, Pixel H, bool resizable,
    m::Action a1, m::Action a2)
: Fl_Box(X, Y, W, H)
, onRightEdge(false)
, onLeftEdge(false)
, hovered(false)
, altered(false)
, pick(0)
, a1(a1)
, a2(a2)
, m_resizable(resizable)
{
	if (w() < MIN_WIDTH)
		size(MIN_WIDTH, h());
}

/* -------------------------------------------------------------------------- */

int geBaseAction::handle(int e)
{
	switch (e)
	{
	case FL_ENTER:
	{
		hovered = true;
		redraw();
		return 1;
	}
	case FL_LEAVE:
	{
		fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
		hovered = false;
		redraw();
		return 1;
	}
	case FL_MOVE:
	{
		if (m_resizable)
		{
			onLeftEdge  = false;
			onRightEdge = false;
			if (Fl::event_x() >= x() && Fl::event_x() < x() + HANDLE_WIDTH)
			{
				onLeftEdge = true;
				fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
			}
			else if (Fl::event_x() >= x() + w() - HANDLE_WIDTH &&
			         Fl::event_x() <= x() + w())
			{
				onRightEdge = true;
				fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
			}
			else
				fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
		}
		return 1;
	}
	default:
		return Fl_Widget::handle(e);
	}
}

/* -------------------------------------------------------------------------- */

void geBaseAction::setLeftEdge(Pixel p)
{
	resize(p, y(), x() - p + w(), h());
	if (w() < MIN_WIDTH)
		size(MIN_WIDTH, h());
}

/* -------------------------------------------------------------------------- */

void geBaseAction::setRightEdge(Pixel p)
{
	size(p, h());
	if (w() < MIN_WIDTH)
		size(MIN_WIDTH, h());
}

/* -------------------------------------------------------------------------- */

void geBaseAction::setPosition(Pixel p)
{
	position(p, y());
}

/* -------------------------------------------------------------------------- */

bool geBaseAction::isOnEdges() const
{
	return onLeftEdge || onRightEdge;
}
} // namespace giada::v
