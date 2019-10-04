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


#ifndef GD_MAINWINDOW_H
#define GD_MAINWINDOW_H


#include "window.h"


class Fl_Widget;


namespace giada {
namespace v 
{
class geKeyboard;
class geMainIO;
class geMainMenu;
class geBeatMeter;
class geMainTransport;
class geMainTimer;
class gdMainWindow : public gdWindow
{
public:

    gdMainWindow(int w, int h, const char* title, int argc, char** argv);
	~gdMainWindow();

    void rebuild() override;

	void clearKeyboard(bool createColumns);

	geKeyboard*      keyboard;
	geBeatMeter*     beatMeter;
	geMainMenu*      mainMenu;
	geMainIO*        mainIO;
	geMainTimer*     mainTimer;
	geMainTransport* mainTransport;
};
}} // giada::v::


#endif
