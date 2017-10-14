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

#ifndef GD_PLUGIN_CHOOSER_H
#define GD_PLUGIN_CHOOSER_H


#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include "window.h"


class Channel;
class geChoice;
class geButton;
class geButton;
class gePluginBrowser;


class gdPluginChooser : public gdWindow
{
private:

  Channel *ch;      // ch == nullptr ? masterOut
	int stackType;

  geChoice         *sortMethod;
  geButton        *addBtn;
  geButton        *cancelBtn;
  gePluginBrowser *browser;

	static void cb_close(Fl_Widget *w, void *p);
	static void cb_add  (Fl_Widget *w, void *p);
	static void cb_sort (Fl_Widget *w, void *p);
  inline void __cb_close();
  inline void __cb_add  ();
  inline void __cb_sort ();

public:

	gdPluginChooser(int x, int y, int w, int h, int stackType, Channel *ch=nullptr);
  ~gdPluginChooser();
};


#endif

#endif // #ifdef WITH_VST
