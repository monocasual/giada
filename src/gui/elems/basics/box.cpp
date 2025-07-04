/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/elems/basics/box.h"
#include "src/gui/const.h"
#include "src/utils/gui.h"
#include <FL/fl_draw.H>
#include <cassert>

namespace giada::v
{
geBox::geBox(int x, int y, int w, int h, const char* l, Fl_Align al)
: Fl_Box(x, y, w, h)
{
	copy_label(l);
	box(FL_NO_BOX);
	color(G_COLOR_GREY_1);
	align(al | FL_ALIGN_INSIDE);
}

/* -------------------------------------------------------------------------- */

geBox::geBox(const char* l, Fl_Align al)
: geBox(0, 0, 0, 0, l, al)
{
}

/* -------------------------------------------------------------------------- */

void geBox::draw()
{
	fl_rectf(x(), y(), w(), h(), color()); // Clear background

	if (box() != FL_NO_BOX)
		fl_rect(x(), y(), w(), h(), G_COLOR_GREY_4); // Border

	if (image() != nullptr)
	{
		if (m_image != nullptr)
			m_image->scale(w(), h());
		draw_label(); // draw_label also paints image, if any
	}
	else if (label() != nullptr)
	{
		fl_color(active() ? G_COLOR_LIGHT_2 : G_COLOR_GREY_4);
		fl_font(FL_HELVETICA, G_GUI_FONT_SIZE_BASE);
		fl_draw(giada::u::gui::truncate(label(), w()).c_str(), x(), y(), w(), h(), align());
	}
}

/* -------------------------------------------------------------------------- */

void geBox::setSvgImage(const char* svg)
{
	m_image = std::make_unique<Fl_SVG_Image>(nullptr, svg);
	image(m_image.get());
}
} // namespace giada::v