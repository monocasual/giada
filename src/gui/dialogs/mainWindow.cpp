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
#include "../../core/const.h"
#include "../../core/conf.h"
#include "../../core/init.h"
#include "../../utils/gui.h"
#include "../elems/basics/boxtypes.h"
#include "../elems/mainWindow/mainIO.h"
#include "../elems/mainWindow/mainMenu.h"
#include "../elems/mainWindow/mainTimer.h"
#include "../elems/mainWindow/mainTransport.h"
#include "../elems/mainWindow/beatMeter.h"
#include "../elems/mainWindow/keyboard/keyboard.h"
#include "warnings.h"
#include "mainWindow.h"


extern gdMainWindow* G_MainWin;


using namespace giada;


gdMainWindow::gdMainWindow(int W, int H, const char* title, int argc, char** argv)
	: gdWindow(W, H, title)
{
	Fl::visible_focus(0);

	Fl::background(25, 25, 25);

	Fl::set_boxtype(G_CUSTOM_BORDER_BOX, g_customBorderBox, 1, 1, 2, 2);
	Fl::set_boxtype(G_CUSTOM_UP_BOX,     g_customUpBox,     1, 1, 2, 2);
	Fl::set_boxtype(G_CUSTOM_DOWN_BOX,   g_customDownBox,   1, 1, 2, 2);

	Fl::set_boxtype(FL_BORDER_BOX, G_CUSTOM_BORDER_BOX);
	Fl::set_boxtype(FL_UP_BOX,     G_CUSTOM_UP_BOX);
	Fl::set_boxtype(FL_DOWN_BOX,   G_CUSTOM_DOWN_BOX);

	size_range(G_MIN_GUI_WIDTH, G_MIN_GUI_HEIGHT);

	mainMenu      = new v::geMainMenu(8, -1);
	mainIO        = new v::geMainIO(412, 8);
	mainTransport = new v::geMainTransport(8, 39);
	mainTimer     = new v::geMainTimer(598, 44);
	beatMeter     = new v::geBeatMeter(100, 83, 609, 20);
	keyboard      = new v::geKeyboard(8, 122, w()-16, 380);

	/* zone 1 - menus, and I/O tools */

	Fl_Group* zone1 = new Fl_Group(8, 8, W-16, 20);
	zone1->add(mainMenu);
	zone1->resizable(new Fl_Box(300, 8, 80, 20));
	zone1->add(mainIO);

	/* zone 2 - mainTransport and timing tools */

	Fl_Group* zone2 = new Fl_Group(8, mainTransport->y(), W-16, mainTransport->h());
	zone2->add(mainTransport);
	zone2->resizable(new Fl_Box(mainTransport->x()+mainTransport->w()+4, zone2->y(), 80, 20));
	zone2->add(mainTimer);

	/* zone 3 - beat meter */

	Fl_Group* zone3 = new Fl_Group(8, beatMeter->y(), W-16, beatMeter->h());
	zone3->add(beatMeter);

	/* zone 4 - the keyboard (Fl_Group is unnecessary here, keyboard is
	 * a group by itself) */

	resizable(keyboard);

	add(zone1);
	add(zone2);
	add(zone3);
	add(keyboard);
	callback(cb_endprogram);
	gu_setFavicon(this);

	show(argc, argv);
}


/* -------------------------------------------------------------------------- */


void gdMainWindow::cb_endprogram(Fl_Widget* v, void* p) { G_MainWin->cb_endprogram(); }


/* -------------------------------------------------------------------------- */


void gdMainWindow::cb_endprogram()
{
	if (!gdConfirmWin("Warning", "Quit Giada: are you sure?"))
		return;

	m::conf::mainWindowX = x();
	m::conf::mainWindowY = y();
	m::conf::mainWindowW = w();
	m::conf::mainWindowH = h();

	hide();
	delete this;
}
