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

#ifndef GD_PLUGINLIST_H
#define GD_PLUGINLIST_H

#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include "window.h"


class Plugin;
class Channel;
class geButton;
class gdPluginList;
class geIdButton;
class geChoice;


class gdPluginList : public gdWindow
{
private:

  geButton  *addPlugin;
	Fl_Scroll *list;

	static void cb_addPlugin(Fl_Widget *v, void *p);
	void cb_addPlugin();

public:

	Channel *ch;      // ch == nullptr ? masterOut
	int stackType;

	gdPluginList(int stackType, Channel *ch=nullptr);
	~gdPluginList();

	/* special callback, passed to browser. When closed (i.e. plugin
	 * has been selected) the same browser will refresh this window. */

	static void cb_refreshList(Fl_Widget*, void*);

	void refreshList();
};


/* -------------------------------------------------------------------------- */


class gdPlugin : public Fl_Group
{
private:

  gdPluginList *pParent;
  Plugin       *pPlugin;

	static void cb_removePlugin(Fl_Widget *v, void *p);
	static void cb_openPluginWindow(Fl_Widget *v, void *p);
	static void cb_setBypass(Fl_Widget *v, void *p);
	static void cb_shiftUp(Fl_Widget *v, void *p);
	static void cb_shiftDown(Fl_Widget *v, void *p);
	static void cb_setProgram(Fl_Widget *v, void *p);
	void cb_removePlugin();
	void cb_openPluginWindow();
	void cb_setBypass();
	void cb_shiftUp();
	void cb_shiftDown();
	void cb_setProgram();

public:

	geIdButton *button;
	geChoice    *program;
	geIdButton *bypass;
	geIdButton *shiftUp;
	geIdButton *shiftDown;
	geIdButton *remove;

	gdPlugin(gdPluginList *gdp, Plugin *p, int x, int y, int w);
};

#endif

#endif // #ifdef WITH_VST
