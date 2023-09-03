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

#include "gui/elems/basics/flexResizable.h"
#include "core/const.h"
#include "gui/elems/basics/box.h"
#include <cassert>

namespace giada::v
{
geFlexResizable::geFlexResizable(Direction d, geResizerBar::Mode mode)
: geFlex(d)
, onDragBar(nullptr)
, m_mode(mode)
{
}

/* -------------------------------------------------------------------------- */

void geFlexResizable::addWidget(Fl_Widget& widget, int size)
{
	/* Add resizer bar only if there is at least one child. */

	if (children() > 0)
	{
		/* TODO - geResizerBar is glitchy when added in a flex layout: if width or
		height is 0 it just doesn't show up. */

		geResizerBar* bar = new geResizerBar(0, 0, G_GUI_INNER_MARGIN, G_GUI_INNER_MARGIN, G_GUI_UNIT, getDirection(), m_mode);
		geFlex::addWidget(bar, G_GUI_INNER_MARGIN);
		m_bars.push_back(bar);

		bar->onDrag = [this](const Fl_Widget& wg) {
			if (onDragBar != nullptr)
				onDragBar(wg);
		};
	}

	geFlex::addWidget(widget);
	geFlex::size(w(), computeHeight());
}

void geFlexResizable::addWidget(Fl_Widget* widget, int size)
{
	addWidget(*widget, size);
}

/* -------------------------------------------------------------------------- */

void geFlexResizable::resizeWidget(int index, int size)
{
	assert(index >= 0 && static_cast<size_t>(index) < m_bars.size());

	geResizerBar*    bar         = m_bars[index];
	const Fl_Widget& firstWidget = bar->getFirstWidget();
	const int        offset      = getDirection() == Direction::HORIZONTAL ? firstWidget.x() - x() : firstWidget.y() - y();

	bar->moveTo(offset + size);
}

/* -------------------------------------------------------------------------- */

Fl_Widget& geFlexResizable::getWidget(int index) { return *m_widgets[index]; }

/* -------------------------------------------------------------------------- */

int geFlexResizable::computeHeight() const
{
	const auto last   = child(children() - 1);
	const int  height = (last->y() + last->h()) - y();
	return height;
}
} // namespace giada::v
