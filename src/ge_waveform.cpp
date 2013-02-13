/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_waveform
 * an element which represents a waveform.
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


#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Menu_Button.H>
#include "ge_waveform.h"
#include "gd_editor.h"
#include "wave.h"
#include "glue.h"
#include "mixer.h"
#include "waveFx.h"
#include "ge_mixed.h"
#include "gg_waveTools.h"


extern Mixer G_Mixer;


gWaveform::gWaveform(int x, int y, int w, int h, int ch, const char *l)
: Fl_Widget(x, y, w, h, l),
	chan(ch),
	menuOpen(false),
	start(0),
	chanStart(0),
	chanStartLit(false),
	chanEnd(0),
	chanEndLit(false),
	selectionA(0),
	selectionB(0),
	selectionA_abs(0),
	selectionB_abs(0)
{
	data.sup  = NULL;
	data.inf  = NULL;
	data.size = 0;
	calcZoom();
	alloc();
}


/* ------------------------------------------------------------------ */


gWaveform::~gWaveform() {
	freeData();
}


/* ------------------------------------------------------------------ */


void gWaveform::freeData() {
	if (data.sup != NULL) {
		free(data.sup);
		free(data.inf);
		data.sup  = NULL;
		data.inf  = NULL;
		data.size = 0;
	}
}


/* ------------------------------------------------------------------ */


void gWaveform::alloc() {

	/* note: zoom must be always greater than 2: we don't want to draw
	 * the pair Left+Right, this is a representation of the left channel */

	freeData();

	data.size = (int) ceilf((float) G_Mixer.chan[chan]->size / (float) zoom);
	data.sup  = (int*) malloc(data.size * sizeof(int));
	data.inf  = (int*) malloc(data.size * sizeof(int));

	/* sampling from the original wave. Warning: zoom must always be a power
	 * of two (even number). */

	int offset = h() / 2;
	int zero   = y() + offset; // center, zero amplitude (-inf dB)

	for (int i=0, s=0; i<data.size; i++) {

		/* how to draw a scaled wave: for each chunk s+zoom compute the peak
		 * inside that range. That's the point we will draw on screen. */

		if (s+zoom > G_Mixer.chan[chan]->size) {
			data.sup[i] = zero;
		}
		else {
			float peaksup = 0.0f;
			float peakinf = 0.0f;
			for (int k=s; k<s+zoom; k+=2) {
				if (G_Mixer.chan[chan]->data[k] > peaksup)
					peaksup = G_Mixer.chan[chan]->data[k];    // Left data only
				else
				if (G_Mixer.chan[chan]->data[k] <= peakinf)
					peakinf = G_Mixer.chan[chan]->data[k];    // Left data only
			}
			data.sup[i] = zero - (peaksup * G_Mixer.chanBoost[chan] * offset);
			data.inf[i] = zero - (peakinf * G_Mixer.chanBoost[chan] * offset);

			/* avoid window overflow  */

			if (data.sup[i] < y())       data.sup[i] = y();
			if (data.inf[i] > y()+h()-1) data.inf[i] = y()+h()-1;
		}
		s += zoom;
	}
	recalcPoints();
}


/* ------------------------------------------------------------------ */


void gWaveform::recalcPoints() {

	selectionA = relativePoint(selectionA_abs);
	selectionB = relativePoint(selectionB_abs);

	chanStart  = relativePoint(G_Mixer.chanStartTrue[chan] / 2);

	/* fix the rounding error when chanEnd is set on the very end of the
	 * sample */

	if (G_Mixer.chanEndTrue[chan] == G_Mixer.chan[chan]->size)
		chanEnd = data.size;
	else
		chanEnd = relativePoint(G_Mixer.chanEndTrue[chan] / 2);
}


/* ------------------------------------------------------------------ */


void gWaveform::draw() {

	if (start < 0)
		start = 0;

	int offset = h() / 2;
	int zero   = y() + offset; // sample zero (-inf dB)

	/* blank canvas */

	fl_rectf(x(), y(), w(), h(), COLOR_BG_0);

	/* draw selection (if any) */

	if (selectionA != selectionB) {

		int a_x = selectionA - start;
		int b_x = selectionB - start;

		if (a_x < 0)
			a_x = 0;
		if (b_x >= w()-1)
			b_x = w()-1;

		if (selectionA < selectionB)
			fl_rectf(a_x+BORDER, y(), b_x-a_x, h(), COLOR_BD_0);
		else
			fl_rectf(b_x+BORDER, y(), a_x-b_x, h(), COLOR_BD_0);
	}

	/* draw waveform. start is the offset, driven by the scrollbar */

	fl_color(0, 0, 0);
	///for (int i=1; i<=w()-2; i++)
	///	if (i+start < data.size) {
	printf("waveform x()=%d\n", x());
	for (int i=0; i<data.size; i++) {
			fl_color(0, 0, 0);
			fl_line(i+x(), zero, i+x(), data.sup[i+start]);
			fl_line(i+x(), zero, i+x(), data.inf[i+start]);
		}

	/* border box */

	fl_rect(x(), y(), w(), h(), COLOR_BD_0);

	/* chanStart, if visible */

	if (chanStart >= start && chanStart-start < w()-2) {

		//printf("[waveform] print chanStart at %d\n", chanStart-start);

		int lineX = x()+chanStart-start+1;

		if (chanStartLit)
			fl_color(COLOR_BD_1);
		else
			fl_color(COLOR_BD_0);

		/* vertical line */
		fl_line(lineX, y()+1, lineX, y()+h()-2);

		/* print flag and avoid overflow */

		if (lineX+FLAG_WIDTH > w()+x()-2)
			fl_rectf(lineX, y()+h()-FLAG_HEIGHT-1, w()-lineX+x()-1, FLAG_HEIGHT);
		else  {
			fl_rectf(lineX, y()+h()-FLAG_HEIGHT-1, FLAG_WIDTH, FLAG_HEIGHT);
			fl_color(255, 255, 255);
			fl_draw("s", lineX+4, y()+h()-3);
		}
	}

	/* print chanEnd, if visible (as above) */

	if (chanEnd >= start && chanEnd-start <= w()-1) {

		//printf("[waveform] print chanEnd at %d\n", chanEnd-start);

		int lineX = x()+chanEnd-start;

		//printf("[waveform] print chanEnd at %d (abs=%d), start=%d\n", x()+i, chanEnd, start);
		if (chanEndLit)
			fl_color(COLOR_BD_1);
		else
			fl_color(COLOR_BD_0);

		fl_line(lineX, y()+1, lineX, y()+h()-2);

		if (lineX-FLAG_WIDTH < x())
			fl_rectf(x()+1, y()+1, lineX-x(), FLAG_HEIGHT);
		else {
			fl_rectf(lineX-FLAG_WIDTH, y()+1, FLAG_WIDTH, FLAG_HEIGHT);
			fl_color(255, 255, 255);
			fl_draw("e", lineX-10, y()+10);
		}
	}
}


/* ------------------------------------------------------------------ */


int gWaveform::handle(int e) {

	int ret = 0;

	switch (e) {

		case FL_PUSH: {

			mouseX = Fl::event_x();
			pushed = true;

			if (!mouseOnEnd() && !mouseOnStart()) {

				/* right button? show the menu. Don't set selectionA,B,etc */

				if (Fl::event_button3()) {
					openEditMenu();
				}
				else
				if (mouseOnSelectionA() || mouseOnSelectionB()) {
					resized = true;
				}
				else {
					dragged = true;
					selectionA = mouseX - BORDER + start;

					if (selectionA >= data.size) selectionA = data.size;

					selectionB = selectionA;
					selectionA_abs = absolutePoint(selectionA);
					selectionB_abs = selectionA_abs;
				}
			}

			ret = 1;
			break;
		}

		case FL_RELEASE: {

			/* don't recompute points if something is selected */

			if (selectionA != selectionB) {
				pushed  = false;
				dragged = false;
				ret = 1;
				break;
			}

			int realChanStart = G_Mixer.chanStartTrue[chan];
			int realChanEnd   = G_Mixer.chanEndTrue[chan];

			if (chanStartLit)
				realChanStart = absolutePoint(chanStart)*2;
			else
			if (chanEndLit)
				realChanEnd = absolutePoint(chanEnd)*2;

			glue_setBeginEndChannel((gdEditor *) window(), chan, realChanStart, realChanEnd, false);

			pushed  = false;
			dragged = false;

			redraw();
			ret = 1;
			break;
		}

		case FL_ENTER: {  // enables FL_DRAG
			ret = 1;
			break;
		}

		case FL_LEAVE: {
			if (chanStartLit || chanEndLit) {
				chanStartLit = false;
				chanEndLit   = false;
				redraw();
			}
			ret = 1;
			break;
		}

		case FL_MOVE: {
			mouseX = Fl::event_x();
			mouseY = Fl::event_y();

			if (mouseOnStart()) {
				chanStartLit = true;
				redraw();
			}
			else
			if (chanStartLit) {
				chanStartLit = false;
				redraw();
			}


			if (mouseOnEnd()) {
				chanEndLit = true;
				redraw();
			}
			else
			if (chanEndLit) {
				chanEndLit = false;
				redraw();
			}

			if (mouseOnSelectionA()) {
				fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
			}
			else
			if (mouseOnSelectionB()) {
				fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
			}
			else {
				fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
			}

			ret = 1;
			break;
		}

		case FL_DRAG: {

			if (chanStartLit && pushed)	{

				chanStart += Fl::event_x() - mouseX;

				if (chanStart < 0)
					chanStart = 0;

				if (chanStart >= chanEnd)
					chanStart = chanEnd-2;

				redraw();
			}
			else
			if (chanEndLit && pushed) {

				chanEnd += Fl::event_x() - mouseX;

				if (chanEnd > data.size)
					chanEnd = data.size;

				if (chanEnd <= chanStart)
					chanEnd = chanStart + 2;

				redraw();
			}

			/* here the mouse is on the waveform, i.e. a selection */

			else
			if (dragged) {

				selectionB = Fl::event_x() - BORDER + start;

				if (selectionB >= data.size)
					selectionB = data.size;

				if (selectionB <= 0)
					selectionB = 0;

				selectionB_abs = absolutePoint(selectionB);

				/*
				printf("selection: x1 = %d (abs = %d) x2 = %d (abs = %d)\n",
					selectionA,
					selectionA_abs,
					selectionB,
					selectionB_abs
				);
				*/
				redraw();
			}
			else
			if (resized) {
				if (mouseOnSelectionA()) {
					selectionA     = Fl::event_x() - BORDER + start;
					selectionA_abs = absolutePoint(selectionA);
				}
				else {
					selectionB     = Fl::event_x() - BORDER + start;
					selectionB_abs = absolutePoint(selectionB);
				}
				redraw();
			}
			mouseX = Fl::event_x();
			ret = 1;
			break;
		}
	}
	return ret;
}


/* ------------------------------------------------------------------ */


bool gWaveform::mouseOnStart() {
	return mouseX-10 >  chanStart - start              &&
				 mouseX-10 <= chanStart - start + FLAG_WIDTH &&
				 mouseY    >  h() + y() - FLAG_HEIGHT;
}


/* ------------------------------------------------------------------ */


bool gWaveform::mouseOnEnd() {
	return mouseX-10 >= chanEnd - start - FLAG_WIDTH &&
				 mouseX-10 <= chanEnd - start              &&
				 mouseY    <= y() + FLAG_HEIGHT + 1;
}


/* ------------------------------------------------------------------ */


bool gWaveform::mouseOnSelectionA() {
	if (selectionA == selectionB)
		return false;
	return mouseX-10 >= selectionA-5-start && mouseX-10 <= selectionA-start;
}


/* ------------------------------------------------------------------ */


bool gWaveform::mouseOnSelectionB() {
	if (selectionA == selectionB)
		return false;
	return mouseX-10 >= selectionB-5-start && mouseX-10 <= selectionB-start;
}


/* ------------------------------------------------------------------ */


int gWaveform::absolutePoint(int p) {

	if (p <= 0)
		return 0;

	if (p > data.size)
		return G_Mixer.chan[chan]->size / 2;

	return (p * (float) zoom) / 2;
}


/* ------------------------------------------------------------------ */


int gWaveform::relativePoint(int p) {
	return (ceilf(p / (float) zoom)) * 2;
}


/* ------------------------------------------------------------------ */


void gWaveform::openEditMenu() {

	if (selectionA == selectionB)
		return;

	menuOpen = true;

	Fl_Menu_Item menu[] = {
		{"Cut"},
		{"Trim"},
		{"Silence"},
		{"Set start/end here"},
		{0}
	};

	if (G_Mixer.chanStatus[chan] == STATUS_PLAY) {
		menu[0].deactivate();
		menu[1].deactivate();
	}

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(11);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, b);
	if (!m) {
		menuOpen = false;
		return;
	}

	/* straightSel() to ensure that point A is always lower than B */

	straightSel();

	if (strcmp(m->label(), "Silence") == 0) {
		wfx_silence(G_Mixer.chan[chan], absolutePoint(selectionA), absolutePoint(selectionB));

		selectionA = 0;
		selectionB = 0;

		alloc();
		redraw();
		menuOpen = false;
		return;
	}

	if (strcmp(m->label(), "Set start/end here") == 0) {

		glue_setBeginEndChannel(
				(gdEditor *) window(), // parent
				chan,
				absolutePoint(selectionA) * 2,  // stereo!
				absolutePoint(selectionB) * 2,  // stereo!
				false, // no recalc (we do it here)
				false  // don't check
				);

		selectionA     = 0;
		selectionB     = 0;
		selectionA_abs = 0;
		selectionB_abs = 0;

		recalcPoints();
		redraw();
		menuOpen = false;
		return;
	}

	if (strcmp(m->label(), "Cut") == 0) {
		wfx_cut(chan, absolutePoint(selectionA), absolutePoint(selectionB));

		/* for convenience reset start/end points */

		glue_setBeginEndChannel(
			(gdEditor *) window(),
			chan,
			0,
			G_Mixer.chan[chan]->size,
			false);

		selectionA     = 0;
		selectionB     = 0;
		selectionA_abs = 0;
		selectionB_abs = 0;
		start          = 0;

		alloc();
		redraw();
		//((gWaveTools*)parent())->updateScrollbar();
		menuOpen = false;
		return;
	}

	if (strcmp(m->label(), "Trim") == 0) {
		wfx_trim(chan, absolutePoint(selectionA), absolutePoint(selectionB));

		glue_setBeginEndChannel(
			(gdEditor *) window(),
			chan,
			0,
			G_Mixer.chan[chan]->size,
			false);

		selectionA     = 0;
		selectionB     = 0;
		selectionA_abs = 0;
		selectionB_abs = 0;
		start          = 0;

		calcZoom();
		alloc();
		redraw();
		//((gWaveTools*)parent())->updateScrollbar();
		menuOpen = false;
		return;
	}
}


/* ------------------------------------------------------------------ */


void gWaveform::straightSel() {
	if (selectionA > selectionB) {
		unsigned tmp = selectionB;
		selectionB = selectionA;
		selectionA = tmp;
	}
}


/* ------------------------------------------------------------------ */


void gWaveform::calcZoom() {
	zoom = (int) ceil((float)(G_Mixer.chan[chan]->size) / (float)(w()-2));
	if (zoom < 2)
		zoom = 2;
	if (zoom % 2 != 0)  // avoid odd values
		zoom++;
	initZoom = zoom;
}


/* ------------------------------------------------------------------ */


void gWaveform::setZoom(int type) {
	int newZoom;
	if (type == -1)			// zoom in
		newZoom = (int) ceil((float) zoom / 2.0f);
	else                // zoom out
		newZoom = zoom * 2;

	if (newZoom % 2 != 0)  // avoid odd values
		newZoom++;

	if (newZoom > 2) {
		zoom = newZoom;
		alloc();

		/* avoid waveform overflow on the rightmost side of the window */

		/*
		if (data.size > w()-2) {
			int overflow = start+w()-2 - data.size;
			if (overflow > 1) {
				//printf("[waveTools] overflow of %d pixel. Wave->start now = %d, after = %d\n", overflow, wave->start, wave->start - overflow);
				start -= overflow;
			}
		}
		*/
		size(data.size, h());
		redraw();
	}
}


/* ------------------------------------------------------------------ */


void gWaveform::resize(int x, int y, int w, int h) {
	Fl_Widget::resize(x, y, data.size, h);
	alloc();
	redraw();
}


/* ------------------------------------------------------------------ */


void gWaveform::scrollTo(int px) {
	start = px;
	redraw();
}


