/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geStatusButton
 * Simple geButton with a boolean 'status' parameter.
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "statusButton.h"
#include "core/const.h"
#include <FL/fl_draw.H>

namespace giada::v
{
geStatusButton::geStatusButton(int x, int y, int w, int h, const char** imgOff,
    const char** imgOn, const char** imgDisabled)
: geButton(x, y, w, h, "", imgOff, imgOn, imgDisabled)
, m_status(false)
{
}

/* -------------------------------------------------------------------------- */

geStatusButton::geStatusButton(const char** imgOff, const char** imgOn,
    const char** imgDisabled)
: geButton("", imgOff, imgOn, imgDisabled)
, m_status(false)
{
}

/* -------------------------------------------------------------------------- */

void geStatusButton::draw()
{
	if (active())
		if (m_status)
			geButton::draw(imgOn, bgColor1, txtColor);
		else
			geButton::draw(imgOff, bgColor0, txtColor);
	else
		geButton::draw(imgDisabled, bgColor0, bdColor);
}

/* -------------------------------------------------------------------------- */

void geStatusButton::setStatus(bool s)
{
	m_status = s;
	redraw();
}

bool geStatusButton::getStatus() const
{
	return m_status;
}
} // namespace giada::v