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


#include <FL/fl_draw.H>
#include "ge_pianoRoll.h"
#include "gd_actionEditor.h"
#include "const.h"


gPianoRollContainer::gPianoRollContainer(int x, int y, class gdActionEditor *pParent)
 : Fl_Scroll(x, y, 200, 80), pParent(pParent)
{
	size(pParent->totalWidth, h());
	type(0); // no scrollbars

	pianoRoll = new gPianoRoll(x, y, pParent->totalWidth, pParent);
}


/* ------------------------------------------------------------------ */


void gPianoRollContainer::draw() {

	pianoRoll->size(this->w(), pianoRoll->h());  /// <--- not optimal


	/* clear background */

	fl_rectf(x(), y(), w(), h(), COLOR_BG_MAIN);

	fl_push_clip(x(), y(), w(), h());
	draw_child(*pianoRoll);
	fl_pop_clip();

	fl_color(COLOR_BD_0);
	fl_line_style(0);
	fl_rect(x(), y(), pParent->totalWidth, h());
}


/* ------------------------------------------------------------------ */


int gPianoRollContainer::handle(int e) {

	int ret = Fl_Group::handle(e);

	switch (e) {
		case FL_PUSH:	{
			ret = 1;
		}
		case FL_DRAG: {
			if (Fl::event_button3()) {
				scroll_to(xposition(), y()-Fl::event_y());
				ret = 1;
			}
		}
	}

	return ret;
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gPianoRoll::gPianoRoll(int x, int y, int w, class gdActionEditor *parent)
 : gActionWidget(x, y, w, 40, parent)
{
	size(w, 128 * 15);  // 128 MIDI channels * 15 px height
}


/* ------------------------------------------------------------------ */


void gPianoRoll::draw() {

	baseDraw();

	//gPianoRollContainer *grp = (gPianoRollContainer*) parent();

	fl_color(fl_rgb_color(54, 54, 54));
	fl_line_style(FL_DASH, 0, NULL);
	fl_font(FL_HELVETICA, 11);

	int octave = 9;

	for (int i=1; i<=128; i++) {

		/* print key note label. C C# D D# E F F# G G# A A# B */

		char note[6];
		int  step = i % 12;

		if      (step == 1)
			sprintf(note, "%dG", octave);
		else if (step == 2)
			sprintf(note, "%dF#", octave);
		else if (step == 3)
			sprintf(note, "%dF", octave);
		else if (step == 4)
			sprintf(note, "%dE", octave);
		else if (step == 5)
			sprintf(note, "%dD#", octave);
		else if (step == 6)
			sprintf(note, "%dD", octave);
		else if (step == 7)
			sprintf(note, "%dC#", octave);
		else if (step == 8)
			sprintf(note, "%dC", octave);
		else if (step == 9)
			sprintf(note, "%dB", octave);
		else if (step == 10)
			sprintf(note, "%dA#", octave);
		else if (step == 11)
			sprintf(note, "%dA", octave);
		else if (step == 0) {
			sprintf(note, "%dG#", octave);
			octave--;
		}

		fl_draw(note, x()+4, y()+((i-1)*15)+1, 30, 15, (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_CENTER));

		/* print horizontal line */

		if (i < 128)
			fl_line(x()+1, y()+i*15, x()+w()-2, y()+i*15);
	}

	fl_line_style(0);
}
