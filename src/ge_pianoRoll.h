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
#include "ge_actionWidget.h"


class gPianoRollContainer : public Fl_Scroll {
private:
	class gdActionEditor *pParent;
	class gPianoRoll     *pianoRoll;

public:
	gPianoRollContainer(int x, int y, class gdActionEditor *parent);
	int  handle(int e);
	void draw();
};


/* ------------------------------------------------------------------ */


class gPianoRoll : public gActionWidget {

public:
	gPianoRoll(int x, int y, int w, class gdActionEditor *parent);
	void draw();
};

#endif
