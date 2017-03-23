/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginChooser
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

#ifndef __GD_PLUGIN_CHOOSER_H__
#define __GD_PLUGIN_CHOOSER_H__

#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include "../elems/ge_window.h"


class gdPluginChooser : public gWindow {

private:

  class Channel *ch;      // ch == nullptr ? masterOut
	int   stackType;

  class gChoice         *sortMethod;
  class gClick          *addBtn;
  class gClick          *cancelBtn;
	class gePluginBrowser *browser;

	static void cb_close(Fl_Widget *w, void *p);
	static void cb_add  (Fl_Widget *w, void *p);
	static void cb_sort (Fl_Widget *w, void *p);
  inline void __cb_close();
  inline void __cb_add  ();
  inline void __cb_sort ();

public:

	gdPluginChooser(int x, int y, int w, int h, int stackType, class Channel *ch=nullptr);
  ~gdPluginChooser();
};


#endif

#endif // #ifdef WITH_VST
