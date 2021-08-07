/* -----------------------------------------------------------------------------
*
* Giada - Your Hardcore Loopmachine
*
* ------------------------------------------------------------------------------
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
* --------------------------------------------------------------------------- */

#include "splitScroll.h"

namespace giada::v
{
geSplitScroll::geSplitScroll(Pixel x, Pixel y, Pixel w, Pixel h)
: geSplit(x, y, w, h)
, m_a(0, 0, 0, 0, Fl_Scroll::VERTICAL_ALWAYS)
, m_b(0, 0, 0, 0, Direction::HORIZONTAL)
{
	m_b.onScrollH = [&a = m_a](int x) {
		a.scroll_to(x, a.yposition());
	};
}

/* -------------------------------------------------------------------------- */

void geSplitScroll::addWidgets(Fl_Widget& wa, Fl_Widget& wb, int topContentH)
{
	m_a.add(&wa);
	m_b.addWidget(&wb);

	init(m_a, m_b);

	if (topContentH != -1)
		resizePanel(geSplit::Panel::A, topContentH);
}

/* -------------------------------------------------------------------------- */

int geSplitScroll::getScrollX() const
{
	return m_b.xposition();
}

int geSplitScroll::getScrollY() const
{
	return m_a.yposition();
}

/* -------------------------------------------------------------------------- */

int geSplitScroll::getContentWidth() const
{
	if (m_a.countChildren() == 0)
		return 0;
	return m_a.child(0)->w();
}

/* -------------------------------------------------------------------------- */

int geSplitScroll::getTopContentH() const
{
	return m_a.h();
}

/* -------------------------------------------------------------------------- */

void geSplitScroll::setScrollX(int p)
{
	p = std::max(0, p);
	m_a.scroll_to(p, m_a.yposition());
	m_b.scroll_to(p, m_b.yposition());
}

void geSplitScroll::setScrollY(int p)
{
	m_a.scroll_to(m_a.xposition(), p);
}
} // namespace giada::v