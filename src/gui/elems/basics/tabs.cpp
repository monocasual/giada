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

#include "gui/elems/basics/tabs.h"
#include "gui/const.h"
#include "gui/elems/basics/boxtypes.h"

namespace giada::v
{
geTabs::geTabs(geompp::Rect<int> r)
: Fl_Tabs(r.x, r.y, r.w, r.h)
{
	box(G_CUSTOM_BORDER_BOX);
	labelcolor(G_COLOR_LIGHT_2);
	end();
}

/* -------------------------------------------------------------------------- */

void geTabs::add(Fl_Widget* wg)
{
	constexpr int TAB_HEIGHT = 25;

	wg->resize(x(), y() + TAB_HEIGHT, w(), h() - TAB_HEIGHT);
	wg->labelsize(G_GUI_FONT_SIZE_BASE);
	wg->selection_color(G_COLOR_GREY_4);

	Fl_Tabs::add(wg);
	resizable(wg); //  To keep the tab height constant during resizing
}
} // namespace giada::v
