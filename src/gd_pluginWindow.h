/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindow
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
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

#ifndef __GD_PLUGINWINDOW_H__
#define __GD_PLUGINWINDOW_H__


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include "pluginHost.h"
#include "ge_mixed.h"


class gdPluginWindow : public gWindow {
private:

	Plugin                    *pPlugin;
	gVector <class gSlider *>  sliders;
	gVector <class gBox *>     values;

	static void cb_setValue(Fl_Widget *v, void *p);
	inline void __cb_setValue(Fl_Widget *v);


public:

	int id;

	gdPluginWindow(Plugin *pPlugin);
	~gdPluginWindow();

};

#endif

#endif // #ifdef WITH_VST
