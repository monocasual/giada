/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include <FL/Fl.H>
#include "core/model/model.h"
#include "core/plugins/pluginManager.h"
#include "core/plugins/pluginHost.h"
#include "core/mixer.h"
#include "core/plugins/plugin.h"
#include "core/const.h"
#include "core/conf.h"
#include "utils/gui.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/pluginWindow.h"
#include "gui/dialogs/pluginList.h"
#include "gui/dialogs/warnings.h"
#include "gui/dialogs/config.h"
#include "gui/dialogs/browser/browserDir.h"
#include "plugin.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada::c::plugin 
{
Param::Param(const m::Plugin& p, int index, ID channelId)
: index    (index)
, pluginId (p.id)
, channelId(channelId)
, name     (p.getParameterName(index))
, text     (p.getParameterText(index))
, label    (p.getParameterLabel(index))
, value    (p.getParameter(index))
{
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Plugin::Plugin(m::Plugin& p, ID channelId)
: id            (p.id)
, channelId     (channelId)
, valid         (p.valid)
, hasEditor     (p.hasEditor())
, isBypassed    (p.isBypassed())
, name          (p.getName())
, uniqueId      (p.getUniqueId())
, currentProgram(p.getCurrentProgram())
, m_plugin      (p)
{
	for (int i = 0; i < p.getNumPrograms(); i++)
	 	programs.push_back({ i, p.getProgramName(i) });
	for (int i = 0; i < p.getNumParameters(); i++)
		paramIndexes.push_back(i);
}


/* -------------------------------------------------------------------------- */


juce::AudioProcessorEditor* Plugin::createEditor() const
{
	return m_plugin.createEditor();
}


/* -------------------------------------------------------------------------- */


const m::Plugin& Plugin::getPluginRef() const { return m_plugin; }


/* -------------------------------------------------------------------------- */


void Plugin::setResizeCallback(std::function<void(int, int)> f)
{
	m_plugin.onEditorResize = f;
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Plugins::Plugins(const m::channel::Data& c)
: channelId(c.id)
, plugins  (c.plugins) 
{
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Plugins getPlugins(ID channelId)
{
	return Plugins(m::model::get().getChannel(channelId));
}


Plugin getPlugin(m::Plugin& plugin, ID channelId)
{
	return Plugin(plugin, channelId);
}


Param getParam (int index, const m::Plugin& plugin, ID channelId)
{
	return Param(plugin, index, channelId);
}


/* -------------------------------------------------------------------------- */


void updateWindow(ID pluginId, bool gui)
{
    m::Plugin* p = m::model::find<m::Plugin>(pluginId);

    assert(p != nullptr);

    if (p->hasEditor())
        return;

    /* Get the parent window first: the plug-in list. Then, if it exists, get
    the child window - the actual pluginWindow. */

    v::gdPluginList* parent = static_cast<v::gdPluginList*>(u::gui::getSubwindow(G_MainWin, WID_FX_LIST));
    if (parent == nullptr)
        return;
    v::gdPluginWindow* child = static_cast<v::gdPluginWindow*>(u::gui::getSubwindow(parent, pluginId + 1));
    if (child == nullptr)
        return;

    if (!gui) Fl::lock();
    child->updateParameters(!gui);
    if (!gui) Fl::unlock();
}


/* -------------------------------------------------------------------------- */


void addPlugin(int pluginListIndex, ID channelId)
{
	if (pluginListIndex >= m::pluginManager::countAvailablePlugins())
		return;
	std::unique_ptr<m::Plugin> p = m::pluginManager::makePlugin(pluginListIndex);
	if (p != nullptr)
		m::pluginHost::addPlugin(std::move(p), channelId);
}


/* -------------------------------------------------------------------------- */


void swapPlugins(const m::Plugin& p1, const m::Plugin& p2, ID channelId)
{
	m::pluginHost::swapPlugin(p1, p2, channelId);
}


/* -------------------------------------------------------------------------- */


void freePlugin(const m::Plugin& plugin, ID channelId)
{
	m::pluginHost::freePlugin(plugin, channelId);
}


/* -------------------------------------------------------------------------- */


void setProgram(ID pluginId, int programIndex)
{
	m::pluginHost::setPluginProgram(pluginId, programIndex);
	updateWindow(pluginId, /*gui=*/true); 
}


/* -------------------------------------------------------------------------- */


void toggleBypass(ID pluginId)
{
	m::pluginHost::toggleBypass(pluginId);
}


/* -------------------------------------------------------------------------- */


void setPluginPathCb(void* data)
{
	v::gdBrowserDir* browser = (v::gdBrowserDir*) data;

	if (browser->getCurrentPath() == "") {
		v::gdAlert("Invalid path.");
		return;
	}

	if (!m::conf::conf.pluginPath.empty() && m::conf::conf.pluginPath.back() != ';')
		m::conf::conf.pluginPath += ";";
	m::conf::conf.pluginPath += browser->getCurrentPath();

	browser->do_callback();

	v::gdConfig* configWin = static_cast<v::gdConfig*>(u::gui::getSubwindow(G_MainWin, WID_CONFIG));
	configWin->refreshVstPath();
}
} // giada::c::plugin::


#endif
