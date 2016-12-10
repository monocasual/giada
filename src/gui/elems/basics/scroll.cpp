/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geScroll
 * Custom scroll with nice scrollbars and something else.
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


#include "../../../core/const.h"
#include "../ge_mixed.h" // TODO for G_BOX
#include "scroll.h"


geScroll::geScroll(int x, int y, int w, int h, int t)
  : Fl_Scroll(x, y, w, h)
{
  type(t);

  scrollbar.color(COLOR_BG_0);
  scrollbar.selection_color(COLOR_BG_1);
  scrollbar.labelcolor(COLOR_BD_1);
  scrollbar.slider(G_BOX);

  hscrollbar.color(COLOR_BG_0);
  hscrollbar.selection_color(COLOR_BG_1);
  hscrollbar.labelcolor(COLOR_BD_1);
  hscrollbar.slider(G_BOX);
}
