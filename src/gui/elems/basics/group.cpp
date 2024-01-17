/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geScroll
 * Custom scroll with nice scrollbars and something else.
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "group.h"
#include <algorithm>
#include <cstddef>

namespace giada
{
namespace v
{
geGroup::geGroup(int x, int y)
: Fl_Group(x, y, 0, 0)
{
	end();
}

/* -------------------------------------------------------------------------- */

std::size_t geGroup::countChildren() const
{
	return m_widgets.size();
}

/* -------------------------------------------------------------------------- */

void geGroup::add(Fl_Widget* widget)
{
	widget->position(widget->x() + x(), widget->y() + y());

	Fl_Group::add(widget);
	m_widgets.push_back(widget);

	int newW = 0;
	int newH = 0;

	for (const Fl_Widget* wg : m_widgets)
	{
		newW = std::max(newW, (wg->x() + wg->w()) - x());
		newH = std::max(newH, (wg->y() + wg->h()) - y());
	}

	/* Don't call size(newW, newH) as it changes widgets position. Adjust width
    and height manually instead. */

	w(newW);
	h(newH);
}

/* -------------------------------------------------------------------------- */

Fl_Widget* geGroup::getChild(std::size_t i)
{
	return m_widgets.at(i); // Throws std::out_of_range in case
}

/* -------------------------------------------------------------------------- */

Fl_Widget* geGroup::getLastChild()
{
	return m_widgets.at(m_widgets.size() - 1); // Throws std::out_of_range in case
}
} // namespace v
} // namespace giada
