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

#include "gui/elems/basics/flexResizable.h"
#include "core/const.h"
#include "gui/elems/basics/box.h"
#include <cassert>

namespace giada::v
{
geFlexResizable::geFlexResizable(int x, int y, int w, int h, Direction d, geResizerBar::Mode mode)
: geFlex(x, y, w, h, d)
, onDragBar(nullptr)
, endMargin(0)
, m_mode(mode)
{
}

/* -------------------------------------------------------------------------- */

geFlexResizable::geFlexResizable(Direction d, geResizerBar::Mode mode)
: geFlexResizable(0, 0, 0, 0, d, mode)
{
}

/* -------------------------------------------------------------------------- */

void geFlexResizable::resize(int newX, int newY, int newW, int newH)
{
	geFlex::resize(newX, newY, newW, newH);

	/* In Mode::RESIZE, we must make sure the last widget, which is flex by
	default, doesn't get shrinked lower than G_GUI_UNIT when this flex container
	is resized. If it happens, the trick is to make the last widget fixed, while
	everything else flex. */

	if (m_widgets.size() < 2 || m_mode == geResizerBar::Mode::MOVE)
		return;

	const Fl_Widget* lastWidget     = m_widgets.back();
	const int        lastWidgetSize = getWidgetMainSize(*lastWidget);

	if (lastWidgetSize < G_GUI_UNIT)
		makeLastWidgetFixed();
	else
		makeLastWidgetFlex();
}

/* -------------------------------------------------------------------------- */

void geFlexResizable::addWidget(Fl_Widget& widget, int size)
{
	/* In RESIZE mode the bar is added before the widget, but only if there
	    is at least one child. */

	if (m_mode == geResizerBar::Mode::RESIZE && children() > 0)
		addResizerBar();

	geFlex::addWidget(widget, size);
	m_widgets.push_back(&widget);

	/* In MOVE mode the bar is added after the widget, so that also the last
	one can be resized. */

	if (m_mode == geResizerBar::Mode::MOVE)
		addResizerBar();

	adjustMainSize();
}

void geFlexResizable::addWidget(Fl_Widget* widget, int size)
{
	addWidget(*widget, size);
}

/* -------------------------------------------------------------------------- */

void geFlexResizable::clearWidgets()
{
	geFlex::clear();
	m_bars.clear();
	m_widgets.clear();
}

/* -------------------------------------------------------------------------- */

void geFlexResizable::resizeWidget(int index, int size)
{
	assert(index >= 0 && static_cast<size_t>(index) < m_bars.size());

	geResizerBar*    bar         = m_bars[index];
	const Fl_Widget& firstWidget = bar->getFirstWidget();
	const int        offset      = getWidgetMainPosition(firstWidget) - getMainPosition();

	bar->moveTo(offset + size);
}

/* -------------------------------------------------------------------------- */

Fl_Widget& geFlexResizable::getWidget(int index) { return *m_widgets[index]; }

/* -------------------------------------------------------------------------- */

int geFlexResizable::computeMainSize() const
{
	const auto last     = child(children() - 1);
	const int  thisPos  = getMainPosition();
	const int  lastPos  = getWidgetMainPosition(*last);
	const int  lastSize = getWidgetMainSize(*last);
	const int  size     = ((lastPos + lastSize) - thisPos) + endMargin;
	return size;
}

/* -------------------------------------------------------------------------- */

int geFlexResizable::getMainPosition() const
{
	return getDirection() == Direction::VERTICAL ? y() : x();
}

int geFlexResizable::getMainSize() const
{
	return getDirection() == Direction::VERTICAL ? h() : w();
}

/* -------------------------------------------------------------------------- */

int geFlexResizable::getWidgetMainPosition(const Fl_Widget& wg) const
{
	return getDirection() == Direction::VERTICAL ? wg.y() : wg.x();
}

int geFlexResizable::getWidgetMainSize(const Fl_Widget& wg) const
{
	return getDirection() == Direction::VERTICAL ? wg.h() : wg.w();
}

/* -------------------------------------------------------------------------- */

void geFlexResizable::makeLastWidgetFlex()
{
	for (Fl_Widget* wg : m_widgets)
		geFlex::fixed(wg, getWidgetMainSize(*wg));
	geFlex::fixed(m_widgets.back(), -1);
}

/* -------------------------------------------------------------------------- */

void geFlexResizable::makeLastWidgetFixed()
{
	for (Fl_Widget* wg : m_widgets)
		geFlex::fixed(wg, -1);
	geFlex::fixed(m_widgets.back(), G_GUI_UNIT);
}

/* -------------------------------------------------------------------------- */

void geFlexResizable::addResizerBar()
{
	/* TODO - geResizerBar is glitchy when added in a flex layout: if width or
	    height is 0 it just doesn't show up. */

	geResizerBar* bar = new geResizerBar(0, 0, G_GUI_INNER_MARGIN, G_GUI_INNER_MARGIN, G_GUI_UNIT, getDirection(), m_mode);
	geFlex::addWidget(bar, G_GUI_INNER_MARGIN);
	m_bars.push_back(bar);

	bar->onDrag = [this](const Fl_Widget& wg)
	{
		if (onDragBar != nullptr)
			onDragBar(wg);

		/*  The whole container gets resized in MOVE mode. */

		if (m_mode == geResizerBar::Mode::MOVE)
			adjustMainSize();
	};

	/* The widget connected to the drag bar becomes fixed, when resized. */

	bar->onRelease = [this](const Fl_Widget& wg)
	{
		geFlex::fixed(const_cast<Fl_Widget&>(wg), getWidgetMainSize(wg));
	};
}

/* -------------------------------------------------------------------------- */

void geFlexResizable::adjustMainSize()
{
	if (getDirection() == Direction::HORIZONTAL)
		geFlex::size(computeMainSize(), h());
	else
		geFlex::size(w(), computeMainSize());
}
} // namespace giada::v
