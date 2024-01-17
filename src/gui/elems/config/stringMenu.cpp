/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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

#include "gui/elems/config/stringMenu.h"
#include "utils/gui.h"

namespace giada::v
{
geStringMenu::geStringMenu(const char* l, const std::string& msgIfNotFound, int labelWidth)
: geChoice(l, labelWidth)
, m_msgIfNotFound(msgIfNotFound)
{
}

/* -------------------------------------------------------------------------- */

void geStringMenu::rebuild(const std::vector<std::string>& data)
{
	clear();

	if (data.size() == 0)
	{
		addItem(m_msgIfNotFound.c_str(), 0);
		showItem(0);
		deactivate();
	}
	else
	{
		for (const std::string& d : data)
			addItem(u::gui::removeFltkChars(d).c_str(), -1); // -1: auto-increment ID
	}
}
} // namespace giada::v