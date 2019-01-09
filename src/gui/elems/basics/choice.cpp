/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <string>
#include <FL/fl_draw.H>
#include "../../../core/const.h"
#include "choice.h"


geChoice::geChoice(int x, int y, int w, int h, const char *l, bool ang)
  : Fl_Choice(x, y, w, h, l), angle(ang)
{
  labelsize(G_GUI_FONT_SIZE_BASE);
  labelcolor(G_COLOR_LIGHT_2);
  box(FL_BORDER_BOX);
  textsize(G_GUI_FONT_SIZE_BASE);
  textcolor(G_COLOR_LIGHT_2);
  color(G_COLOR_GREY_2);
}


/* -------------------------------------------------------------------------- */


void geChoice::draw()
{
  fl_rectf(x(), y(), w(), h(), G_COLOR_GREY_2);              // bg
  fl_rect(x(), y(), w(), h(), (Fl_Color) G_COLOR_GREY_4);    // border
  if (angle)
    fl_polygon(x()+w()-8, y()+h()-1, x()+w()-1, y()+h()-8, x()+w()-1, y()+h()-1);

  /* pick up the text() from the selected item (value()) and print it in
   * the box and avoid overflows */

  fl_color(!active() ? G_COLOR_GREY_4 : G_COLOR_LIGHT_2);
  if (value() != -1) {
    if (fl_width(text(value())) < w()-8) {
      fl_draw(text(value()), x(), y(), w(), h(), FL_ALIGN_CENTER);
    }
    else {
      std::string tmp = text(value());
      int size        = tmp.size();
      while (fl_width(tmp.c_str()) >= w()-16) {
        tmp.resize(size);
        size--;
      }
      tmp += "...";
      fl_draw(tmp.c_str(), x(), y(), w(), h(), FL_ALIGN_CENTER);
    }

  }
}


/* -------------------------------------------------------------------------- */


void geChoice::showItem(const char *c)
{
  value(find_index(c));
}
