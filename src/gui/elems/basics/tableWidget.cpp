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

#include "src/gui/elems/basics/tableWidget.h"
#include "src/gui/const.h"
#include "src/gui/drawing.h"

namespace giada::v
{
geTableWidget::geTableWidget()
: geTableBase()
{
}

/* -------------------------------------------------------------------------- */

void geTableWidget::init()
{
	forEachCell([this](int row, int col, geompp::Rect<int> cellBounds)
	{ add(setCellContent(row, col, cellBounds)); });
}

/* -------------------------------------------------------------------------- */

void geTableWidget::draw_cell(TableContext context, int /*row*/, int col, int X, int Y, int W, int H)
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
		forEachCell([this, &index](int /*row*/, int /*col*/, geompp::Rect<int> cellBounds)
		{ 
			if (index < children()) 
				child(index++)->resize(cellBounds.x, cellBounds.y, cellBounds.w, cellBounds.h); });
		init_sizes(); // tell group children resized
		return;
	}

	default:
		return;
	}
}
} // namespace giada::v
