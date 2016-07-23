/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_browser
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_BROWSER_H
#define GE_BROWSER_H

#include <FL/Fl.H>
#include <FL/Fl_File_Browser.H>
#include <string>
#include "ge_mixed.h"


using std::string;


class gBrowser : public Fl_File_Browser
{
private:

	string currentDir;

	/* normalize
	 * Make sure the string never ends with a trailing slash. */

	string normalize(const string &s);

public:

	gBrowser(int x, int y, int w, int h);

	/* init
	 * Initialize browser and show 'dir' as initial directory. */

	void loadDir(const string &dir);

	/* getSelectedItem
	 * Return the full path or just the displayed name of the i-th selected item.
	 * Always with the trailing slash! */

	string getSelectedItem(bool fullPath=true);

	string getCurrentDir();

	void preselect(int position, int line);

	int handle(int e);
};

#endif
