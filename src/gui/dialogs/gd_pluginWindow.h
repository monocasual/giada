/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindow
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

#ifndef __GD_PLUGIN_WINDOW_H__
#define __GD_PLUGIN_WINDOW_H__


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include "../elems/ge_window.h"


class gdPluginWindow : public gWindow
{
private:
	class Plugin *pPlugin;

public:
	int id;

	gdPluginWindow(Plugin *pPlugin);
};


/* -------------------------------------------------------------------------- */


class Parameter : public Fl_Group
{
private:
	int   paramIndex;
	class Plugin *pPlugin;

	static void cb_setValue(Fl_Widget *v, void *p);
	inline void __cb_setValue();

	void updateValue();

public:
	class gBox    *label;
	class gSlider *slider;
	class gBox    *value;

	Parameter(int paramIndex, class Plugin *p, int x, int y, int w);
};


#endif

#endif // #ifdef WITH_VST
