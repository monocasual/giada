/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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

#include "button.h"
#include "core/const.h"
#include "utils/gui.h"
#include <FL/fl_draw.H>

namespace giada::v
{
geButton::geButton(int x, int y, int w, int h, const char* l,
    const char** imgOff, const char** imgOn, const char** imgDisabled)
: Fl_Button(x, y, w, h, l)
, onClick(nullptr)
, imgOff(imgOff)
, imgOn(imgOn)
, imgDisabled(imgDisabled)
, bgColor0(G_COLOR_GREY_2)
, bgColor1(G_COLOR_GREY_4)
, bdColor(G_COLOR_GREY_4)
, txtColor(G_COLOR_LIGHT_2)
{
	callback(cb_click);
}

/* -------------------------------------------------------------------------- */

geButton::geButton(const char* l, const char** imgOff, const char** imgOn, const char** imgDisabled)
: geButton(0, 0, 0, 0, l, imgOff, imgOn, imgDisabled)
{
}

/* -------------------------------------------------------------------------- */

void geButton::cb_click(Fl_Widget* w, void*)
{
	geButton* b = static_cast<geButton*>(w);
	if (b->onClick != nullptr)
		b->onClick();
}

/* -------------------------------------------------------------------------- */

void geButton::draw()
{
	//Fl_Button::draw();

	if (active())
		if (value())
			draw(imgOn, bgColor1, txtColor);
		else
			draw(imgOff, bgColor0, txtColor);
	else
		draw(imgDisabled, bgColor0, bdColor);
}

/* -------------------------------------------------------------------------- */

void geButton::draw(const char** img, Fl_Color bgColor, Fl_Color textColor)
{
	fl_rect(x(), y(), w(), h(), bdColor); // draw border

	if (img != nullptr)
	{
		fl_draw_pixmap(img, x() + 1, y() + 1);
		return;
	}

	fl_rectf(x() + 1, y() + 1, w() - 2, h() - 2, bgColor); // draw background
	fl_color(textColor);

	if (label() != nullptr)
	{
		fl_font(FL_HELVETICA, G_GUI_FONT_SIZE_BASE);
		fl_draw(giada::u::gui::truncate(label(), w() - 16).c_str(), x() + 2, y(), w() - 2, h(), FL_ALIGN_CENTER);
	}
}
} // namespace giada::v