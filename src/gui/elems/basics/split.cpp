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

#include "split.h"
#include "core/const.h"
#include "gui/elems/basics/box.h"

namespace giada::v
{
geSplit::geSplit(int x, int y, int w, int h)
: Fl_Group(x, y, w, h)
, m_a(nullptr)
, m_b(nullptr)
, m_bar(0, 0, w, G_GUI_INNER_MARGIN, G_GUI_UNIT, geResizerBar::Direction::VERTICAL, geResizerBar::Mode::RESIZE)
{
	end();
}

/* -------------------------------------------------------------------------- */

void geSplit::init(Fl_Widget& a, Fl_Widget& b)
{
	a.resize(x(), y(), w(), (h() / 2) - G_GUI_INNER_MARGIN); // Panel A goes on top
	a.redraw();
	m_a = &a;

	m_bar.resize(x(), m_a->y() + m_a->h(), w(), G_GUI_INNER_MARGIN);

	b.resize(x(), m_bar.y() + m_bar.h(), w(), h() / 2); // Panel B goes on bottom
	b.redraw();
	m_b = &b;

	Fl_Group::add(m_a);
	Fl_Group::add(m_bar);
	Fl_Group::add(m_b);

	resizable(m_a);
}

/* -------------------------------------------------------------------------- */

void geSplit::resizePanel(Panel p, int s)
{
	m_bar.moveTo(p == Panel::A ? s : h() - s);
}
} // namespace giada::v