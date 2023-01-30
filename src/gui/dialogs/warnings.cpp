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
#include "gui/elems/basics/textButton.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "window.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>

extern giada::v::Ui g_ui;

namespace giada::v
{
namespace
{
bool confirmRet_ = false;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void gdAlert(const char* msg)
{
	gdWindow win(u::gui::getCenterWinBounds({-1, -1, 300, 90}), g_ui.getI18Text(LangMap::COMMON_WARNING));
	win.set_modal();
	win.begin();
	geBox*        box = new geBox(10, 10, 280, 40, msg);
	geTextButton* b   = new geTextButton(210, 60, 80, 20, g_ui.getI18Text(LangMap::COMMON_CLOSE));
	win.end();
	box->labelsize(G_GUI_FONT_SIZE_BASE);

	b->shortcut(FL_Enter);
	b->onClick = [&win]() { win.hide(); };

	win.show();

	while (win.shown())
		Fl::wait();
}

/* -------------------------------------------------------------------------- */

int gdConfirmWin(const char* title, const char* msg)
{
	gdWindow win(u::gui::getCenterWinBounds({-1, -1, 300, 90}), title);
	win.set_modal();
	win.begin();
	new geBox(10, 10, 280, 40, msg);
	geTextButton* ok = new geTextButton(212, 62, 80, 20, g_ui.getI18Text(LangMap::COMMON_OK));
	geTextButton* ko = new geTextButton(124, 62, 80, 20, g_ui.getI18Text(LangMap::COMMON_CANCEL));
	win.end();

	ok->shortcut(FL_Enter);
	ok->onClick = [&win]() { confirmRet_ = true; win.hide(); };

	ko->onClick = [&win]() { confirmRet_ = false; win.hide(); };

	win.show();

	while (win.shown())
		Fl::wait();

	return confirmRet_;
}
} // namespace giada::v