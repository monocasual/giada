/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "warnings.h"
#include "core/const.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "utils/gui.h"
#include "window.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>

namespace giada::v
{
namespace
{
bool confirmRet_ = false;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gdAlert(const char* c)
{
	gdWindow* modal = new gdWindow(
	    (Fl::w() / 2) - 150,
	    (Fl::h() / 2) - 47,
	    300, 90, "Alert");
	modal->set_modal();
	modal->begin();
	geBox*    box = new geBox(10, 10, 280, 40, c);
	geButton* b   = new geButton(210, 60, 80, 20, "Close");
	modal->end();
	box->labelsize(G_GUI_FONT_SIZE_BASE);
	b->callback(cb_window_closer, (void*)modal);
	b->shortcut(FL_Enter);
	u::gui::setFavicon(modal);
	modal->show();
}

/* -------------------------------------------------------------------------- */

int gdConfirmWin(const char* title, const char* msg)
{
	gdWindow win(u::gui::centerWindowX(300), u::gui::centerWindowY(90), 300, 90, title);
	win.set_modal();
	win.begin();
	new geBox(10, 10, 280, 40, msg);
	geButton* ok = new geButton(212, 62, 80, 20, "Ok");
	geButton* ko = new geButton(124, 62, 80, 20, "Cancel");
	win.end();

	ok->shortcut(FL_Enter);
	ok->onClick = [&win]() { confirmRet_ = true; win.hide(); };

	ko->onClick = [&win]() { confirmRet_ = false; win.hide(); };

	u::gui::setFavicon(&win);
	win.show();

	while (win.shown())
		Fl::wait();

	return confirmRet_;
}
} // namespace giada::v