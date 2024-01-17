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

#include "gui/elems/basics/progress.h"
#include "core/const.h"
#include "gui/elems/basics/boxtypes.h"

namespace giada::v
{
geProgress::geProgress(int x, int y, int w, int h, const char* l)
: Fl_Progress(x, y, w, h, l)
{
	color(G_COLOR_GREY_2, G_COLOR_GREY_4);
	box(G_CUSTOM_BORDER_BOX);
}

/* -------------------------------------------------------------------------- */

geProgress::geProgress()
: geProgress(0, 0, 0, 0)
{
}
} // namespace giada::v