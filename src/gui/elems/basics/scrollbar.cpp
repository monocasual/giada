/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geScroll
 * Custom scroll with nice scrollbars and something else.
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

#include "src/gui/elems/basics/scrollbar.h"
#include "src/gui/const.h"
#include "src/gui/elems/basics/boxtypes.h"

namespace giada::v
{
geScrollbar::geScrollbar()
: Fl_Scrollbar(0, 0, 0, 0)
, onScroll(nullptr)
{
	color(G_COLOR_GREY_2);
	selection_color(G_COLOR_GREY_4);
	labelcolor(G_COLOR_LIGHT_1);
	slider(G_CUSTOM_BORDER_BOX);

	callback([](Fl_Widget* w)
	{
		const geScrollbar* self = static_cast<geScrollbar*>(w);
		if (self->onScroll)
			self->onScroll(self->value());
	});
}
} // namespace giada::v
