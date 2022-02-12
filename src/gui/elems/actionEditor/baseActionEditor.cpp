/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/dialogs/actionEditor/baseActionEditor.h"
#include "baseAction.h"
#include "baseActionEditor.h"
#include "core/const.h"
#include "core/sequencer.h"
#include "gridTool.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

namespace giada::v
{
geBaseActionEditor::geBaseActionEditor(Pixel x, Pixel y, Pixel w, Pixel h,
    gdBaseActionEditor* base)
: Fl_Group(x, y, w, h)
, m_data(nullptr)
, m_base(base)
, m_action(nullptr)
{
}

/* -------------------------------------------------------------------------- */

geBaseAction* geBaseActionEditor::getActionAtCursor() const
{
	for (int i = 0; i < children(); i++)
	{
		geBaseAction* a = static_cast<geBaseAction*>(child(i));
		if (a->hovered)
			return a;
	}
	return nullptr;
}

/* -------------------------------------------------------------------------- */

void geBaseActionEditor::baseDraw(bool clear) const
{
	/* Clear the screen. */

	if (clear)
		fl_rectf(x(), y(), w(), h(), G_COLOR_GREY_1);

	/* Draw the outer container. */

	fl_color(G_COLOR_GREY_4);
	fl_rect(x(), y(), w(), h());

	/* Draw grid, beats and bars. A grid set to 1 has a cell size == beat, so
	painting it is useless. */

	if (m_base->gridTool.getValue() > 1)
	{
		fl_color(G_COLOR_GREY_3);
		drawVerticals(m_base->gridTool.getCellSize(m_data->framesInBeat));
	}

	fl_color(G_COLOR_GREY_4);
	drawVerticals(m_data->framesInBeat);

	fl_color(G_COLOR_LIGHT_1);
	drawVerticals(m_data->framesInBar);

	/* Cover unused area. Avoid drawing cover if width == 0 (i.e. beats are 32). */

	Pixel coverWidth = m_base->fullWidth - m_base->loopWidth;
	if (coverWidth != 0)
		fl_rectf(m_base->loopWidth + x(), y() + 1, coverWidth, h() - 2, G_COLOR_GREY_4);
}

/* -------------------------------------------------------------------------- */

void geBaseActionEditor::drawVerticals(int steps) const
{
	/* Start drawing from steps, not from 0. The zero-th element is always 
	graphically useless. */
	for (Frame i = steps; i < m_data->framesInLoop; i += steps)
	{
		Pixel p = m_base->frameToPixel(i) + x();
		fl_line(p, y() + 1, p, y() + h() - 2);
	}
}

/* -------------------------------------------------------------------------- */

int geBaseActionEditor::handle(int e)
{
	switch (e)
	{
	case FL_PUSH:
		return push();
	case FL_DRAG:
		return drag();
	case FL_RELEASE:
		fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK); // Make sure cursor returns normal
		return release();
	default:
		return Fl_Group::handle(e);
	}
}

/* -------------------------------------------------------------------------- */

int geBaseActionEditor::push()
{
	m_action = getActionAtCursor();

	if (Fl::event_button1())
	{ // Left button
		if (m_action == nullptr)
		{                                          // No action under cursor: add a new one
			if (Fl::event_x() < m_base->loopWidth) // Avoid click on grey area
				onAddAction();
		}
		else // Prepare for dragging
			m_action->pick = Fl::event_x() - m_action->x();
	}
	else if (Fl::event_button3())
	{ // Right button
		if (m_action != nullptr)
		{
			onDeleteAction();
			m_action = nullptr;
		}
	}
	return 1;
}

/* -------------------------------------------------------------------------- */

int geBaseActionEditor::drag()
{
	if (m_action == nullptr)
		return 0;
	if (m_action->isOnEdges())
		onResizeAction();
	else
		onMoveAction();
	m_action->altered = true;
	redraw();
	return 1;
}

/* -------------------------------------------------------------------------- */

int geBaseActionEditor::release()
{
	int ret = 0;
	if (m_action != nullptr && m_action->altered)
	{
		onRefreshAction();
		ret = 1;
	}
	m_action = nullptr;
	return ret;
}
} // namespace giada::v