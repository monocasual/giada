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

#ifndef GE_TABLE_H
#define GE_TABLE_H

#include "deps/geompp/src/rect.hpp"
#include <FL/Fl_Table.H>
#include <functional>
#include <string>

namespace giada::v
{
/* geTable
A table where each cell contains a Fl_Widget. TODO - geTableText for text-only
content. */

class geTable : public Fl_Table
{
public:
	virtual Fl_Widget*  setCellContent(int row, int col, int x, int y, int w, int h) = 0;
	virtual std::string setHeaderText(int col)                                       = 0;

	void draw() override;

	void init();

protected:
	geTable();

private:
	void draw_cell(TableContext, int row, int col, int x, int y, int w, int h) override;

	void forEachCell(std::function<void(int, int, int, int, int, int)>);
};
} // namespace giada::v

#endif
