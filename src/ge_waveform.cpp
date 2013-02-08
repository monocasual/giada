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


#include "ge_waveform.h"
#include "gd_editor.h"
#include "wave.h"
#include "glue.h"
#include "mixer.h"


extern Mixer G_Mixer;


gWaveform::gWaveform(int x, int y, int w, int h, int ch, const char *l)
: Fl_Widget(x, y, w, h, l),
	chan(ch),
	menuOpen(false),
	start(0),
	data(NULL),
	dataSize(0),
	chanStart(0),
	chanStartLit(false),
	chanEnd(0),
	chanEndLit(false),
	selectionA(0),
	selectionB(0),
	selectionA_abs(0),
	selectionB_abs(0)
{
	calcZoom();
	alloc();
}


/* ------------------------------------------------------------------ */


gWaveform::~gWaveform() {
	if (data != NULL) {
		free(data);
		dataSize = 0;
		//printf("[waveform] data free'd\n");
	}
}


/* ------------------------------------------------------------------ */

/*
bool gWaveform::setZoom(unsigned z) {

	bool ret = true;
	zoom = z;

	if (zoom < 2) {
		zoom = 2;
		ret  = false;
	}
	else if (zoom > initZoom) {
		zoom = initZoom;
		ret = true; // realloc, for safety
	}
	//printf("[waveform] set zoom to %d\n", zoom);
	return ret;
}
*/


/* ------------------------------------------------------------------ */


void gWaveform::alloc() {

	/* note: zoom must be always greater than 2: we don't want to draw
	 * the pair Left+Right, this is a representation of the left channel */

	/* full wave zoom-affected in memory. Later, we draw a part of it */

	if (data != NULL) {
		free(data);
		data  = NULL;
		dataSize = 0;
		//printf("[waveform] data free'd\n");
	}

	dataSize  = (int) ceilf((float)(G_Mixer.chan[chan]->size) / (float)zoom);
	data   = (int*) malloc(dataSize*sizeof(int));

	printf(
		"[waveform] alloc %d cells of data (orig.size=%d, zoom=%d)\n",
		dataSize,
		G_Mixer.chan[chan]->size,
		zoom
	);

	/* sampling from the original wave */

	int zero = h()+y()-2; // zero sample (-inf dB)
	int s = 0;
	int i = 0;
	int corrector = zoom % 2 != 0 ? 1 : 0;

	while (i<dataSize) {

		/* how to draw a scaled wave: for each chunk s+zoom compute the peak
		 * inside that range. That's the point we will draw on screen. Always
		 * pick up Left samples: corrector helps to do it right. */

		if (s+zoom+corrector > G_Mixer.chan[chan]->size) {
			data[i] = zero;
		}
		else {

			/* computing values > 0 in data */

			float peak = 0.0f;
			for (int k=s; k<s+zoom+corrector; k+=2) {  // k+=2: LRLRLRLRLRLRLR... only L channels
				if (G_Mixer.chan[chan]->data[k] > peak)
					peak = G_Mixer.chan[chan]->data[k];
			}
			data[i] = zero - (peak * G_Mixer.chanBoost[chan] * h());

			//printf("data[%d]=%d | range [%d-%d) (zoom=%d)\n", i, data[i], s, s+zoom+corrector, zoom);

			/* avoid window overflow */

			if (data[i] < y())
				data[i] = y();
		}
		i += 1;
		s += zoom + corrector;
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
		chanEnd = dataSize;
	else
		chanEnd = relativePoint(G_Mixer.chanEndTrue[chan] / 2);
}


/* ------------------------------------------------------------------ */


void gWaveform::draw() {

	if (start < 0)
		start = 0;

	int zero = h()+y()-2; // zero sample (-inf dB)

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

		//printf("[waveform] drawing selectionA = %d, selectionB = %d\n", a_x, b_x);

		if (selectionA < selectionB)
			fl_rectf(a_x+BORDER, y(), b_x-a_x, h(), COLOR_BD_0);
		else
			fl_rectf(b_x+BORDER, y(), a_x-b_x, h(), COLOR_BD_0);
	}

	/* draw waveform. S is the offset, driven by the scrollbar */

	fl_color(0, 0, 0);
	//printf("[waveform] drawing from %d to %d\n", start, w()-2);
	for (int i=1; i<=w()-2; i++) {
		if (i+start < dataSize) {
			fl_color(0, 0, 0);
			fl_line(i+x(), zero, i+x(), data[i+start]);
		}
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

		case FL_MOUSEWHEEL: {
			setZoom(Fl::event_dy());
			ret = 1;
			break;
		}

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

					if (selectionA >= dataSize)	selectionA = dataSize;

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

				if (chanEnd > dataSize)
					chanEnd = dataSize;

				if (chanEnd <= chanStart)
					chanEnd = chanStart + 2;

				redraw();
			}

			/* here the mouse is on the waveform, i.e. a selection */

			else
			if (dragged) {

				selectionB = Fl::event_x() - BORDER + start;

				if (selectionB >= dataSize)
					selectionB = dataSize;

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

	if (p > dataSize)
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

		calcZoom();
		alloc();
		redraw();
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
	initZoom = zoom;
	//printf("[waveform] init zoom = %d (chansize=%d, windowsize=%d\n", zoom, G_Mixer.chan[chan]->size, w-2);
}


/* ------------------------------------------------------------------ */


void gWaveform::setZoom(int type) {
	if (type == -1)			// zoom in
		zoom = (int) ceil((float) zoom / 2.0f);
	else                // zoom out
		zoom = zoom * 2;

	if (zoom % 2 != 0)  // avoid odd values
		zoom++;

	alloc();

	/* avoid waveform overflow on the rightmost side of the window */

	if (dataSize > w()-2) {
		int overflow = start+w()-2 - dataSize;
		if (overflow > 1) {
			//printf("[waveTools] overflow of %d pixel. Wave->start now = %d, after = %d\n", overflow, wave->start, wave->start - overflow);
			start -= overflow;
		}
	}
	redraw();
}
