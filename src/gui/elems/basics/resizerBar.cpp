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

#include "resizerBar.h"
#include "gui/const.h"
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include <cassert>
#include <cstddef>
#include <vector>

namespace giada::v
{
geResizerBar::geResizerBar(int X, int Y, int W, int H, int minSize, Direction dir, Mode mode)
: Fl_Box(X, Y, W, H)
, m_direction(dir)
, m_mode(mode)
, m_minSize(minSize)
, m_lastPos(0)
, m_hover(false)
{
	visible_focus(0);
}

/* -------------------------------------------------------------------------- */

void geResizerBar::handleDrag(int diff)
{
	m_mode == Mode::MOVE ? move(diff) : resize(diff);

	Fl_Group* group = static_cast<Fl_Group*>(parent());
	group->init_sizes();
	group->redraw();
}

/* -------------------------------------------------------------------------- */

void geResizerBar::move(int diff)
{
	Fl_Widget&              wfirst  = getFirstWidget();
	std::vector<Fl_Widget*> wothers = findWidgets([this](const Fl_Widget& wd)
	{ return isAfter(wd); });

	if (m_direction == Direction::VERTICAL)
	{
		if (wfirst.h() + diff < m_minSize)
			diff = 0;
		wfirst.resize(wfirst.x(), wfirst.y(), wfirst.w(), wfirst.h() + diff);
		for (Fl_Widget* wd : wothers)
			wd->resize(wd->x(), wd->y() + diff, wd->w(), wd->h());
		resize(x(), y() + diff, w(), h());
	}
	else if (m_direction == Direction::HORIZONTAL)
	{
		if (wfirst.w() + diff < m_minSize)
			diff = 0;
		wfirst.resize(wfirst.x(), wfirst.y(), wfirst.w() + diff, wfirst.h());
		for (Fl_Widget* wd : wothers)
			wd->resize(wd->x() + diff, wd->y(), wd->w(), wd->h());
		resize(x() + diff, y(), w(), h());
	}
}

/* -------------------------------------------------------------------------- */

void geResizerBar::resize(int diff)
{
	Fl_Widget& wa = getFirstWidget();
	Fl_Widget& wb = *findWidgets([this](const Fl_Widget& wd)
	{ return isAfter(wd); }, /*howmany=*/1)[0];

	if (m_direction == Direction::VERTICAL)
	{
		if (wa.h() + diff < m_minSize || wb.h() - diff < m_minSize)
			diff = 0;
		wa.resize(wa.x(), wa.y(), wa.w(), wa.h() + diff);
		wb.resize(wb.x(), wb.y() + diff, wb.w(), wb.h() - diff);
		resize(x(), y() + diff, w(), h());
	}
	else if (m_direction == Direction::HORIZONTAL)
	{
		if (wa.w() + diff < m_minSize || wb.w() - diff < m_minSize)
			diff = 0;
		wa.resize(wa.x(), wa.y(), wa.w() + diff, wa.h());
		wb.resize(wb.x() + diff, wb.y(), wb.w() - diff, wb.h());
		resize(x() + diff, y(), w(), h());
	}
}

/* -------------------------------------------------------------------------- */

bool geResizerBar::isBefore(const Fl_Widget& wd) const
{
	const int before = m_direction == Direction::VERTICAL ? y() : x();
	return (m_direction == Direction::VERTICAL && wd.y() + wd.h() == before) ||
	       (m_direction == Direction::HORIZONTAL && wd.x() + wd.w() == before);
}

/* -------------------------------------------------------------------------- */

bool geResizerBar::isAfter(const Fl_Widget& wd) const
{
	const int after = m_direction == Direction::VERTICAL ? y() + h() : x() + w();
	return (m_direction == Direction::VERTICAL && wd.y() >= after) ||
	       (m_direction == Direction::HORIZONTAL && wd.x() >= after);
}

/* -------------------------------------------------------------------------- */

Fl_Widget& geResizerBar::getFirstWidget()
{
	return *findWidgets([this](const Fl_Widget& wd)
	{ return isBefore(wd); }, /*howmany=*/1)[0];
}

/* -------------------------------------------------------------------------- */

std::vector<Fl_Widget*> geResizerBar::findWidgets(std::function<bool(const Fl_Widget&)> f, int howmany) const
{
	std::vector<Fl_Widget*> out;
	Fl_Group*               group = static_cast<Fl_Group*>(parent());

	for (int t = 0; t < group->children(); t++)
	{
		Fl_Widget* wd = group->child(t);
		if (!f(*wd))
			continue;
		out.push_back(wd);
		if (howmany != -1 && out.size() == (size_t)howmany)
			break;
	}

	/* Make sure it finds the exact number of widgets requested, in case
	howmany != -1. */

	assert(howmany == -1 || (howmany != -1 && out.size() == (size_t)howmany));

	return out;
}

/* -------------------------------------------------------------------------- */

void geResizerBar::draw()
{
	Fl_Box::draw();
	fl_rectf(x(), y(), w(), h(), m_hover ? G_COLOR_GREY_2 : G_COLOR_GREY_1);
}

/* -------------------------------------------------------------------------- */

int geResizerBar::handle(int e)
{
	int ret        = 0;
	int currentPos = m_direction == Direction::VERTICAL ? Fl::event_y_root() : Fl::event_x_root();

	switch (e)
	{
	case FL_FOCUS:
		ret = 1;
		break;
	case FL_ENTER:
		ret = 1;
		fl_cursor(m_direction == Direction::VERTICAL ? FL_CURSOR_NS : FL_CURSOR_WE);
		m_hover = true;
		redraw();
		break;
	case FL_LEAVE:
		ret = 1;
		fl_cursor(FL_CURSOR_DEFAULT);
		m_hover = false;
		redraw();
		break;
	case FL_PUSH:
		ret       = 1;
		m_lastPos = currentPos;
		break;
	case FL_DRAG:
		handleDrag(currentPos - m_lastPos);
		m_lastPos = currentPos;
		ret       = 1;
		if (onDrag != nullptr)
			onDrag(getFirstWidget());
		break;
	case FL_RELEASE:
		if (onRelease != nullptr)
			onRelease(getFirstWidget());
		break;
	default:
		break;
	}
	return (Fl_Box::handle(e) | ret);
}

/* -------------------------------------------------------------------------- */

void geResizerBar::resize(int X, int Y, int W, int H)
{
	if (m_direction == Direction::VERTICAL)
		Fl_Box::resize(X, Y, W, h());
	else
		Fl_Box::resize(X, Y, w(), H);
}

/* -------------------------------------------------------------------------- */

void geResizerBar::moveTo(int p)
{
	const Fl_Widget& wd   = getFirstWidget();
	const int        curr = m_direction == Direction::VERTICAL ? wd.h() : wd.w();
	handleDrag(p - curr);
}
} // namespace giada::v
