/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * boxtypes
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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
#include "boxtypes.h"


void g_customBorderBox(int x, int y, int w, int h, Fl_Color c)
{
  fl_color(c);
  fl_rectf(x, y, w, h);
  fl_color(COLOR_BD_0);
  fl_rect(x, y, w, h);
}


void g_customUpBox(int x, int y, int w, int h, Fl_Color c)
{
  fl_color(COLOR_BG_0);
  fl_rectf(x, y, w, h);
  fl_color(COLOR_BG_0);
  fl_rect(x, y, w, h);
}


void g_customDownBox(int x, int y, int w, int h, Fl_Color c)
{
  fl_color(c);
  fl_rectf(x, y, w, h);
  fl_color(COLOR_BG_0);
  fl_rect(x, y, w, h);
}
