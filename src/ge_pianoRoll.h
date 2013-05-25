/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_pianoRoll
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#ifndef GE_PIANOROLL_H
#define GE_PIANOROLL_H

#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include "ge_actionWidget.h"
#include "recorder.h"


class gPianoRollContainer : public Fl_Scroll {
private:
	class gdActionEditor *pParent;
	class gPianoRoll     *pianoRoll;

public:
	gPianoRollContainer(int x, int y, class gdActionEditor *parent);

#if 0
	int  handle(int e);
#endif
	void draw();
};


/* ------------------------------------------------------------------ */


class gPianoRoll : public gActionWidget {
private:

	/* onItem
	 * is curson on a gPianoItem? */

	bool onItem();

	/* drawSurface
	 * generate a complex drawing in memory first and copy it to the
	 * screen at a later point in time. Fl_Offscreen surface holds the
	 * necessary data. */

	void drawSurface();

	int  push_y;
	Fl_Offscreen surface;


public:
	gPianoRoll(int x, int y, int w, class gdActionEditor *pParent);

	void draw();
	int  handle(int e);
};


/* ------------------------------------------------------------------ */


class gPianoItem : public Fl_Box {
private:
	recorder::action *a;
	class gdActionEditor *pParent;

	bool selected;
	int  push_x;

	/* MIDI note, start frame, end frame. Used only if it's a newly added
	 * action */

	int  note;
	int  frame_a;
	int  frame_b;

	bool onLeftEdge;
	bool onRightEdge;


public:

	/* pianoItem ctor
	 * if action *a == NULL, record a new action */

	gPianoItem(int x, int y, int rel_x, int rel_y, recorder::action *a, class gdActionEditor *pParent);

	void draw();
	int  handle(int e);
	void record();

};

#endif
