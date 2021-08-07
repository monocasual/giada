/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#ifndef GE_SPLITSCROLL_H
#define GE_SPLITSCROLL_H

#include "gui/elems/basics/liquidScroll.h"
#include "gui/elems/basics/scroll.h"
#include "gui/elems/basics/split.h"

namespace giada::v
{
class geSplitScroll : public geSplit
{
public:
	geSplitScroll(int x, int y, int w, int h);

	int getScrollX() const;
	int getScrollY() const;
	int getContentWidth() const;
	int getTopContentH() const;

	void addWidgets(Fl_Widget& a, Fl_Widget& b, int topContentH = -1);
	void setScrollX(int p);
	void setScrollY(int p);

private:
	geScroll       m_a;
	geLiquidScroll m_b;
};
} // namespace giada::v

#endif
