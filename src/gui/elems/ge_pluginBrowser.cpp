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


#include "ge_pluginBrowser.h"


gePluginBrowser::gePluginBrowser(int x, int y, int w, int h)
	: Fl_Browser(x, y, w, h)
{
	// http://seriss.com/people/erco/fltk/#Fl_Browser

	int widths[] = {100, 100, 0}; // widths for each column
  column_widths(widths);
  column_char('\t');       // tabs as column delimiters
  type(FL_MULTI_BROWSER);
  add("USER\tPID\tXXX");        // lines of tab delimited data
  add("abc\tdef\tghi");         // lines of tab delimited data
	box(FL_BORDER_BOX);
}


#endif
