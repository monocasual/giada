/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_waveScroll
 * Scrollbar per muoversi orizzontalmente all'interno di un wave
 * rappresentato da gWaveform.
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2011 Giovanni A. Zuliani | Monocasual
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

#ifndef GE_WAVESCROLL_H
#define GE_WAVESCROLL_H

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/fl_draw.H>
#include "const.h"


class gWavescroll : public Fl_Scrollbar {
public:
	gWavescroll(
		int x,
		int y,
		int w,
		int h,
		int initZoom,
		int zoom,
		unsigned maxSize,
		const char *l=0
	);

	void draw();
	int  handle(int e);

	/* getPos
	 * returns 'pos' without x() offset + borders */

	int  getPos();

	/* getRange
	 * returns the maximum value available */

	int  getRange();

	int initWidth;
	int initZoom;
	int zoom;

	/* width, pos
	 *
	 *    a    b
	 * |--[    ]-----|
	 *
	 * a   = pos
	 * b-a = width */
	int width, pos;

	int  mouseX;
	bool pushed;
};

#endif
