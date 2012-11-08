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

#ifndef GG_WAVETOOLS_H
#define GG_WAVETOOLS_H

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include "ge_waveform.h"
#include "ge_waveScroll.h"
#include "mixer.h"

class gWaveTools : public Fl_Group {
private:
	static void cb_zoomPlus(Fl_Widget *w, void *p);
	static void cb_zoomMins(Fl_Widget *w, void *p);
	static void cb_scroll(Fl_Widget *w, void *p);
	static void cb_moveLeft(Fl_Widget *w, void *p);
	static void cb_moveRight(Fl_Widget *w, void *p);
	inline void __cb_zoomPlus(Fl_Widget *v);
	inline void __cb_zoomMins(Fl_Widget *v);
	inline void __cb_scroll();
	inline void __cb_moveLeft();
	inline void __cb_moveRight();

	/* getScrollPosition
	 * return the scrollbar value according to the length of the wave */
	int getScrollPosition();

	void zoom(int type);

	void move(int dir);

	/* setScrollPosition
	 * update the scrollbar position when clicking on < and > buttons */
	int setScrollPosition();

	class gWavescroll  *scrollBar;
	class gClickRepeat *scrollLeft;
	class gClickRepeat *scrollRight;
	class gClick		   *zoomPlus;
	class gClick		   *zoomMins;

	int j; // for scrolling (see .cpp)

public:
	gWaveTools(int X,int Y,int W,int H, int chan, const char *L=0);

	class gWaveform   *wave;
};

#endif
