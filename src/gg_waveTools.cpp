/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gg_waveTools
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
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

#include "gg_waveTools.h"

extern Mixer G_Mixer;


gWaveTools::gWaveTools(int X, int Y, int W, int H, int chan, const char *L)
: Fl_Group(X,Y,W,H,L) {

	wave = new gWaveform(8, 8, 484, 154, chan);
	scrollLeft  = new gClickRepeat(8,  170, 14, 14, "", scrollLeftOff_xpm, scrollLeftOn_xpm);
	scrollRight = new gClickRepeat(442, 170, 14, 14, "", scrollRightOff_xpm, scrollRightOn_xpm);
	zoomPlus    = new gClick(460, 170, 14, 14, "", zoomInOff_xpm, zoomInOn_xpm);
	zoomMins    = new gClick(478, 170, 14, 14, "", zoomOutOff_xpm, zoomOutOn_xpm);
	scrollBar   = new gWavescroll(26, 170, 412, 14,
		wave->initZoom,
		wave->zoom,
		G_Mixer.chan[chan]->size
	);
	end();

	scrollBar->callback(cb_scroll,      (void*)this);
	scrollLeft->callback(cb_moveLeft,   (void*)this);
	scrollRight->callback(cb_moveRight, (void*)this);
	zoomPlus->callback(cb_zoomPlus,     (void*)this);
	zoomMins->callback(cb_zoomMins,     (void*)this);
}


void gWaveTools::cb_zoomPlus(Fl_Widget *v, void *p)  { ((gWaveTools*)p)->__cb_zoomPlus(v); }
void gWaveTools::cb_zoomMins(Fl_Widget *v, void *p)  { ((gWaveTools*)p)->__cb_zoomMins(v); }
void gWaveTools::cb_scroll(Fl_Widget *v, void *p)    { ((gWaveTools*)p)->__cb_scroll();	}
void gWaveTools::cb_moveRight(Fl_Widget *v, void *p) { ((gWaveTools*)p)->__cb_moveRight(); }
void gWaveTools::cb_moveLeft(Fl_Widget *v, void *p)  { ((gWaveTools*)p)->__cb_moveLeft();	}


void gWaveTools::__cb_zoomPlus(Fl_Widget *v) {
	zoom(1);
}


void gWaveTools::__cb_zoomMins(Fl_Widget *v) {
	zoom(0);
}


void gWaveTools::__cb_scroll() {

	/* never scroll if the zoom == 100% */

	if (wave->zoom == wave->initZoom)
		return;

	wave->start = getScrollPosition();
	wave->redraw();
}


void gWaveTools::__cb_moveLeft() {
	move(0);
}


void gWaveTools::__cb_moveRight() {
	move(1);
}


int gWaveTools::getScrollPosition() {

	/* how many pixels the window hides */

	int totalHidden = wave->dataSize - wave->w()+2;

	/* if scrollRatio == 1: one pixel scroll corresponds to a shift of
	 * one pixel of the waveform */

	float scrollRatio = totalHidden / float(scrollBar->getRange());

	return (int) scrollBar->getPos() * scrollRatio;
}


int gWaveTools::setScrollPosition() {
	int totalHidden   = wave->dataSize - wave->w()+2;
	float scrollRatio = totalHidden / float(scrollBar->getRange());
	return (wave->start / scrollRatio) + scrollBar->x() + 1; // 1px left border
}


void gWaveTools::zoom(int type) {

	unsigned newZoom;
	if (type == 1)			// zoom plus
		newZoom = (int)ceil((float)wave->zoom / 2.0f);
	else                // zoom mins
		newZoom = wave->zoom * 2;

	/* avoid odd zoom values, it would be a mess */

	if (newZoom % 2 != 0)
		newZoom += 1;

	if (wave->setZoom(newZoom))
		wave->alloc();

	/* make zoom relative to scrollbar */

	wave->start = getScrollPosition();

	/* avoid waveform overflow on the rightmost side of the window */

	if (wave->dataSize > wave->w()-2) {
		int overflow = wave->start+wave->w()-2 - wave->dataSize;
		if (overflow > 1) {
			//printf("[waveTools] overflow of %d pixel. Wave->start now = %d, after = %d\n", overflow, wave->start, wave->start - overflow);
			wave->start -= overflow;
		}
	}

	wave->redraw();
}


void gWaveTools::move(int dir) {

	if (wave->zoom == wave->initZoom)
		return;

	if (dir == 1)  // scroll >
		wave->start += 8;
	else           // scroll <
		wave->start -= 8;

	if (wave->start < 0)
		wave->start = 0;
	else if (wave->start + wave->w() > wave->dataSize)
		wave->start = wave->dataSize - wave->w() + 2;

	scrollBar->pos = setScrollPosition();

	wave->redraw();
	scrollBar->redraw();
}
