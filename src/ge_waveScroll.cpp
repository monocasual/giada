/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_waveScroll
 *
 * a custom scrollbar.
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

#include "ge_waveScroll.h"


gWavescroll::gWavescroll(
	int x, int y,
	int w, int h,
	int initZoom,
	int zoom,
	unsigned maxSize,
	const char *l
)
: Fl_Scrollbar(x, y, w, h, l),
	initZoom(initZoom),
	zoom(zoom),
	pos(1+x),			// 1 pixel for the border
	mouseX(0),
	pushed(false)
{
	//updateWidth();
	width = 14;
}


void gWavescroll::draw() {

	/* clear everything */

	fl_rectf(x(), y(), w(), h(), COLOR_BG_0);

	/* container + cursor */

	fl_color(COLOR_BD_0);
	fl_rect(x(), y(), w(), h());
	fl_rectf(pos, y(), width, h());
}


int gWavescroll::handle(int e) {

	int ret = 0;

	switch (e) {
		case FL_PUSH: {
			mouseX = Fl::event_x();
			pushed = true;
			ret = 1;
			break;
		}
		case FL_RELEASE: {
			pushed = false;
			ret = 1;
			break;
		}
		case FL_DRAG: {
			if (
				(Fl::event_x() > pos &&
				 Fl::event_x() < pos + width + x()) ||
				pushed)
			{

				/* position consistent with the mouse */

				pos += Fl::event_x() - mouseX;

				if (pos + width > w() + x() - 1) // mind the border
					pos = w() + x() - width - 1;
				else if (pos <= x()+1)				   // mind the border
					pos = x()+1;

				do_callback();

				/* redraw to show the movement */

				redraw();

				mouseX = Fl::event_x();
			}
			ret = 1;
			break;
		}
	}
	return ret;
}


int gWavescroll::getPos() {
	if (pos-x()-1 + width < w())
		return pos-x()-1;
	else
		return pos-x();
}


int gWavescroll::getRange() {
	return w() - width - 2;
}
