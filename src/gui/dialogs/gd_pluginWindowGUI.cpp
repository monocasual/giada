/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindowGUI
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


#include "../../utils/log.h"
#include "../../utils/gui.h"
#include "../../core/pluginHost.h"
#include "../../core/plugin.h"
#include "../../core/const.h"
#include "../elems/ge_mixed.h"
#include "gd_pluginWindowGUI.h"

#ifdef __APPLE__
#import "../../utils/cocoa.h" // objective-c
#endif


extern PluginHost G_PluginHost;


gdPluginWindowGUI::gdPluginWindowGUI(Plugin *pPlugin)
 : gWindow(450, 300), pPlugin(pPlugin)
{
  show();

#ifndef __APPLE__

  Fl::check();

#endif

  gu_log("[gdPluginWindowGUI] opening GUI, this=%p, xid=%p\n",
    (void*) this, (void*) fl_xid(this));

#if defined(__APPLE__)

  void *cocoaWindow = (void*) fl_xid(this);
  cocoa_setWindowSize(cocoaWindow, pPlugin->getEditorW(), pPlugin->getEditorH());
  pPlugin->showEditor(cocoa_getViewFromWindow(cocoaWindow));

#else

  pPlugin->showEditor((void*) fl_xid(this));

#endif

  int pluginW = pPlugin->getEditorW();
  int pluginH = pPlugin->getEditorH();

  resize((Fl::w() - pluginW) / 2, (Fl::h() - pluginH) / 2, pluginW, pluginH);

  Fl::add_timeout(GUI_PLUGIN_RATE, cb_refresh, (void*) this);

  copy_label(pPlugin->getName().c_str());

}


/* -------------------------------------------------------------------------- */


void gdPluginWindowGUI::cb_close(Fl_Widget *v, void *p)   { ((gdPluginWindowGUI*)p)->__cb_close(); }
void gdPluginWindowGUI::cb_refresh(void *data) { ((gdPluginWindowGUI*)data)->__cb_refresh(); }


/* -------------------------------------------------------------------------- */


void gdPluginWindowGUI::__cb_close()
{
  Fl::remove_timeout(cb_refresh);
  pPlugin->closeEditor();
  gu_log("[gdPluginWindowGUI::__cb_close] GUI closed, this=%p\n", (void*) this);
}


/* -------------------------------------------------------------------------- */


void gdPluginWindowGUI::__cb_refresh()
{
  //gu_log("[gdPluginWindowGUI::__cb_refresh] refresh!\n");
  G_PluginHost.runDispatchLoop();
  Fl::repeat_timeout(GUI_PLUGIN_RATE, cb_refresh, (void*) this);
}


/* -------------------------------------------------------------------------- */


gdPluginWindowGUI::~gdPluginWindowGUI()
{
  __cb_close();
}

#endif // #ifdef WITH_VST
