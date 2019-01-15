/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <FL/x.H>
#include "../../utils/log.h"
#include "../../utils/gui.h"
#include "../../core/pluginHost.h"
#include "../../core/plugin.h"
#include "../../core/const.h"
#include "pluginWindowGUI.h"

#ifdef G_OS_MAC
#import "../../utils/cocoa.h" // objective-c
#endif


using namespace giada::m;


gdPluginWindowGUI::gdPluginWindowGUI(Plugin* plugin)
#ifdef G_OS_MAC
 : gdWindow(Fl::w(), Fl::h()), m_plugin(plugin)
#else
 : gdWindow(320, 200), m_plugin(plugin)
#endif
{
  show();

#if defined(G_OS_LINUX) || defined(G_OS_MAC) || defined(G_OS_FREEBSD)

  /*  Fl_Window::show() is not guaranteed to show and draw the window on all 
  platforms immediately. Instead this is done in the background; particularly on 
  X11 it will take a few messages (client server roundtrips) to display the 
  window. Usually this small delay doesn't matter, but in some cases you may 
  want to have the window instantiated and displayed synchronously. Currently 
  (as of FLTK 1.3.4) this method has an effect on X11 and Mac OS. 

  http://www.fltk.org/doc-1.3/classFl__Window.html#aafbec14ca8ff8abdaff77a35ebb23dd8 */

  wait_for_expose();
  Fl::flush();

#endif

  gu_log("[gdPluginWindowGUI] opening GUI, this=%p, xid=%p\n",
    (void*) this, (void*) fl_xid(this));

#ifdef G_OS_MAC

  void* cocoaWindow = (void*) fl_xid(this);
  m_plugin->showEditor(cocoa_getViewFromWindow(cocoaWindow));

#else

  m_plugin->showEditor((void*) fl_xid(this));

  int pluginW = m_plugin->getEditorW();
  int pluginH = m_plugin->getEditorH();

  resize((Fl::w() - pluginW) / 2, (Fl::h() - pluginH) / 2, pluginW, pluginH);


#endif

  Fl::add_timeout(G_GUI_PLUGIN_RATE, cb_refresh, (void*) this);

  copy_label(m_plugin->getName().c_str());

}


/* -------------------------------------------------------------------------- */


void gdPluginWindowGUI::cb_close(Fl_Widget* v, void* p) { ((gdPluginWindowGUI*)p)->cb_close(); }
void gdPluginWindowGUI::cb_refresh(void* data) { ((gdPluginWindowGUI*)data)->cb_refresh(); }


/* -------------------------------------------------------------------------- */


void gdPluginWindowGUI::cb_close()
{
  Fl::remove_timeout(cb_refresh);
  m_plugin->closeEditor();
  gu_log("[gdPluginWindowGUI::__cb_close] GUI closed, this=%p\n", (void*) this);
}


/* -------------------------------------------------------------------------- */


void gdPluginWindowGUI::cb_refresh()
{
  pluginHost::runDispatchLoop();
  Fl::repeat_timeout(G_GUI_PLUGIN_RATE, cb_refresh, (void*) this);
}


/* -------------------------------------------------------------------------- */


gdPluginWindowGUI::~gdPluginWindowGUI()
{
  cb_close();
}

#endif // #ifdef WITH_VST
