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

#include "src/gui/elems/basics/check.h"
#include "src/gui/const.h"
#include "src/utils/gui.h"
#include <FL/fl_draw.H>
#include <cstring>

namespace giada::v
{
geCheck::geCheck()
: geCheck(0, 0, 0, 0)
{
}

/* -------------------------------------------------------------------------- */

geCheck::geCheck(int x, int y, int w, int h, const char* l)
: Fl_Check_Button(x, y, w, h, l)
{
	callback(cb_onChange, this);
}

/* -------------------------------------------------------------------------- */

geCheck::geCheck(const char* l)
: geCheck(0, 0, 0, 0, l)
{
}

/* -------------------------------------------------------------------------- */

void geCheck::cb_onChange(Fl_Widget* /*w*/, void* p) { (static_cast<geCheck*>(p))->cb_onChange(); }

/* -------------------------------------------------------------------------- */

void geCheck::cb_onChange()
{
	if (onChange != nullptr)
		onChange(value());
}

/* -------------------------------------------------------------------------- */

void geCheck::draw()
{
	fl_rectf(x(), y(), w(), h(), FL_BACKGROUND_COLOR); // clearer

	const Fl_Color boxColor  = !active() ? FL_INACTIVE_COLOR : G_COLOR_GREY_4;
	const int      textColor = !active() ? FL_INACTIVE_COLOR : G_COLOR_LIGHT_2;
	const Fl_Align textAlign = FL_ALIGN_LEFT | FL_ALIGN_CENTER;
	const int      textW     = w() - (CHECKBOX_WIDTH + G_GUI_UNIT);

	if (value())
		fl_rectf(x(), y(), CHECKBOX_WIDTH, h(), boxColor);
	else
		fl_rect(x(), y(), CHECKBOX_WIDTH, h(), boxColor);

	if (label() == nullptr)
		return;

	fl_font(FL_HELVETICA, G_GUI_FONT_SIZE_BASE);
	fl_color(textColor);
	fl_draw(u::gui::truncate(label(), textW).c_str(), x() + G_GUI_UNIT, y(), w(), h(), textAlign);
}
} // namespace giada::v
