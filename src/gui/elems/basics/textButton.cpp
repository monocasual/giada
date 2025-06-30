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

#include "gui/elems/basics/textButton.h"
#include "core/const.h"
#include "gui/const.h"
#include "gui/drawing.h"
#include "utils/gui.h"

namespace giada::v
{
geTextButton::geTextButton(int x, int y, int w, int h, const char* l)
: geButton(x, y, w, h, l)
, m_backgroundColorOff(G_COLOR_GREY_2)
, m_backgroundColorOn(G_COLOR_GREY_4)
, m_borderColor(G_COLOR_GREY_4)
, m_textColor(G_COLOR_LIGHT_2)
{
}

/* -------------------------------------------------------------------------- */

geTextButton::geTextButton(const char* l)
: geTextButton(0, 0, 0, 0, l)
{
}

/* -------------------------------------------------------------------------- */

void geTextButton::draw()
{
	const Fl_Color          textColor = active() ? m_textColor : G_COLOR_GREY_4;
	const Fl_Color          bgColor   = getValue() ? m_backgroundColorOn : m_backgroundColorOff;
	const geompp::Rect<int> bounds    = getBounds();

	drawRectf(bounds, bgColor);      // draw background
	drawRect(bounds, m_borderColor); // draw border

	if (label() != nullptr)
		drawText(u::gui::truncate(label(), w() - 16), bounds.reduced(1), FL_HELVETICA, G_GUI_FONT_SIZE_BASE, textColor);
}
} // namespace giada::v