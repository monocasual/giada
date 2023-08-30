/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/dialogs/mainWindow.h"
#include "core/const.h"
#include "glue/main.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/mainIO.h"
#include "gui/elems/mainWindow/mainMenu.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/elems/mainWindow/mainTransport.h"
#include "gui/elems/mainWindow/sequencer.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "warnings.h"
#include <FL/Fl.H>
#include <FL/Fl_Tooltip.H>
#include <fmt/core.h>

extern giada::v::Ui* g_ui;

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

void gdMainWindow::ScopedProgress::setProgress(float v)
{
	m_progress.setProgress(v);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gdMainWindow::gdMainWindow(geompp::Rect<int> r, const char* title)
: gdWindow(r, title)
{
	Fl::visible_focus(0);

	Fl::background(25, 25, 25); // TODO use G_COLOR_GREY_1

	Fl::set_boxtype(G_CUSTOM_BORDER_BOX, g_customBorderBox, 1, 1, 2, 2);
	Fl::set_boxtype(G_CUSTOM_UP_BOX, g_customUpBox, 1, 1, 2, 2);
	Fl::set_boxtype(G_CUSTOM_DOWN_BOX, g_customDownBox, 1, 1, 2, 2);

	Fl::set_boxtype(FL_BORDER_BOX, G_CUSTOM_BORDER_BOX);
	Fl::set_boxtype(FL_UP_BOX, G_CUSTOM_UP_BOX);
	Fl::set_boxtype(FL_DOWN_BOX, G_CUSTOM_DOWN_BOX);

	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		/* zone 1 - menus, and I/O tools */

		geFlex* zone1 = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
		{
			mainMenu = new v::geMainMenu();
			mainIO   = new v::geMainIO();
			zone1->add(mainMenu, 350);
			zone1->add(new geBox(), 80);
			zone1->add(mainIO);
			zone1->end();
		}

		/* zone 2 - mainTransport and timing tools */

		geFlex* zone2 = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN, {3, 0, 0, 0});
		{
			mainTransport = new v::geMainTransport();

			geFlex* zoneTimer = new geFlex(Direction::VERTICAL, G_GUI_INNER_MARGIN, {2, 0, 3, 0});
			{
				mainTimer = new v::geMainTimer();
				zoneTimer->add(mainTimer);
				zoneTimer->end();
			}

			zone2->add(mainTransport, 400);
			zone2->add(new geBox());
			zone2->add(zoneTimer, 237);
			zone2->end();
		}

		/* zone 3 - sequencer */

		geFlex* zone3 = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN, {0, 5});
		{
			sequencer = new v::geSequencer();
			zone3->add(new geBox(), 80);
			zone3->add(sequencer);
			zone3->add(new geBox(), 80);
			zone3->end();
		}

		keyboard = new v::geKeyboard();

		container->add(zone1, G_GUI_UNIT);
		container->add(zone2, 28);
		container->add(zone3, 40);
		container->add(keyboard);
		container->end();
	}

	add(container);
	resizable(container);

	callback([](Fl_Widget* /*w*/, void* /*v*/) {
		if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape)
			return; // ignore Escape
		c::main::quitGiada();
	});

	size_range(G_MIN_GUI_WIDTH, G_MIN_GUI_HEIGHT);
	refresh();
	show();
}

/* -------------------------------------------------------------------------- */

gdMainWindow::~gdMainWindow()
{
	g_ui->model.mainWindowBounds = getBounds();
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

void gdMainWindow::setTitle(const std::string& title)
{
	copy_label(fmt::format("{} - {}", G_APP_NAME, title).c_str());
}

/* -------------------------------------------------------------------------- */

gdMainWindow::ScopedProgress gdMainWindow::getScopedProgress(const char* msg)
{
	return {m_progress, msg};
}
} // namespace giada::v