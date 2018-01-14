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


#include "../core/pluginHost.h"
#include "../core/mixer.h"
#include "../core/plugin.h"
#include "../core/channel.h"
#include "../core/const.h"
#include "../core/conf.h"
#include "../utils/gui.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/pluginWindow.h"
#include "../gui/dialogs/pluginList.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../gui/dialogs/gd_config.h"
#include "../gui/dialogs/browser/browserDir.h"
#include "plugin.h"


extern gdMainWindow* G_MainWin;


using namespace giada::m;


namespace giada {
namespace c     {
namespace plugin 
{
namespace
{
/* getPluginWindow
Returns the plugInWindow (GUI-less one) with the parameter list. It might be 
nullptr if there is no plug-in window shown on screen. */

gdPluginWindow* getPluginWindow(const Plugin* p)
{
	/* Get the parent window first: the plug-in list. Then, if it exists, get
	the child window - the actual pluginWindow. */

	gdPluginList* parent = static_cast<gdPluginList*>(gu_getSubwindow(G_MainWin, WID_FX_LIST));
	if (parent == nullptr)
		return nullptr;
	return static_cast<gdPluginWindow*>(gu_getSubwindow(parent, p->getId() + 1));
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


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


void setProgram(Plugin* p, int index)
{
	p->setCurrentProgram(index);

	/* No need to update plug-in editor if it has one: the plug-in's editor takes
	care of it on its own. Conversely, update the specific parameter for UI-less 
	plug-ins. */

	if (p->hasEditor())
		return;

	gdPluginWindow* child = getPluginWindow(p);
	if (child == nullptr) 
		return;
	
	child->updateParameters(true);
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

	gdPluginWindow* child = getPluginWindow(p);
	if (child == nullptr) 
		return;

	Fl::lock();
	child->updateParameter(index, !gui);
	Fl::unlock();
}


/* -------------------------------------------------------------------------- */


void setPluginPathCb(void* data)
{
	gdBrowserDir* browser = (gdBrowserDir*) data;

	if (browser->getCurrentPath() == "") {
		gdAlert("Invalid path.");
		return;
	}

	if (!conf::pluginPath.empty() && conf::pluginPath.back() != ';')
		conf::pluginPath += ";";
	conf::pluginPath += browser->getCurrentPath();

	browser->do_callback();

	gdConfig* configWin = static_cast<gdConfig*>(gu_getSubwindow(G_MainWin, WID_CONFIG));
	configWin->refreshVstPath();
}

}}}; // giada::c::plugin::


#endif
