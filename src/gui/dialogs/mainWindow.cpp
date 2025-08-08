/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/dialogs/mainWindow.h"
#include "src/glue/main.h"
#include "src/gui/dialogs/warnings.h"
#include "src/gui/elems/basics/boxtypes.h"
#include "src/gui/elems/basics/flex.h"
#include "src/gui/elems/mainWindow/keyboard/keyboard.h"
#include "src/gui/elems/mainWindow/mainInput.h"
#include "src/gui/elems/mainWindow/mainMenu.h"
#include "src/gui/elems/mainWindow/mainOutput.h"
#include "src/gui/elems/mainWindow/mainTimer.h"
#include "src/gui/elems/mainWindow/mainTransport.h"
#include "src/gui/elems/mainWindow/sequencer.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include <FL/Fl.H>
#include <FL/Fl_Tooltip.H>
#include <fmt/core.h>

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdMainWindow::ScopedProgress::ScopedProgress(gdProgress& p, const char* msg, std::function<void()> onCancel)
: m_progress(p)
{
	m_progress.popup(msg, onCancel != nullptr);
	m_progress.onCancel = onCancel;
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

	geFlex* container = new geFlex(getContentBounds(), Direction::VERTICAL);
	{
		geFlex* header = new geFlex(getContentBounds(), Direction::VERTICAL);
		{
			geMainMenu* mainMenu = new geMainMenu();

			header->addWidget(mainMenu);
			header->end();
		}

		geFlex* body = new geFlex(getContentBounds(), Direction::VERTICAL, G_GUI_OUTER_MARGIN, {G_GUI_OUTER_MARGIN});
		{
			/* zone 2 - mainTransport and timing tools */

			geFlex* zone2 = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
			{
				mainTransport = new v::geMainTransport();
				cpuLoad       = new geCpuLoad();

				geFlex* zoneTimer = new geFlex(Direction::VERTICAL, G_GUI_INNER_MARGIN, {2, 0, 3, 0});
				{
					mainTimer = new v::geMainTimer();
					zoneTimer->addWidget(mainTimer);
					zoneTimer->end();
				}

				zone2->addWidget(mainTransport, 400);
				zone2->addWidget(new geBox());
				zone2->addWidget(cpuLoad, 100);
				zone2->addWidget(new geBox(), G_GUI_OUTER_MARGIN);
				zone2->addWidget(zoneTimer, 237);
				zone2->end();
			}

			/* zone 3 - sequencer */

			geFlex* zone3 = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN, {0, 15});
			{
				sequencer = new v::geSequencer();
				zone3->addWidget(new geBox(), 80);
				zone3->addWidget(sequencer);
				zone3->addWidget(new geBox(), 80);
				zone3->end();
			}

			/* zone 4 - input/output and channels */

			geFlex* zone4 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
			{
				mainInput  = new geMainInput();
				keyboard   = new v::geKeyboard();
				mainOutput = new geMainOutput();
				zone4->addWidget(mainInput, G_GUI_UNIT);
				zone4->addWidget(keyboard);
				zone4->addWidget(mainOutput, G_GUI_UNIT);
				zone4->end();
			}

			body->addWidget(zone2, 25);
			body->addWidget(zone3, 70);
			body->addWidget(zone4);
			body->end();
		}

#if !G_OS_MAC // No need on macOS
		container->addWidget(header, 25);
#endif
		container->addWidget(body);
		container->end();
	}

	add(container);
	resizable(container);

	callback([](Fl_Widget* /*w*/, void* /*v*/)
	{
		if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape)
			return; // ignore Escape
		c::main::quitGiada(); });

	size_range(G_MIN_GUI_WIDTH, G_MIN_GUI_HEIGHT);
	refresh();
	show();
}

/* -------------------------------------------------------------------------- */

void gdMainWindow::refresh()
{
	mainTimer->refresh();
	cpuLoad->refresh();
	mainTransport->refresh();
	sequencer->refresh();
	keyboard->refresh();
	mainInput->refresh();
	mainOutput->refresh();
}

/* -------------------------------------------------------------------------- */

void gdMainWindow::rebuild()
{
	keyboard->rebuild();
	mainTimer->rebuild();
	mainInput->rebuild();
	mainOutput->rebuild();
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

gdMainWindow::ScopedProgress gdMainWindow::getScopedProgress(const char* msg, std::function<void()> onCancel)
{
	return {m_progress, msg, onCancel};
}

/* -------------------------------------------------------------------------- */

void gdMainWindow::resize(int x, int y, int w, int h)
{
	gdWindow::resize(x, y, w, h);
	g_ui->model.mainWindowBounds = getBounds();
}
} // namespace giada::v