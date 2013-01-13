/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_actionWidget
 *
 * parent class of any widget inside the action editor.
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


#include "ge_actionWidget.h"
#include "gd_actionEditor.h"
#include "mixer.h"


extern Mixer G_Mixer;


gActionWidget::gActionWidget(int x, int y, int w, int h, gdActionEditor *parent)
	:	Fl_Group(x, y, w, h), parent(parent) {}


/* ------------------------------------------------------------------ */


gActionWidget::~gActionWidget() {}


/* ------------------------------------------------------------------ */


void gActionWidget::baseDraw() {

	/* widen or narrow the widget */

	w(parent->totalWidth);

	/* clear the screen */

	fl_rectf(x(), y(), parent->totalWidth, h(), COLOR_BG_MAIN);

	/* draw the container */

	fl_color(COLOR_BD_0);
	fl_rect(x(), y(), parent->totalWidth, h());

	/* print beats and bars. The method is the same of the waveform in sample
	 * editor. Take totalwidth (the width in pixel of the area to draw), knowing
	 * that totalWidth = totalFrames / zoom. Then, for each pixel of totalwidth,
	 * put a concentrate of each block (which is totalFrames / zoom). */

	int    j = 0;
	bool end = false;
	bool wantHidden = false;

	/* don't draw beyond the hidden area. If 'end': stop drawing. */

	if (G_Mixer.beats == 32)
		parent->coverX = w()+x()-1;
	else
		wantHidden = true;


	/* grid drawing, if > 1 */

	if (parent->gridTool->getValue() > 1) {
		fl_color(fl_rgb_color(54, 54, 54));
		fl_line_style(FL_DASH, 0, NULL);
		for (int i=0; i<(int) parent->gridTool->points.size; i++) {
			int px = parent->gridTool->points.at(i)+x()-1;
			fl_line(px, y()+1, px, y()+h()-2);
			/*
			char c[16];
			sprintf(c, "%d (%d)", px, parent->gridTool->points.at(i));
			fl_draw(c, px+1, y()+h()-4);
			*/
		}
		fl_line_style(0);
	}

	/* box and beat separator drawing */

	for (int i=1; i<parent->totalWidth && !end; i++) {
		int step = parent->zoom*i;
		while (j < step && j <= parent->totalFrames) {

			if (wantHidden && j >= parent->framesPerBeats) {        // search for the start point
				parent->coverX = i+x()-1;                             // of the hidden area
				end = true;
			}

			if (j % parent->framesPerBeat == 0) {                            // print each beat
				fl_color(COLOR_BD_0);
				fl_line(i+x()-1, y()+1, i+x()-1, y()+h()-2);
			}

			if (j % parent->framesPerBar == 0 && i!=1 && i != parent->totalWidth) {    // print bar division
				fl_color(COLOR_BG_2);
				fl_line(i+x()-1, y()+1, i+x()-1, y()+h()-2);
			}
			j+=2;
		}
		j = step;
	}
}
