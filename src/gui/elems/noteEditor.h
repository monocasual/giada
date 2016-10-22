/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_PIANOROLL_H
#define GE_PIANOROLL_H

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include "../../core/recorder.h"
#include "baseActionEditor.h"


class geNoteEditor : public Fl_Scroll
{
private:

	class gdActionEditor *pParent;
	class gePianoRoll    *pianoRoll;

public:

	geNoteEditor(int x, int y, class gdActionEditor *parent);
	~geNoteEditor();
	void draw();
	void updateActions();
};


/* -------------------------------------------------------------------------- */


class gePianoRoll : public geBaseActionEditor
{
private:

	/* onItem
	 * is curson on a gePianoItem? */

	bool onItem(int rel_x, int rel_y);

	/* drawSurface*
	 * generate a complex drawing in memory first and copy it to the
	 * screen at a later point in time. Fl_Offscreen surface holds the
	 * necessary data. */

	/* drawSurface1
	 * draw first tile of note values. */

	void drawSurface1();

	/* drawSurface2
	 * draw the rest of the piano roll. */

	void drawSurface2();

	int  push_y;
	Fl_Offscreen surface1;  // notes, no repeat
	Fl_Offscreen surface2;  // lines, x-repeat


public:

	gePianoRoll(int x, int y, int w, class gdActionEditor *pParent);

	void draw();
	int  handle(int e);
	void updateActions();

	enum
	{
		MAX_NOTES = 127,
		CELL_H    = 15
	};
};


#endif
