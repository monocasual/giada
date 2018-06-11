
/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindowGUI
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#ifndef GD_PLUGIN_WINDOW_GUI_H
#define GD_PLUGIN_WINDOW_GUI_H


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include "window.h"
#if defined(__APPLE__)
	#include <Carbon/Carbon.h>
#endif


class Plugin;


class gdPluginWindowGUI : public gdWindow
{
private:

	Plugin* m_plugin;

	static void cb_close  (Fl_Widget* v, void* p);
	static void cb_refresh(void* data);
	inline void cb_close  ();
	inline void cb_refresh();

public:

	gdPluginWindowGUI(Plugin* p);
	~gdPluginWindowGUI();
};


#endif // include guard

#endif // #ifdef WITH_VST
