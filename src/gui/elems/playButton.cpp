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

#include "src/gui/elems/playButton.h"
#include "src/gui/const.h"

namespace giada::v
{
gePlayButton::gePlayButton(const std::string& label)
: geTextButton(label.c_str())
{
}

/* -------------------------------------------------------------------------- */

void gePlayButton::setPlayState()
{
	m_backgroundColorOff = G_COLOR_LIGHT_1;
	m_borderColor        = G_COLOR_LIGHT_1;
	m_textColor          = G_COLOR_GREY_1;
}

/* -------------------------------------------------------------------------- */

void gePlayButton::setEndingState()
{
	m_backgroundColorOff = G_COLOR_GREY_4;
}

/* -------------------------------------------------------------------------- */

void gePlayButton::setDefaultState()
{
	m_backgroundColorOff = G_COLOR_GREY_2;
	m_borderColor        = G_COLOR_GREY_4;
	m_textColor          = G_COLOR_LIGHT_2;
}
}; // namespace giada::v
