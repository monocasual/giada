/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginList
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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

#ifndef __GD_PLUGINLIST_H__
#define __GD_PLUGINLIST_H__

#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include "ge_window.h"


class gdPluginList : public gWindow {

private:

 	class gClick *addPlugin;
	Fl_Scroll    *list;

	//gVector<class gdPluginWindow *> subWindows;

	static void cb_addPlugin          (Fl_Widget *v, void *p);
	inline void __cb_addPlugin        ();

public:

	struct channel *ch;      // ch == NULL ? masterOut
	int    stackType;

	gdPluginList(int stackType, struct channel *ch=NULL);
	~gdPluginList();

	/* special callback, passed to browser. When closed (i.e. plugin
	 * has been selected) the same browser will refresh this window. */

	static void cb_refreshList(Fl_Widget*, void*);

	void refreshList();
};


/* ------------------------------------------------------------------ */


class gdPlugin : public Fl_Group {

private:

	class  gdPluginList *pParent;
	class  Plugin       *pPlugin;

	static void cb_removePlugin       (Fl_Widget *v, void *p);
	static void cb_openPluginWindow   (Fl_Widget *v, void *p);
	static void cb_setBypass          (Fl_Widget *v, void *p);
	static void cb_shiftUp            (Fl_Widget *v, void *p);
	static void cb_shiftDown          (Fl_Widget *v, void *p);
	static void cb_setProgram         (Fl_Widget *v, void *p);
	inline void __cb_removePlugin     (Fl_Widget *v);
	inline void __cb_openPluginWindow (Fl_Widget *v);
	inline void __cb_setBypass        (Fl_Widget *v);
	inline void __cb_shiftUp          (Fl_Widget *v);
	inline void __cb_shiftDown        (Fl_Widget *v);
	inline void __cb_setProgram       (Fl_Widget *v);

public:

	class gButton *button;
	class gChoice *program;
	class gButton *bypass;
	class gButton *shiftUp;
	class gButton *shiftDown;
	class gButton *remove;

	gdPlugin(gdPluginList *gdp, class Plugin *p, int x, int y, int w);

};

#endif

#endif // #ifdef WITH_VST
