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

#ifndef GE_SPLITSCROLL_H
#define GE_SPLITSCROLL_H

#include "core/types.h"
#include "deps/geompp/src/rect.hpp"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/liquidScroll.h"
#include "gui/elems/basics/resizerBar.h"
#include "gui/elems/basics/scroll.h"

namespace giada::v
{
class geFlexResizable;
class geScrollbar;
class geSplitScroll : public geFlex
{
public:
	geSplitScroll(Pixel x, Pixel y, Pixel w, Pixel h);

	Pixel               getScrollX() const;
	Pixel               getScrollY() const;
	Pixel               getContentWidth() const;
	Pixel               getTopContentH() const;
	geompp::Rect<Pixel> getBoundsNoScrollbar() const;
	Pixel               getBottomScrollbarH() const;

	void draw() override;

	void addWidgets(Fl_Widget& a, Fl_Widget& b, Pixel topContentH = -1);
	void setScrollX(Pixel p);
	void setScrollY(Pixel p);

	/* resizeWidget
	Resizes widget at index 'index' to size 'size'. */

	void resizeWidget(int index, int size);

	/* initScrollbar
	Initializes the horizontal scrollbar properties (size, position, ...). Call
	this whenever the owned widgets size change. */

	void initScrollbar();

	/* onScrollV
	Callback fired when the vertical scrollbar is moved. */

	std::function<void(int y)> onScrollV = nullptr;

	/* onDragBar
	Callback fired when the horizontal drag bar is moved. */

	std::function<void()> onDragBar = nullptr;

private:
	geFlexResizable* m_body;
	geScrollbar*     m_scrollbar;
	geScroll         m_a;
	geLiquidScroll   m_b;
};
} // namespace giada::v

#endif
