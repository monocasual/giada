/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_PIANO_ROLL_H
#define GE_PIANO_ROLL_H


#include <FL/fl_draw.H>
#include "baseActionEditor.h"


class gePianoRoll : public geBaseActionEditor
{
private:

	enum class Notes
	{
		G = 1, FS = 2, F = 3, E = 4, DS = 5, D = 6, CS = 7, C = 8, B = 9, AS = 10,
		A = 11, GS = 0
	};

	/* drawSurface*
	Generates a complex drawing in memory first and copy it to the screen at a
	later point in time. Fl_Offscreen surface holds the necessary data.	The first
	call creates an offscreen surface of CELL_W pixel wide containing note values.
	The second call creates another offscreen surface of CELL_W pixels wide
	containing the rest of the piano roll. The latter will then be tiled during
	the ::draw() call. */

	void drawSurface1();
	void drawSurface2();

	int push_y;

	Fl_Offscreen surface1;  // notes, no repeat
	Fl_Offscreen surface2;  // lines, x-repeat

public:

	static const int MAX_KEYS    = 127;
	static const int MAX_OCTAVES = 9;
	static const int KEYS        = 12;
	static const int CELL_H      = 18;
	static const int CELL_W      = 40;

	gePianoRoll(int x, int y, int w, class gdActionEditor *pParent);

	void draw();
	int  handle(int e);
	void updateActions();

	/* cursorOnItem
	Defines wheter the cursor is over a piano item. This value is updated by each
	gePianoItem sub-widget. */

	bool cursorOnItem;
};


#endif
