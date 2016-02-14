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


#include "../../core/plugin.h"
#include "../../core/pluginHost.h"
#include "ge_pluginBrowser.h"


extern PluginHost G_PluginHost;


using std::vector;


gePluginBrowser::gePluginBrowser(int x, int y, int w, int h)
	: Fl_Browser(x, y, w, h)
{
	// http://seriss.com/people/erco/fltk/#BrowserColumns

	int widths[] = {300, 300, 0}; // widths for each column
  column_widths(widths);
  column_char('\t');       // tabs as column delimiters
  type(FL_MULTI_BROWSER);

	add("ID\tNAME\tCATEGORY");        // lines of tab delimited data
	for (int i=0; i<G_PluginHost.countAvailablePlugins(); i++) {
		PluginHost::PluginInfo pi = G_PluginHost.getAvailablePluginInfo(i);
		string s = pi.uid + "\t" + pi.name + "\t" + pi.category;
		add(s.c_str());
	}
	//box(FL_BORDER_BOX);
	end();
}


#endif
