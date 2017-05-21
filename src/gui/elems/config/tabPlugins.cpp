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


#ifdef WITH_VST


#include <FL/Fl.H>
#include "../../../core/const.h"
#include "../../../core/conf.h"
#include "../../../core/pluginHost.h"
#include "../../../utils/string.h"
#include "../../../utils/fs.h"
#include "../basics/box.h"
#include "../basics/radio.h"
#include "../basics/check.h"
#include "../basics/input.h"
#include "../basics/button.h"
#include "tabPlugins.h"


using std::string;
using namespace giada::m;


geTabPlugins::geTabPlugins(int X, int Y, int W, int H)
	: Fl_Group(X, Y, W, H, "Plugins")
{
	folderPath = new geInput(x()+w()-250, y()+8, 250, 20);
	scanButton = new geButton(x()+w()-120, folderPath->y()+folderPath->h()+8, 120, 20);
	info       = new geBox(x(), scanButton->y()+scanButton->h()+8, w(), 242);

	end();

	labelsize(GUI_FONT_SIZE_BASE);

	info->label("Scan in progress. Please wait...");
	info->hide();

	folderPath->value(conf::pluginPath.c_str());
	folderPath->label("Plugins folder");

	scanButton->callback(cb_scan, (void*) this);

	updateCount();
}


/* -------------------------------------------------------------------------- */


void geTabPlugins::updateCount()
{
	string scanLabel = "Scan (" + gu_itoa(pluginHost::countAvailablePlugins()) + " found)";
	scanButton->label(scanLabel.c_str());
}


/* -------------------------------------------------------------------------- */


void geTabPlugins::cb_scan(Fl_Widget *w, void *p) { ((geTabPlugins*)p)->__cb_scan(w); }


/* -------------------------------------------------------------------------- */


void geTabPlugins::cb_onScan(float progress, void *p)
{
	string l = "Scan in progress (" + gu_itoa((int)(progress*100)) + "%). Please wait...";
	((geTabPlugins *)p)->info->label(l.c_str());
	Fl::wait();
}


/* -------------------------------------------------------------------------- */


void geTabPlugins::__cb_scan(Fl_Widget *w)
{
	info->show();
	pluginHost::scanDir(folderPath->value(), cb_onScan, (void*) this);
	pluginHost::saveList(gu_getHomePath() + G_SLASH + "plugins.xml");
	info->hide();
	updateCount();
}


/* -------------------------------------------------------------------------- */


void geTabPlugins::save()
{
	conf::pluginPath = folderPath->value();
}


#endif // WITH_VST
