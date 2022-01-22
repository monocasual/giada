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

#include "input.h"
#include "boxtypes.h"
#include "core/const.h"

geInput::geInput(int x, int y, int w, int h, const char* l)
: Fl_Input(x, y, w, h, l)
{
	//Fl::set_boxtype(G_CUSTOM_BORDER_BOX, gDrawBox, 1, 1, 2, 2);
	box(G_CUSTOM_BORDER_BOX);
	labelsize(G_GUI_FONT_SIZE_BASE);
	labelcolor(G_COLOR_LIGHT_2);
	color(G_COLOR_BLACK);
	textcolor(G_COLOR_LIGHT_2);
	cursor_color(G_COLOR_LIGHT_2);
	selection_color(G_COLOR_GREY_4);
	textsize(G_GUI_FONT_SIZE_BASE);

	when(FL_WHEN_CHANGED);
	callback(cb_onChange, this);
}

/* -------------------------------------------------------------------------- */

void geInput::cb_onChange(Fl_Widget* /*w*/, void* p) { (static_cast<geInput*>(p))->cb_onChange(); }

/* -------------------------------------------------------------------------- */

void geInput::cb_onChange()
{
	if (onChange != nullptr)
		onChange(value());
}