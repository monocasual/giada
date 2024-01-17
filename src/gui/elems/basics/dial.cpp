/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/elems/basics/dial.h"
#include "core/const.h"
#include <FL/fl_draw.H>

namespace giada::v
{
geDial::geDial(int x, int y, int w, int h, const char* l)
: Fl_Dial(x, y, w, h, l)
, onChange(nullptr)
{
	labelsize(G_GUI_FONT_SIZE_BASE);
	labelcolor(G_COLOR_LIGHT_2);
	align(FL_ALIGN_LEFT);
	type(FL_FILL_DIAL);
	angles(0, 360);
	color(G_COLOR_GREY_2);           // background
	selection_color(G_COLOR_GREY_4); // selection
	callback(cb_change);
}

/* -------------------------------------------------------------------------- */

geDial::geDial(const char* l)
: geDial(0, 0, 0, 0, l)
{
}

/* -------------------------------------------------------------------------- */

void geDial::cb_change(Fl_Widget* w, void*)
{
	geDial* d = static_cast<geDial*>(w);
	if (d->onChange != nullptr)
		d->onChange(d->value());
}

/* -------------------------------------------------------------------------- */

void geDial::draw()
{
	double angle = (angle2() - angle1()) * (value() - minimum()) / (maximum() - minimum()) + angle1();

	fl_color(G_COLOR_GREY_2);
	fl_pie(x(), y(), w(), h(), 270 - angle1(), angle > angle1() ? 360 + 270 - angle : 270 - 360 - angle);

	fl_color(G_COLOR_GREY_4);
	fl_arc(x(), y(), w(), h(), 0, 360);
	fl_pie(x(), y(), w(), h(), 270 - angle, 270 - angle1());
}
} // namespace giada::v