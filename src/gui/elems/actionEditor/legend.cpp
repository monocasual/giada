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

#include "src/gui/elems/actionEditor/legend.h"
#include "src/gui/const.h"
#include "src/gui/drawing.h"
#include <FL/fl_draw.H>

namespace giada::v
{
geLegend::geLegend(const char* label)
: geBox(label)
{
}

/* -------------------------------------------------------------------------- */

void geLegend::draw()
{
	const geompp::Rect<int> bounds = {x(), y(), w(), h()};

	drawRectf(bounds, G_COLOR_GREY_2); // Clear background

	drawLine(bounds.getTopLine(), G_COLOR_GREY_4);
	drawLine(bounds.getLeftLine(), G_COLOR_GREY_4);
	drawLine(bounds.getBottomLine().withShiftedY(-1), G_COLOR_GREY_4);

	drawTextVertical(label(), bounds, FL_HELVETICA, G_GUI_FONT_SIZE_BASE, G_COLOR_GREY_5);
}
} // namespace giada::v
