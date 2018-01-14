/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_GRID_TOOL_H
#define GE_GRID_TOOL_H


#include <vector>
#include <FL/Fl_Group.H>


class geChoice;
class geCheck;
class gdActionEditor;


class geGridTool : public Fl_Group
{
private:

  geChoice *gridType;
	geCheck  *active;

	gdActionEditor *parent;

	static void cb_changeType(Fl_Widget *w, void *p);
	inline void __cb_changeType();

public:

	geGridTool(int x, int y, gdActionEditor *parent);
	~geGridTool();

	int  getValue();
	bool isOn();
	void calc();

	/* getSnapPoint
	 * given a cursor position in input, return the x coordinates of the
	 * nearest snap point (in pixel, clean, ie. not x()-shifted) */

	int getSnapPoint(int v);
	int getSnapFrame(int v);

	/* getCellSize
	 * return the size in pixel of a single cell of the grid. */

	int getCellSize();

	std::vector<int> points;   // points of the grid
	std::vector<int> frames;   // frames of the grid

	std::vector<int> bars;
	std::vector<int> beats;
};


#endif
