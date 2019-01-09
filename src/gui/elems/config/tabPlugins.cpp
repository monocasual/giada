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


#ifdef WITH_VST


#include <functional>
#include <FL/Fl.H>
#include "../../../core/const.h"
#include "../../../core/conf.h"
#include "../../../core/graphics.h"
#include "../../../core/pluginManager.h"
#include "../../../glue/plugin.h"
#include "../../../utils/string.h"
#include "../../../utils/fs.h"
#include "../../../utils/gui.h"
#include "../../dialogs/window.h"
#include "../../dialogs/gd_mainWindow.h"
#include "../../dialogs/browser/browserDir.h"
#include "../basics/box.h"
#include "../basics/radio.h"
#include "../basics/check.h"
#include "../basics/input.h"
#include "../basics/button.h"
#include "tabPlugins.h"


extern gdMainWindow* G_MainWin;


using std::string;
using namespace giada::m;


geTabPlugins::geTabPlugins(int X, int Y, int W, int H)
	: Fl_Group(X, Y, W, H, "Plugins")
{
	m_browse     = new geButton(x()+w()-G_GUI_UNIT, y()+9, G_GUI_UNIT, G_GUI_UNIT, "", zoomInOff_xpm, zoomInOn_xpm);
	m_folderPath = new geInput(m_browse->x()-258, y()+9, 250, G_GUI_UNIT);
	m_scanButton = new geButton(x()+w()-120, m_folderPath->y()+m_folderPath->h()+8, 120, G_GUI_UNIT);
	m_info       = new geBox(x(), m_scanButton->y()+m_scanButton->h()+8, w(), 242);

	end();

	labelsize(G_GUI_FONT_SIZE_BASE);
	selection_color(G_COLOR_GREY_4);

	m_info->label("Scan in progress. Please wait...");
	m_info->hide();

	m_folderPath->value(conf::pluginPath.c_str());
	m_folderPath->label("Plugins folder");

	m_browse->callback(cb_browse, (void*) this);

	m_scanButton->callback(cb_scan, (void*) this);

	refreshCount();
}


/* -------------------------------------------------------------------------- */


void geTabPlugins::refreshCount()
{
	string scanLabel = "Scan (" + gu_iToString(pluginManager::countAvailablePlugins()) + " found)";
	m_scanButton->label(scanLabel.c_str());
}


/* -------------------------------------------------------------------------- */


void geTabPlugins::cb_scan(Fl_Widget* w, void* p) { ((geTabPlugins*)p)->cb_scan(); }
void geTabPlugins::cb_browse(Fl_Widget* w, void* p) { ((geTabPlugins*)p)->cb_browse(); }


/* -------------------------------------------------------------------------- */


void geTabPlugins::cb_browse()
{
	gdBrowserDir* browser = new gdBrowserDir(0, 0, 800, 600, "Add plug-ins directory", 
		conf::patchPath, giada::c::plugin::setPluginPathCb);

	static_cast<gdWindow*>(top_window())->addSubWindow(browser);
}


/* -------------------------------------------------------------------------- */


void geTabPlugins::cb_scan()
{
	std::function<void(float)> callback = [this] (float progress) 
	{
		string l = "Scan in progress (" + gu_iToString((int)(progress*100)) + "%). Please wait...";
		m_info->label(l.c_str());
		Fl::wait();
	};

	m_info->show();
	pluginManager::scanDirs(m_folderPath->value(), callback);
	pluginManager::saveList(gu_getHomePath() + G_SLASH + "plugins.xml");
	m_info->hide();
	refreshCount();
}


/* -------------------------------------------------------------------------- */


void geTabPlugins::save()
{
	conf::pluginPath = m_folderPath->value();
}


/* -------------------------------------------------------------------------- */


void geTabPlugins::refreshVstPath()
{
	m_folderPath->value(conf::pluginPath.c_str());
	m_folderPath->redraw();
}


#endif // WITH_VST
