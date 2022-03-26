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

#include "mainWindow.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/init.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/mainIO.h"
#include "gui/elems/mainWindow/mainMenu.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/elems/mainWindow/mainTransport.h"
#include "gui/elems/mainWindow/sequencer.h"
#include "utils/gui.h"
#include "warnings.h"
#include <FL/Fl.H>
#include <FL/Fl_Tooltip.H>

namespace giada::v
{
gdMainWindow::ScopedProgress::ScopedProgress(gdProgress& p, const char* msg)
: m_progress(p)
{
	m_progress.popup(msg);
}

/* -------------------------------------------------------------------------- */

gdMainWindow::ScopedProgress::~ScopedProgress()
{
	m_progress.hide();
}

/* -------------------------------------------------------------------------- */

gdProgress& gdMainWindow::ScopedProgress::get()
{
	return m_progress;
}
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gdMainWindow::gdMainWindow(int W, int H, const char* title, int argc, char** argv, m::Conf::Data& c)
: gdWindow(W, H, title)
, m_conf(c)
{
	Fl::visible_focus(0);

	Fl::background(25, 25, 25); // TODO use G_COLOR_GREY_1

	Fl::set_boxtype(G_CUSTOM_BORDER_BOX, g_customBorderBox, 1, 1, 2, 2);
	Fl::set_boxtype(G_CUSTOM_UP_BOX, g_customUpBox, 1, 1, 2, 2);
	Fl::set_boxtype(G_CUSTOM_DOWN_BOX, g_customDownBox, 1, 1, 2, 2);

	Fl::set_boxtype(FL_BORDER_BOX, G_CUSTOM_BORDER_BOX);
	Fl::set_boxtype(FL_UP_BOX, G_CUSTOM_UP_BOX);
	Fl::set_boxtype(FL_DOWN_BOX, G_CUSTOM_DOWN_BOX);

	Fl_Tooltip::color(G_COLOR_GREY_1);
	Fl_Tooltip::textcolor(G_COLOR_LIGHT_2);
	Fl_Tooltip::size(G_GUI_FONT_SIZE_BASE);
	Fl_Tooltip::enable(m_conf.showTooltips);

	size_range(G_MIN_GUI_WIDTH, G_MIN_GUI_HEIGHT);

	mainMenu      = new v::geMainMenu(0, 0);
	mainIO        = new v::geMainIO(0, 0, 0, 0);
	mainTransport = new v::geMainTransport(8, 39);
	mainTimer     = new v::geMainTimer(571, 44);
	sequencer     = new v::geSequencer(100, 78, 609, 30);
	keyboard      = new v::geKeyboard(8, 122, w() - 16, 380);

	/* zone 1 - menus, and I/O tools */

	geFlex* zone1 = new geFlex(getContentBounds().reduced(G_GUI_OUTER_MARGIN).withH(G_GUI_UNIT),
	    Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
	zone1->add(mainMenu, 300);
	zone1->add(new Fl_Box(0, 0, 0, 0));
	zone1->add(mainIO, 430);
	zone1->end();

	/* zone 2 - mainTransport and timing tools */

	Fl_Group* zone2 = new Fl_Group(8, mainTransport->y(), W - 16, mainTransport->h());
	zone2->add(mainTransport);
	zone2->resizable(new Fl_Box(mainTransport->x() + mainTransport->w() + 4, zone2->y(), 80, 20));
	zone2->add(mainTimer);

	/* zone 3 - beat meter */

	Fl_Group* zone3 = new Fl_Group(8, sequencer->y(), W - 16, sequencer->h());
	zone3->add(sequencer);

	/* zone 4 - the keyboard (Fl_Group is unnecessary here, keyboard is
	 * a group by itself) */

	resizable(keyboard);

	add(zone1);
	add(zone2);
	add(zone3);
	add(keyboard);

	callback([](Fl_Widget* /*w*/, void* /*v*/) {
		if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape)
			return; // ignore Escape
		m::init::closeMainWindow();
	});
	u::gui::setFavicon(this);

	refresh();

	show(argc, argv);
}

/* -------------------------------------------------------------------------- */

gdMainWindow::~gdMainWindow()
{
	m_conf.mainWindowX = x();
	m_conf.mainWindowY = y();
	m_conf.mainWindowW = w();
	m_conf.mainWindowH = h();
}

/* -------------------------------------------------------------------------- */

void gdMainWindow::refresh()
{
	mainIO->refresh();
	mainTimer->refresh();
	mainTransport->refresh();
	sequencer->refresh();
	keyboard->refresh();
}

/* -------------------------------------------------------------------------- */

void gdMainWindow::rebuild()
{
	keyboard->rebuild();
	mainIO->rebuild();
	mainTimer->rebuild();
}

/* -------------------------------------------------------------------------- */

void gdMainWindow::clearKeyboard()
{
	keyboard->init();
}

/* -------------------------------------------------------------------------- */

gdMainWindow::ScopedProgress gdMainWindow::getScopedProgress(const char* msg)
{
	return {m_progress, msg};
}

} // namespace giada::v