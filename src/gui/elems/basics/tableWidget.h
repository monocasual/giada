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

#ifndef GE_TABLE_WIDGET_H
#define GE_TABLE_WIDGET_H

#include "src/gui/elems/basics/tableBase.h"
#include <string>

namespace giada::v
{
/* geTable
A table where each cell contains a Fl_Widget. */

class geTableWidget : public geTableBase
{
public:
	virtual Fl_Widget*  getCellContent(int row, int col, geompp::Rect<int> cellBounds) const = 0;
	virtual std::string getHeaderText(int col) const                                         = 0;

	void init();

protected:
	geTableWidget();

private:
	void draw_cell(TableContext, int row, int col, int x, int y, int w, int h) override;
};
} // namespace giada::v

#endif
