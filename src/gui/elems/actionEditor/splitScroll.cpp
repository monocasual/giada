/* -----------------------------------------------------------------------------
*
* Giada - Your Hardcore Loopmachine
*
* ------------------------------------------------------------------------------
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
* --------------------------------------------------------------------------- */

#include "splitScroll.h"
#include "gui/drawing.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/flexResizable.h"
#include "gui/elems/basics/scrollbar.h"

namespace giada::v
{
geSplitScroll::geSplitScroll(Pixel x, Pixel y, Pixel w, Pixel h)
: geFlex(x, y, w, h, Direction::VERTICAL /*, geResizerBar::Mode::RESIZE*/)
, m_a(0, 0, 0, 0, Fl_Scroll::VERTICAL_ALWAYS)
, m_b(0, 0, 0, 0, Direction::HORIZONTAL, /*withScrollbars=*/false)
{
	m_body = new geFlexResizable(Direction::VERTICAL, geResizerBar::Mode::RESIZE);
	{
		m_body->addWidget(m_a);
		m_body->addWidget(m_b);
		m_body->end();
	}

	m_scrollbar = new geScrollbar();

	addWidget(m_body);
	addWidget(new geBox(), G_GUI_OUTER_MARGIN);
	addWidget(m_scrollbar, 16);
	end();

	m_scrollbar->type(FL_HORIZONTAL);
	m_scrollbar->onScroll = [this](float v) {
		m_a.scroll_to(v, m_a.yposition());
		m_b.scroll_to(v, m_b.yposition());
	};

	m_body->onDragBar = [this](const Fl_Widget&) {
		if (onDragBar)
			onDragBar();
	};

	m_a.onScrollV = [this](Pixel y) {
		if (onScrollV)
			onScrollV(y);
	};

	m_b.onScrollH = [&a = m_a](Pixel x) {
		a.scroll_to(x, a.yposition());
	};
}

/* -------------------------------------------------------------------------- */

void geSplitScroll::draw()
{
	geFlex::draw();

	const geompp::Rect<int> bounds_a = {m_a.x(), m_a.y(), m_a.w(), m_a.h()};
	const geompp::Rect<int> bounds_b = {m_b.x(), m_b.y(), m_b.w(), m_b.h()};

	drawLine(bounds_a.getTopLine(), G_COLOR_GREY_4);
	drawLine(bounds_a.getLeftLine(), G_COLOR_GREY_4);
	drawLine(bounds_a.getBottomLine().withShiftedY(-1), G_COLOR_GREY_4);

	drawLine(bounds_b.getLeftLine(), G_COLOR_GREY_4);
}

/* -------------------------------------------------------------------------- */

void geSplitScroll::addWidgets(Fl_Widget& wa, Fl_Widget& wb, Pixel topContentH)
{
	wa.position(x(), y());

	m_a.add(&wa);
	m_b.addWidget(&wb);

	if (topContentH != -1)
		m_body->resizeWidget(0, topContentH);
}

/* -------------------------------------------------------------------------- */

Pixel geSplitScroll::getScrollX() const
{
	return m_b.xposition();
}

Pixel geSplitScroll::getScrollY() const
{
	return m_a.yposition();
}

/* -------------------------------------------------------------------------- */

Pixel geSplitScroll::getContentWidth() const
{
	if (m_a.countChildren() == 0)
		return 0;
	return m_a.child(0)->w();
}

/* -------------------------------------------------------------------------- */

Pixel geSplitScroll::getTopContentH() const
{
	return m_a.h();
}

/* -------------------------------------------------------------------------- */

geompp::Rect<Pixel> geSplitScroll::getBoundsNoScrollbar() const
{
	return {
	    x(), y(),
	    w() - m_a.scrollbar.w() - G_GUI_OUTER_MARGIN,
	    h() - m_b.hscrollbar.h() - G_GUI_OUTER_MARGIN};
}

/* -------------------------------------------------------------------------- */

Pixel geSplitScroll::getBottomScrollbarH() const
{
	return m_b.hscrollbar.h();
}

/* -------------------------------------------------------------------------- */

void geSplitScroll::setScrollX(Pixel p)
{
	p = std::max(0, p);
	m_a.scroll_to(p, m_a.yposition());
	m_b.scroll_to(p, m_b.yposition());

	initScrollbar();
}

void geSplitScroll::setScrollY(Pixel p)
{
	m_a.scroll_to(m_a.xposition(), p);
}

/* -------------------------------------------------------------------------- */

void geSplitScroll::resizeWidget(int index, int size)
{
	m_body->resizeWidget(index, size);
}

/* -------------------------------------------------------------------------- */

void geSplitScroll::initScrollbar()
{
	m_scrollbar->range(0, getContentWidth() - w());
	m_scrollbar->slider_size(1.0 / (getContentWidth() / (double)w()));
	m_scrollbar->value(getScrollX());
}
} // namespace giada::v
