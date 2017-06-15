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


#include "../../../core/const.h"
#include "boxtypes.h"
#include "slider.h"


geSlider::geSlider(int x, int y, int w, int h, const char *l)
 : Fl_Slider(x, y, w, h, l)
{
 type(FL_HOR_FILL_SLIDER);

 labelsize(GUI_FONT_SIZE_BASE);
 align(FL_ALIGN_LEFT);
 labelcolor(G_COLOR_LIGHT_2);

 box(G_CUSTOM_BORDER_BOX);
 color(G_COLOR_GREY_2);
 selection_color(G_COLOR_GREY_4);
}
