/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindow
 *
 * ---------------------------------------------------------------------
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
 * ------------------------------------------------------------------ */

#ifdef WITH_VST

#ifndef __GD_PLUGIN_WINDOW_DEPR_H__
#define __GD_PLUGIN_WINDOW_DEPR_H__


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include "../elems/ge_window.h"


class gdPluginWindow_DEPR_ : public gWindow {

private:
	class Plugin_DEPR_ *pPlugin;

public:
	int id;

	gdPluginWindow_DEPR_(Plugin_DEPR_ *pPlugin);
};


/* ------------------------------------------------------------------ */


class Parameter_DEPR_ : public Fl_Group {

private:
	int   id;
	class Plugin_DEPR_ *pPlugin;

	static void cb_setValue(Fl_Widget *v, void *p);
	inline void __cb_setValue();

public:
	class gBox    *label;
	class gSlider *slider;
	class gBox    *value;

	Parameter_DEPR_(int id, class Plugin_DEPR_ *p, int x, int y, int w);
};


#endif

#endif // #ifdef WITH_VST
