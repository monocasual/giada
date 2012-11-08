/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_browser
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */

#ifndef GE_BROWSER_H
#define GE_BROWSER_H

#include <FL/Fl.H>
#include <FL/Fl_Hold_Browser.H>
#include <string>
#include "ge_mixed.h"

class gBrowser : public Fl_Hold_Browser {
public:
	gBrowser(int x, int y, int w, int h, const char *L=0);
	~gBrowser();
	void init(const char *init_path=NULL);
	void refresh();
	void sort();
	void up_dir();
	void down_dir(const char *path);
	const char *get_selected_item();

	/* path_obj
	 * the actual path*/

	class gInput *path_obj;

	/* selected_item
	 * choosen item */

	std::string selected_item;

#ifdef _WIN32
private:

	/* showDrives [WIN32 only]
	 * lists all the available drivers */

	void showDrives();
#endif
};

#endif
