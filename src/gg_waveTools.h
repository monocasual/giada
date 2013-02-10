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


#ifndef GG_WAVETOOLS_H
#define GG_WAVETOOLS_H

#include <FL/Fl.H>
#include <FL/Fl_Group.H>


class gWaveTools : public Fl_Group {
private:
	static void cb_scroll(Fl_Widget *w, void *p);
	inline void __cb_scroll();

public:
	class gScrollbar *scrollbar;
	class gWaveform  *waveform;

	gWaveTools(int X,int Y,int W, int H, int chan, const char *L=0);

	int  handle(int e);

	void updateWaveform();
	void updateScrollbar();
};

#endif
