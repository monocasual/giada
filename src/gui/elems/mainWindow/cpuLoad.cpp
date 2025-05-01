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

#include "gui/elems/mainWindow/cpuLoad.h"
#include "core/const.h"
#include "glue/main.h"
#include "gui/drawing.h"
#include <fmt/core.h>

namespace giada::v
{
geCpuLoad::geCpuLoad()
: Fl_Box(0, 0, 0, 0)
{
}

/* -------------------------------------------------------------------------- */

void geCpuLoad::draw()
{
	const double load = c::main::getCpuLoad();

	const geompp::Rect bounds(x(), y(), w(), h());

	drawRectf(bounds, G_COLOR_GREY_2);
	drawRect(bounds, G_COLOR_GREY_4);

	drawText(fmt::format("{:.1f}%", load), bounds, FL_HELVETICA, G_GUI_FONT_SIZE_BASE, G_COLOR_GREY_4);
}

/* -------------------------------------------------------------------------- */

void geCpuLoad::refresh()
{
	redraw();
}
} // namespace giada::v
