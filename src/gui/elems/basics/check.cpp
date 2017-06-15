/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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
#include "../../../core/const.h"
#include "check.h"


geCheck::geCheck(int x, int y, int w, int h, const char *l)
: Fl_Check_Button(x, y, w, h, l)
{
}


/* -------------------------------------------------------------------------- */


void geCheck::draw()
{
  int color = !active() ? FL_INACTIVE_COLOR : G_COLOR_GREY_4;

  if (value()) {
    fl_rect(x(), y(), 12, 12, (Fl_Color) color);
    fl_rectf(x(), y(), 12, 12, (Fl_Color) color);
  }
  else {
    fl_rectf(x(), y(), 12, 12, FL_BACKGROUND_COLOR);
    fl_rect(x(), y(), 12, 12, (Fl_Color) color);
  }

  fl_rectf(x()+20, y(), w(), h(), FL_BACKGROUND_COLOR);  // clearer
  fl_font(FL_HELVETICA, G_GUI_FONT_SIZE_BASE);
  fl_color(!active() ? FL_INACTIVE_COLOR : G_COLOR_LIGHT_2);
  fl_draw(label(), x()+20, y(), w(), h(), (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_TOP));
}
