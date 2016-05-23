/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_pluginBrowser
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


#ifdef WITH_VST

#ifndef GE_PLUGIN_BROWSER_H
#define GE_PLUGIN_BROWSER_H


#include <vector>
#include <FL/Fl.H>
#include <FL/Fl_Browser.H>


using std::vector;


class gePluginBrowser : public Fl_Browser
{
private:

	int widths[5] = {0};

	void computeWidths();

public:

	gePluginBrowser(int x, int y, int w, int h);

	void refresh();
};

#endif

#endif
