/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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


#ifndef GD_BROWSER_SAVE_H
#define GD_BROWSER_SAVE_H


#include "../window.h"
#include "browserBase.h"


class Channel;
class Fl_Group;
class geCheck;
class geBrowser;
class geButton;
class geInput;
class geProgress;


class gdBrowserSave : public gdBrowserBase
{
private:

	geInput* name;

	static void cb_down(Fl_Widget* v, void* p);
	static void cb_save(Fl_Widget* w, void* p);
	void cb_down();
	void cb_save();

public:

	gdBrowserSave(int x, int y, int w, int h, const std::string& title,
			const std::string& path, const std::string& name, void (*callback)(void*),
			Channel* ch);

	std::string getName();
};


#endif
