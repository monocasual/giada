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

#ifndef GE_TABLE_BASE_H
#define GE_TABLE_BASE_H

#include <FL/Fl_Table.H>
#include <functional>

namespace giada::v
{
/* geTableBase
A base class for a table. */

class geTableBase : public Fl_Table
{
public:
	void draw() override;

	std::function<void(int column, bool doubleClick)>          onClickHeader;
	std::function<void(int row, int column, bool doubleClick)> onClickRow;
	std::function<void(int column, int width)>                 onResizeColumn;

protected:
	geTableBase();

	void loadLayout(const std::vector<int>&);

	/* fitContent
	Adjusts each column's width so that it automatically fits the widest
	content in that column. Requires getContentWidth() to be implemented. */

	void fitContent();

	/* getContentWidth (virtual)
	Implement this if you want to fit all columns to content. */

	virtual int getContentWidth(int /*row*/, int /*column*/) const { return -1; }

	void forEachCell(std::function<void(int, int, int, int, int, int)>);

private:
	static void cb_onClick(Fl_Widget*, void*);
	void        cb_onClick();
};
} // namespace giada::v

#endif
