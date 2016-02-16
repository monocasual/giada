/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindowGUI
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


#include "../../utils/log.h"
#include "../../utils/gui_utils.h"
#include "../../core/pluginHost.h"
#include "../../core/plugin.h"
#include "../../core/const.h"
#include "../elems/ge_mixed.h"
#include "gd_pluginWindowGUI.h"


extern PluginHost G_PluginHost;


gdPluginWindowGUI::gdPluginWindowGUI(Plugin *pPlugin)
 : gWindow(450, 300), pPlugin(pPlugin)
{
  show();
  Fl::check();

  gLog("[gdPluginWindowGUI] opening GUI, this=%p, xid=%p\n",
    (void*) this, (void*) fl_xid(this));

  pPlugin->initEditor((void*) fl_xid(this));
  resize(0, 0, pPlugin->getEditorW(), pPlugin->getEditorH());

  Fl::add_timeout(GUI_PLUGIN_RATE, cb_refresh, (void*) this);

  copy_label(pPlugin->getName().toStdString().c_str());

}


/* -------------------------------------------------------------------------- */


void gdPluginWindowGUI::cb_close(Fl_Widget *v, void *p)   { ((gdPluginWindowGUI*)p)->__cb_close(); }
void gdPluginWindowGUI::cb_refresh(void *data) { ((gdPluginWindowGUI*)data)->__cb_refresh(); }


/* -------------------------------------------------------------------------- */


void gdPluginWindowGUI::__cb_close()
{
  Fl::remove_timeout(cb_refresh);
  pPlugin->closeEditor();
  gLog("[gdPluginWindowGUI::__cb_close] GUI closed, this=%p\n", (void*) this);
}


/* -------------------------------------------------------------------------- */


void gdPluginWindowGUI::__cb_refresh()
{
  //gLog("[gdPluginWindowGUI::__cb_refresh] refresh!\n");
  G_PluginHost.runDispatchLoop();
  Fl::repeat_timeout(GUI_PLUGIN_RATE, cb_refresh, (void*) this);
}


/* -------------------------------------------------------------------------- */


gdPluginWindowGUI::~gdPluginWindowGUI()
{
  __cb_close();
}

#endif // #ifdef WITH_VST
