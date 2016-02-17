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
#include "../../core/const.h"
#include "../../core/pluginHost.h"
#include "ge_mixed.h"
#include "ge_pluginBrowser.h"


extern PluginHost G_PluginHost;


using std::vector;


gePluginBrowser::gePluginBrowser(int x, int y, int w, int h)
	: Fl_Browser(x, y, w, h)
{
	box(G_BOX);
	textsize(GUI_FONT_SIZE_BASE);
	textcolor(COLOR_TEXT_0);
	selection_color(COLOR_BG_1);
	color(COLOR_BG_0);

	this->scrollbar.color(COLOR_BG_0);
	this->scrollbar.selection_color(COLOR_BG_1);
	this->scrollbar.labelcolor(COLOR_BD_1);
	this->scrollbar.slider(G_BOX);

	this->hscrollbar.color(COLOR_BG_0);
	this->hscrollbar.selection_color(COLOR_BG_1);
	this->hscrollbar.labelcolor(COLOR_BD_1);
	this->hscrollbar.slider(G_BOX);

	type(FL_HOLD_BROWSER);

	computeWidths();

  column_widths(widths);
  column_char('\t');       // tabs as column delimiters

	add("NAME\tMANUFACTURER\tCATEGORY\tID");
	add("---\t---\t---\t---");
	for (int i=0; i<G_PluginHost.countAvailablePlugins(); i++) {
		PluginHost::PluginInfo pi = G_PluginHost.getAvailablePluginInfo(i);
		string s = pi.name + "\t" + pi.manufacturerName + "\t" + pi.category + "\t" + pi.uid;
		add(s.c_str());
	}

	end();
}


/* -------------------------------------------------------------------------- */


void gePluginBrowser::computeWidths()
{
	int w0, w1;
	for (int i=0; i<G_PluginHost.countAvailablePlugins(); i++) {
		PluginHost::PluginInfo pi = G_PluginHost.getAvailablePluginInfo(i);
		w0 = (int) fl_width(pi.name.c_str());
		w1 = (int) fl_width(pi.manufacturerName.c_str());
		if (w0 > widths[0]) widths[0] = w0;
		if (w1 > widths[1]) widths[1] = w1;
	}
	widths[0] += 60;
	widths[1] += 60;
	widths[2] = fl_width("CATEGORY") + 60;
	widths[3] = 0;
}


#endif
