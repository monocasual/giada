/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 * gd_mainWindow
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#ifndef GD_MAINWINDOW_H
#define GD_MAINWINDOW_H


#include "../elems/ge_window.h"


class gdMainWindow : public gWindow
{
private:

	static void cb_endprogram  (class Fl_Widget *v, void *p);
	inline void __cb_endprogram();

public:

	class geKeyboard       *keyboard;
	class geBeatMeter     *beatMeter;
	class geMainMenu      *mainMenu;
	class geMainIO        *mainIO;
  class geMainTimer     *mainTimer;
	class geMainTransport *mainTransport;

	gdMainWindow(int w, int h, const char *title, int argc, char **argv);
};


#endif
