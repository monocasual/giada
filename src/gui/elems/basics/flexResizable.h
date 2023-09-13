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

#ifndef GE_FLEXRESIZABLE_H
#define GE_FLEXRESIZABLE_H

#include "core/types.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/resizerBar.h"
#include <functional>

namespace giada::v
{
/* geFlexResizable
A flex-based container where each element is resizable via resizer bar. See
geResizerBar for the two possible resizing modes. */

class geFlexResizable : public geFlex
{
public:
	geFlexResizable(Direction, geResizerBar::Mode);

	void resize(int x, int y, int w, int h) override;

	void addWidget(Fl_Widget*, int size = -1) override;
	void addWidget(Fl_Widget&, int size = -1) override;

	/* resizeWidget
	Resizes widget at index 'index' to size 'size'. */

	void resizeWidget(int index, int size);

	/* getWidget
	Returns widget at intex 'index'. */

	Fl_Widget& getWidget(int index);

	/* onDragBar
	Callback fired when one of the resizer bars is being dragged around. The
	Fl_Widget parameter is a reference to the widget that is being resized by
	the resizer bar. */

	std::function<void(const Fl_Widget&)> onDragBar;

private:
	int computeHeight() const;
	int getWidgetMainSize(const Fl_Widget*) const;

	void makeLastWidgetFlex();
	void makeLastWidgetFixed();

	geResizerBar::Mode         m_mode;
	std::vector<geResizerBar*> m_bars;
	std::vector<Fl_Widget*>    m_widgets;
};
} // namespace giada::v

#endif
