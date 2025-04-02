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

#include "src/gui/elems/basics/tableText.h"
#include "src/gui/const.h"
#include "src/gui/drawing.h"

namespace giada::v
{
geTableText::geTableText()
: geTableBase()
{
}

/* -------------------------------------------------------------------------- */

int geTableText::handle(int event)
{
	/* Force full-row selection on click/keyboard press. */

	const int ret = Fl_Table::handle(event);
	switch (event)
	{
	case FL_PUSH:
	case FL_KEYDOWN:
	case FL_DRAG:
	{
		if (callback_context() == Fl_Table::CONTEXT_CELL)
		{
			const geompp::Range<int> selection = getSelection();
			set_selection(selection.a, 0, selection.b - 1, cols());
		}
		else
			set_selection(-1, -1, -1, -1);
		break;
	}
	}
	return ret;
}

/* -------------------------------------------------------------------------- */

void geTableText::draw_cell(TableContext context, int row, int col, int X, int Y, int W, int H)
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
		drawText(getHeaderText(col), bounds, FL_HELVETICA, G_GUI_FONT_SIZE_BASE, G_COLOR_LIGHT_2, FL_ALIGN_CENTER);
		fl_pop_clip();
		return;
	}
	case Fl_Table::CONTEXT_CELL:
	{
		const geompp::Rect<int> bounds = {X, Y, W, H};
		fl_push_clip(X, Y, W, H);
		const auto backgroundColor = is_selected(row, col) ? G_COLOR_GREY_3 : G_COLOR_GREY_1;
		drawRectf(bounds, backgroundColor); // Table background
		drawText(getCellText(row, col), bounds, FL_HELVETICA, G_GUI_FONT_SIZE_BASE, G_COLOR_LIGHT_2, FL_ALIGN_LEFT);
		fl_pop_clip();
		return;
	}

	default:
		return;
	}
}

/* -------------------------------------------------------------------------- */

geompp::Range<int> geTableText::getSelection()
{
	int rowTop;
	int columnLeft;
	int rowBottom;
	int columnRight;
	get_selection(rowTop, columnLeft, rowBottom, columnRight);
	if (rowTop == -1 || rowBottom == -1)
		return {};
	return {rowTop, rowBottom + 1};
}
} // namespace giada::v
