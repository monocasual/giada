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

#include "src/gui/elems/basics/tableBase.h"
#include "src/gui/const.h"
#include "src/gui/drawing.h"
#include "src/gui/elems/basics/boxtypes.h"

namespace giada::v
{
geTableBase::geTableBase()
: Fl_Table(0, 0, 0, 0)
, onClickHeader(nullptr)
, onClickRow(nullptr)
, onResizeColumn(nullptr)
{
	end();

	vscrollbar->color(G_COLOR_GREY_2);
	vscrollbar->selection_color(G_COLOR_GREY_4);
	vscrollbar->labelcolor(G_COLOR_LIGHT_1);
	vscrollbar->slider(G_CUSTOM_BORDER_BOX);

	hscrollbar->color(G_COLOR_GREY_2);
	hscrollbar->selection_color(G_COLOR_GREY_4);
	hscrollbar->labelcolor(G_COLOR_LIGHT_1);
	hscrollbar->slider(G_CUSTOM_BORDER_BOX);

	col_resize_min(G_GUI_UNIT);

	when(FL_WHEN_CHANGED); // Enable 'resize' events

	callback(cb_onClick, this);
}

/* -------------------------------------------------------------------------- */

void geTableBase::draw()
{
	Fl_Table::draw();
	drawRect({x(), y(), w(), h()}, G_COLOR_GREY_4);
}

/* -------------------------------------------------------------------------- */

void geTableBase::forEachCell(std::function<void(int, int, int, int, int, int)> f)
{
	for (int row = 0; row < rows(); row++)
	{
		for (int col = 0; col < cols(); col++)
		{
			int X, Y, W, H;
			find_cell(CONTEXT_TABLE, row, col, X, Y, W, H);
			f(row, col, X, Y, W, H);
		}
	}
}

/* -------------------------------------------------------------------------- */

void geTableBase::cb_onClick(Fl_Widget* /*w*/, void* p) { (static_cast<geTableBase*>(p))->cb_onClick(); }

/* -------------------------------------------------------------------------- */

void geTableBase::cb_onClick()
{
	if (Fl::event() == FL_RELEASE)
		return;

	const TableContext context = callback_context();
	if (context & CONTEXT_COL_HEADER && onClickHeader != nullptr)
		onClickHeader(callback_col(), Fl::event_clicks() > 0);
	else if (context & CONTEXT_CELL && onClickRow != nullptr)
		onClickRow(callback_row(), callback_col(), Fl::event_clicks() > 0);
	else if (context & CONTEXT_RC_RESIZE && col_width(callback_col()) > 0 && onResizeColumn != nullptr)
		onResizeColumn(callback_col(), col_width(callback_col()));
}

/* -------------------------------------------------------------------------- */

void geTableBase::loadLayout(const std::vector<int>& layout)
{
	assert(layout.size() == static_cast<std::size_t>(cols()));

	int i = 0;
	for (const int width : layout)
		col_width(i++, width);
}

/* -------------------------------------------------------------------------- */

void geTableBase::fitContent()
{
	if (getContentWidth(0, 0) == -1)
		return;

	for (int column = 0; column < cols(); column++)
	{
		int width = 0;
		for (int row = 0; row < rows(); row++)
			width = std::max(width, getContentWidth(row, column));
		col_width(column, width);
	}
}
} // namespace giada::v
