/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geScroll
 * Custom scroll with nice scrollbars and something else.
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

#include "pack.h"
#include "core/const.h"

namespace giada::v
{
gePack::gePack(int x, int y, Direction d, int gutter)
: geGroup(x, y)
, m_direction(d)
, m_gutter(gutter)
{
	end();
}

/* -------------------------------------------------------------------------- */

void gePack::add(Fl_Widget* widget)
{
	if (countChildren() == 0)
		widget->position(0, 0);
	else if (m_direction == Direction::HORIZONTAL)
		widget->position((getLastChild()->x() + getLastChild()->w() + m_gutter) - x(), 0);
	else
		widget->position(0, (getLastChild()->y() + getLastChild()->h() + m_gutter) - y());

	geGroup::add(widget);
}
} // namespace giada::v