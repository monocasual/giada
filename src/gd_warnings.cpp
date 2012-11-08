/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_warnings
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


#include "gd_warnings.h"


void gdAlert(const char *c) {
	Fl_Window *modal = new Fl_Window(
			(Fl::w() / 2) - 150,
			(Fl::h() / 2) - 47,
			300, 90, "Alert");
	modal->set_modal();
	modal->begin();
		gBox *box = new gBox(10, 10, 280, 40, c);
		gClick *b = new gClick(210, 60, 80, 20, "Close");
	modal->end();
	box->labelsize(11);
	b->callback(__cb_window_closer, (void *)modal);
	b->shortcut(FL_Enter);
	gu_setFavicon(modal);
	modal->show();
}


int gdConfirmWin(const char *title, const char *msg) {
	Fl_Window *win = new Fl_Window(
			(Fl::w() / 2) - 150,
			(Fl::h() / 2) - 47,
			300, 90, title);
	win->set_modal();
	win->begin();
		new gBox(10, 10, 280, 40, msg);
		gClick *ok = new gClick(212, 62, 80, 20, "Ok");
		gClick *ko = new gClick(124, 62, 80, 20, "Cancel");
	win->end();
	ok->shortcut(FL_Enter);
	gu_setFavicon(win);
	win->show();

	/* no callbacks here. readqueue() check the event stack. */

	int r = 0;
	while (true) {
		Fl_Widget *o = Fl::readqueue();
		if (!o) Fl::wait();
		else if (o == ok) {r = 1; break;}
		else if (o == ko) {r = 0; break;}
	}
	//delete win;
	win->hide();
	return r;
}
