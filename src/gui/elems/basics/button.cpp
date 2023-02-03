/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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

#include "gui/elems/basics/button.h"

namespace giada::v
{
geButton::geButton(int x, int y, int w, int h, const char* l)
: Fl_Button(x, y, w, h, l)
, onClick(nullptr)
, m_value(false)
, m_toggleable(false)
, m_forced(false)
{
}

/* -------------------------------------------------------------------------- */

geButton::geButton(const char* l)
: geButton(0, 0, 0, 0, l)
{
}

/* -------------------------------------------------------------------------- */

int geButton::handle(int e)
{
	switch (e)
	{
	case FL_PUSH:
		take_focus();
		if (!m_toggleable && !m_forced)
			m_value = true;
		redraw();
		if (when() == FL_WHEN_CHANGED && onClick != nullptr)
			onClick();
		return 1;

	case FL_RELEASE:
		if (!m_forced)
			m_value = m_toggleable ? !m_value : false;
		redraw();
		if (onClick != nullptr)
			onClick();
		return 1;

	case FL_SHORTCUT:
		if (!(shortcut() ? Fl::test_shortcut(shortcut()) : test_shortcut()))
			return 0;
		if (onClick != nullptr)
			onClick();
		return 1;

	default:
		return Fl_Button::handle(e);
	}
}

/* -------------------------------------------------------------------------- */

geompp::Rect<int> geButton::getBounds() const
{
	return {x(), y(), w(), h()};
}

/* -------------------------------------------------------------------------- */

void geButton::setToggleable(bool v)
{
	m_toggleable = v;
}

/* -------------------------------------------------------------------------- */

void geButton::setValue(bool v)
{
	m_value = v;
	redraw();
}

/* -------------------------------------------------------------------------- */

void geButton::forceValue(bool v)
{
	setValue(v);
	m_forced = true;
}

/* -------------------------------------------------------------------------- */

bool geButton::getValue() const
{
	return m_value;
}
} // namespace giada::v