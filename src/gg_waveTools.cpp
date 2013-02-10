/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gg_waveTools
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


#include "gg_waveTools.h"
#include "graphics.h"
#include "ge_mixed.h"
#include "ge_waveform.h"
#include "mixer.h"


gWaveTools::gWaveTools(int x, int y, int w, int h, int chan, const char *l)
	: Fl_Group(x, y, w, h, l)
{

	waveform  = new gWaveform(x, y, w, h-28, chan);
	scrollbar = new gScrollbar(x, y+h-20, w, 20);
	scrollbar->slider_size(1.0f);
	scrollbar->bounds(0, waveform->getSize());
	scrollbar->value(0);
	scrollbar->callback(cb_scroll, (void*)this);

	resizable(waveform);
}


/* ------------------------------------------------------------------ */


void gWaveTools::updateWaveform() {
	waveform->alloc();
	waveform->redraw();
}


/* ------------------------------------------------------------------ */


void gWaveTools::updateScrollbar() {
	scrollbar->slider_size(waveform->displayRatio());
	scrollbar->bounds(0, waveform->getSize()-w()+2);
}


/* ------------------------------------------------------------------ */


int gWaveTools::handle(int e) {
	int ret = Fl_Group::handle(e);
	switch (e) {
		case FL_MOUSEWHEEL: {
			waveform->setZoom(Fl::event_dy());
			updateScrollbar();
			ret = 1;
			break;
		}
	}
	return ret;
}


/* ------------------------------------------------------------------ */


void gWaveTools::cb_scroll(Fl_Widget *v, void *p)  { ((gWaveTools*)p)->__cb_scroll(); }


/* ------------------------------------------------------------------ */


void gWaveTools::__cb_scroll() {
	waveform->scrollTo(scrollbar->value());
}
