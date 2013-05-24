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
#include "channel.h"
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

#if 0
int gPianoRollContainer::handle(int e) {

	int ret = Fl_Group::handle(e);

	switch (e) {
		case FL_PUSH:	{
			ret = 1;
			break;
		}
		case FL_DRAG: {
			if (Fl::event_button3())
				scroll_to(xposition(), y()-Fl::event_y());
			ret = 1;
			break;
		}
	}

	return ret;
}
#endif


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gPianoRoll::gPianoRoll(int x, int y, int w, class gdActionEditor *pParent)
 : gActionWidget(x, y, w, 40, pParent)
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

	draw_children();
}


/* ------------------------------------------------------------------ */


int gPianoRoll::handle(int e) {

	int ret = Fl_Group::handle(e);

	switch (e) {
		case FL_PUSH:	{

			push_y = Fl::event_y() - y();

			if (Fl::event_button1()) {

				/* ax is driven by grid, ay by the height in px of each note */

				int ax = Fl::event_x();
				int ay = Fl::event_y();

				/* vertical snap */

				int edge = (ay-y()-3) % 15;
				if (edge != 0) ay -= edge;

				/* horizontal snap (grid tool) TODO */

				add(new gPianoItem(ax, ay, ax-x(), ay-y()-3, NULL, pParent));
				redraw();
			}
			ret = 1;
			break;
		}
		case FL_DRAG: {
			if (Fl::event_button3()) {
				position(x(), Fl::event_y() - push_y);
				((gPianoRollContainer*)parent())->redraw();
			}
			ret = 1;
			break;
		}
	}
	return ret;
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gPianoItem::gPianoItem(int x, int y, int rel_x, int rel_y, recorder::action *a, gdActionEditor *pParent)
	: Fl_Box  (x, y, 20, 10),
	  a       (a),
		pParent (pParent),
		selected(false)
{
	if (a)
		puts("[gPianoItem] new gPianoItem, display mode");
	else {
		note    = 127 - (rel_y / 15);
		frame_a = rel_x * pParent->zoom;
		frame_b = frame_a + 4096;
		printf("[gPianoItem] new gPianoItem, record mode, note %d, frame_a %d, frame_b %d\n", note, frame_a, frame_b);
		record();
	}
}


/* ------------------------------------------------------------------ */


void gPianoItem::draw() {
	fl_rectf(x(), y(), w(), h(), (Fl_Color) selected ? COLOR_BD_1 : COLOR_BG_2);
}


/* ------------------------------------------------------------------ */


void gPianoItem::record() {

	/* note on */

	uint32_t event = 0;
	event |= (0x90 << 24);   // note on
	event |= (note << 16);   // note value
	event |= (0x3F <<  8);   // velocity
	event |= (0x00);
	recorder::rec(pParent->chan->index, ACTION_MIDI, frame_a, event);

	/* note off */

	event = 0;
	event |= (0x80 << 24);   // note off
	event |= (note << 16);   // note value
	event |= (0x3F <<  8);   // velocity
	event |= (0x00);
	recorder::rec(pParent->chan->index, ACTION_MIDI, frame_b, event);
}


/* ------------------------------------------------------------------ */


int gPianoItem::handle(int e) {
	int ret = 0;
	switch (e) {
		case FL_ENTER: {
			selected = true;
			ret = 1;
			redraw();
			break;
		}
		case FL_LEAVE: {
			fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
			selected = false;
			ret = 1;
			redraw();
			break;
		}
		case FL_PUSH: {
			recorder::deleteAction(pParent->chan->index, frame_a, ACTION_MIDI);
			recorder::deleteAction(pParent->chan->index, frame_b, ACTION_MIDI);
			Fl::delete_widget(this);
			((gPianoRoll*)parent())->redraw();
			ret = 1;
			break;
		}
	}
	return ret;
}
