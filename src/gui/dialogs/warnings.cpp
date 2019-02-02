/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
 * -------------------------------------------------------------------------- */


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include "../../utils/gui.h"
#include "../../core/const.h"
#include "../elems/basics/button.h"
#include "../elems/basics/box.h"
#include "window.h"
#include "warnings.h"


using namespace giada;


void gdAlert(const char *c)
{
	Fl_Window *modal = new Fl_Window(
			(Fl::w() / 2) - 150,
			(Fl::h() / 2) - 47,
			300, 90, "Alert");
	modal->set_modal();
	modal->begin();
		geBox *box = new geBox(10, 10, 280, 40, c);
		geButton *b = new geButton(210, 60, 80, 20, "Close");
	modal->end();
	box->labelsize(G_GUI_FONT_SIZE_BASE);
	b->callback(__cb_window_closer, (void *)modal);
	b->shortcut(FL_Enter);
	u::gui::setFavicon(modal);
	modal->show();
}


int gdConfirmWin(const char *title, const char *msg)
{
	Fl_Window *win = new Fl_Window(
			(Fl::w() / 2) - 150,
			(Fl::h() / 2) - 47,
			300, 90, title);
	win->set_modal();
	win->begin();
		new geBox(10, 10, 280, 40, msg);
		geButton *ok = new geButton(212, 62, 80, 20, "Ok");
		geButton *ko = new geButton(124, 62, 80, 20, "Cancel");
	win->end();
	ok->shortcut(FL_Enter);
	u::gui::setFavicon(win);
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
