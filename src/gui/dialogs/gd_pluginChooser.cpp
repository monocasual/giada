/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginChooser
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


#include "../../utils/gui_utils.h"
#include "../../core/channel.h"
#include "../../core/mixer.h"
#include "../../core/pluginHost.h"
#include "../elems/ge_pluginBrowser.h"
#include "../elems/ge_mixed.h"
#include "gd_pluginChooser.h"


extern PluginHost G_PluginHost;
extern Mixer      G_Mixer;


gdPluginChooser::gdPluginChooser(int X, int Y, int W, int H, int stackType, class Channel *ch)
  : gWindow(640, 480, "Available plugins"), ch(ch), stackType(stackType)
{
  browser = new gePluginBrowser(8, 8, w()-16, h()-44);

  Fl_Group *group_btn = new Fl_Group(8, browser->y()+browser->h()+8, w()-16, h()-browser->h()-16);
    gBox *b = new gBox(8, browser->y()+browser->h(), 100, 20); 	// spacer window border <-> buttons
    addBtn = new gClick(w()-88, group_btn->y(), 80, 20, "Add");
    cancelBtn = new gClick(addBtn->x()-88, group_btn->y(), 80, 20, "Cancel");
  group_btn->resizable(b);
  group_btn->end();

  end();

  addBtn->callback(cb_add, (void*) this);
  cancelBtn->callback(cb_close, (void*) this);

  resizable(browser);
	gu_setFavicon(this);
  show();
}


/* -------------------------------------------------------------------------- */


void gdPluginChooser::cb_close(Fl_Widget *v, void *p)  { ((gdPluginChooser*)p)->__cb_close(); }
void gdPluginChooser::cb_add(Fl_Widget *v, void *p)  { ((gdPluginChooser*)p)->__cb_add(); }


/* -------------------------------------------------------------------------- */


void gdPluginChooser::__cb_close() {
	do_callback();
}


/* -------------------------------------------------------------------------- */


void gdPluginChooser::__cb_add() {
  int index = browser->value() - 3; // subtract header lines
  if (index >= 0) {
    printf("loading %d\n", index);
    G_PluginHost.addPlugin(index, stackType, &G_Mixer.mutex_plugins, ch);
  }
}

#endif // #ifdef WITH_VST
