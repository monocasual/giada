/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <FL/fl_draw.H>
#include "core/const.h"
#include "utils/gui.h"
#include "menuButton.h"


namespace giada {
namespace v
{

geMenuButton::geMenuButton(int x, int y, int w, int h, const char* l,
							const char* txt) 
: Fl_Menu_Button(x, y, w, h, l),
fg_text(txt)
  //bgColor0    (G_COLOR_GREY_2),
  //bgColor1    (G_COLOR_GREY_4),
  //bdColor     (G_COLOR_GREY_4),
  //txtColor    (G_COLOR_LIGHT_2)
{
	labelsize(G_GUI_FONT_SIZE_BASE);
	labelcolor(G_COLOR_LIGHT_2);
	box(FL_BORDER_BOX);
	textsize(G_GUI_FONT_SIZE_BASE);
	textcolor(G_COLOR_LIGHT_2);
	color(G_COLOR_GREY_2);
}


/* -------------------------------------------------------------------------- */


void geMenuButton::draw()
{
	fl_rectf(x(), y(), w(), h(), G_COLOR_GREY_2);              // bg
	fl_rect(x(), y(), w(), h(), (Fl_Color) G_COLOR_GREY_4);    // border
	fl_polygon(x()+w()-8, y()+h()-1, x()+w()-1,	// tiny triangle
		y()+h()-8, x()+w()-1, y()+h()-1);

	fl_color(!active() ? G_COLOR_GREY_4 : G_COLOR_LIGHT_2);

	// Draw foreground text
	fl_draw(u::gui::truncate(fg_text, w()-16).c_str(),
		x(), y(), w(), h(), FL_ALIGN_CENTER);
}


/* -------------------------------------------------------------------------- */


void geMenuButton::text(std::string txt)
{
	fg_text = txt;
}

/* -------------------------------------------------------------------------- */


std::string geMenuButton::text()
{
	return fg_text;
}

/* -------------------------------------------------------------------------- */


}}  // giada::v
