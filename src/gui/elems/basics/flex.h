/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GE_FLEX_H
#define GE_FLEX_H

#include "deps/geompp/src/border.hpp"
#include "deps/geompp/src/rect.hpp"
#include "gui/types.h"
#include <FL/Fl_Flex.H>
#include <vector>

namespace giada::v
{
/* geFlex
Like a FlexBox item, it's a group that contains widgets that can be stretched 
to fill the area. Inspired by https://github.com/osen/FL_Flex. */

class geFlex : public Fl_Flex
{
public:
	geFlex(int x, int y, int w, int h, Direction d, int gutter = 0, geompp::Border<int> pad = {});
	geFlex(geompp::Rect<int>, Direction d, int gutter = 0, geompp::Border<int> pad = {});
	geFlex(Direction d, int gutter = 0, geompp::Border<int> pad = {});

	/* add
	Adds an existing widget to the Flex layout. If 'size' == -1, the widget
	will be stretched to take up the available space. WARNING: like Fl_Group,
	geFlex owns widgets! */

	void add(Fl_Widget&, int size = -1);
	void add(Fl_Widget*, int size = -1);

	/* end
	Finalize the Flex item. Call this when you're done add()ing widgets. */

	void end();
};
} // namespace giada::v

#endif