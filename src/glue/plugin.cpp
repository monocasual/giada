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


#include "../core/pluginHost.h"
#include "../core/mixer.h"
#include "../core/plugin.h"
#include "../core/channel.h"
#include "../core/const.h"
#include "../utils/gui.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/pluginWindow.h"
#include "../gui/dialogs/pluginList.h"
#include "plugin.h"


extern gdMainWindow* G_MainWin;


using namespace giada::m;


namespace giada {
namespace c     {
namespace plugin 
{
Plugin* addPlugin(Channel* ch, int index, int stackType)
{
  if (index >= pluginHost::countAvailablePlugins())
    return nullptr;
  return pluginHost::addPlugin(index, stackType, &mixer::mutex_plugins, ch);
}


/* -------------------------------------------------------------------------- */


void swapPlugins(Channel* ch, int index1, int index2, int stackType)
{
  pluginHost::swapPlugin(index1, index2, stackType, &mixer::mutex_plugins,
    ch);
}


/* -------------------------------------------------------------------------- */


void freePlugin(Channel* ch, int index, int stackType)
{
  pluginHost::freePlugin(index, stackType, &mixer::mutex_plugins, ch);
}


/* -------------------------------------------------------------------------- */


void setParameter(Plugin* p, int index, float value, bool gui)
{
	p->setParameter(index, value);

	/* No need to update plug-in editor if it has one: the plug-in's editor takes
	care of it on its own. Conversely, update the specific parameter for UI-less 
	plug-ins. */

	if (p->hasEditor())
		return;

	gdPluginList* parent = static_cast<gdPluginList*>(gu_getSubwindow(G_MainWin, WID_FX_LIST));
	gdPluginWindow* child = static_cast<gdPluginWindow*>(gu_getSubwindow(parent, p->getId() + 1));
	if (child != nullptr) {
		Fl::lock();
		child->updateParameter(index, !gui);
		Fl::unlock();
	}
}

}}}; // giada::c::plugin::


#endif
