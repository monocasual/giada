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

#include "gui/elems/basics/table.h"
#include "core/const.h"
#include "gui/const.h"
#include "gui/drawing.h"
#include "gui/elems/basics/boxtypes.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

namespace giada::v
{
geTable::geTable()
: Fl_Table(0, 0, 0, 0)
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
}

/* -------------------------------------------------------------------------- */

void geTable::draw()
{
	Fl_Table::draw();
	drawRect({x(), y(), w(), h()}, G_COLOR_GREY_4);
}

/* -------------------------------------------------------------------------- */

void geTable::init()
{
	forEachCell([this](int row, int col, int X, int Y, int W, int H)
	{ add(setCellContent(row, col, X, Y, W, H)); });
}

/* -------------------------------------------------------------------------- */

void geTable::draw_cell(TableContext context, int /*row*/, int col, int X, int Y, int W, int H)
{
	switch (context)
	{
	case CONTEXT_ROW_HEADER: // Row header cell needs to be drawn
	case CONTEXT_COL_HEADER: // Column header cell needs to be drawn
	{
		const geompp::Rect<int> bounds = {X, Y, W, H};
		fl_push_clip(X, Y, W, H);
		drawRectf(bounds, G_COLOR_GREY_1);                     // Table background
		drawRectf(bounds.withTrimmedRight(2), G_COLOR_GREY_3); // Label background
		drawText(setHeaderText(col), bounds, FL_HELVETICA, G_GUI_FONT_SIZE_BASE, G_COLOR_LIGHT_2, FL_ALIGN_CENTER);
		fl_pop_clip();
		return;
	}

	case CONTEXT_RC_RESIZE: // Table or row/column is resized or scrolled
	{
		int index = 0;
		forEachCell([this, &index](int /*row*/, int /*col*/, int X, int Y, int W, int H)
		{ 
			if (index < children()) 
				child(index++)->resize(X, Y, W, H); });
		init_sizes(); // tell group children resized
		return;
	}

	default:
		return;
	}
}

/* -------------------------------------------------------------------------- */

void geTable::forEachCell(std::function<void(int, int, int, int, int, int)> f)
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
} // namespace giada::v
