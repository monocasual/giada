/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geScroll
 * Custom scroll with nice scrollbars and something else.
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


#include "../../../core/const.h"
#include "boxtypes.h"
#include "scroll.h"


geScroll::geScroll(int x, int y, int w, int h, int t)
  : Fl_Scroll(x, y, w, h)
{
  type(t);

  scrollbar.color(G_COLOR_GREY_2);
  scrollbar.selection_color(G_COLOR_GREY_4);
  scrollbar.labelcolor(G_COLOR_LIGHT_1);
  scrollbar.slider(G_CUSTOM_BORDER_BOX);

  hscrollbar.color(G_COLOR_GREY_2);
  hscrollbar.selection_color(G_COLOR_GREY_4);
  hscrollbar.labelcolor(G_COLOR_LIGHT_1);
  hscrollbar.slider(G_CUSTOM_BORDER_BOX);
}
