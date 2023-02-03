/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gLiquidScroll
 * custom scroll that tells children to follow scroll's width when
 * resized. Thanks to Greg Ercolano from FLTK dev team.
 * http://seriss.com/people/erco/fltk/
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

#ifndef GE_LIQUID_SCROLL_H
#define GE_LIQUID_SCROLL_H

#include "core/const.h"
#include "deps/geompp/src/rect.hpp"
#include "gui/types.h"
#include "scroll.h"

namespace giada::v
{
class geLiquidScroll : public geScroll
{
public:
	geLiquidScroll(int x, int y, int w, int h, Direction d);
	geLiquidScroll(geompp::Rect<int>, Direction d);

	void resize(int x, int y, int w, int h) override;

	/* addWidget
    Adds a new widget to the bottom, with proper spacing. */

	template <typename T>
	T* addWidget(T* wg)
	{
		int numChildren = countChildren();

		int wx = x();
		int wy = y() - yposition() + (numChildren * (wg->h() + G_GUI_INNER_MARGIN));
		int ww = w() - 24;
		int wh = wg->h();

		wg->resize(wx, wy, ww, wh);
		add(wg);
		redraw();

		return wg;
	}

private:
	Direction m_direction;
};
} // namespace giada::v

#endif
