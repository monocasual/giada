/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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

#ifndef GD_PLUGINLIST_H
#define GD_PLUGINLIST_H


#include "../../core/pluginHost.h"
#include "window.h"


class Fl_Scroll;
class geButton;


class gdPluginList : public gdWindow
{
private:

	geButton*  addPlugin;
	Fl_Scroll* list;

	static void cb_addPlugin(Fl_Widget* v, void* p);
	void cb_addPlugin();

public:

	giada::m::Channel* ch;      // ch == nullptr ? masterOut
	giada::m::pluginHost::StackType stackType;

	gdPluginList(giada::m::pluginHost::StackType t, giada::m::Channel* ch=nullptr);
	~gdPluginList();

	/* special callback, passed to browser. When closed (i.e. plugin
	 * has been selected) the same browser will refresh this window. */

	static void cb_refreshList(Fl_Widget*, void*);

	void refreshList();
};


#endif

#endif // #ifdef WITH_VST
