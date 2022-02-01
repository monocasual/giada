/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/plugins/plugin.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/engine.h"
#include "core/kernelAudio.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "gui/dialogs/config.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/pluginList.h"
#include "gui/dialogs/pluginWindow.h"
#include "gui/ui.h"
#include "plugin.h"
#include "utils/gui.h"
#include <FL/Fl.H>
#include <cassert>
#include <memory>

extern giada::v::Ui     g_ui;
extern giada::m::Engine g_engine;

namespace giada::c::plugin
{
Param::Param(const m::Plugin& p, int index, ID channelId)
: index(index)
, pluginId(p.id)
, channelId(channelId)
, name(p.getParameterName(index))
, text(p.getParameterText(index))
, label(p.getParameterLabel(index))
, value(p.getParameter(index))
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Plugin::Plugin(m::Plugin& p, ID channelId)
: id(p.id)
, channelId(channelId)
, valid(p.valid)
, hasEditor(p.hasEditor())
, isBypassed(p.isBypassed())
, name(p.getName())
, uniqueId(p.getUniqueId())
, currentProgram(p.getCurrentProgram())
, m_plugin(p)
{
	for (int i = 0; i < p.getNumPrograms(); i++)
		programs.push_back({i, p.getProgramName(i)});
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

Plugins::Plugins(const m::Channel& c)
: channelId(c.id)
, plugins(c.plugins)
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Plugins getPlugins(ID channelId)
{
	return Plugins(g_engine.model.get().getChannel(channelId));
}

Plugin getPlugin(m::Plugin& plugin, ID channelId)
{
	return Plugin(plugin, channelId);
}

Param getParam(int index, const m::Plugin& plugin, ID channelId)
{
	return Param(plugin, index, channelId);
}

std::vector<m::PluginManager::PluginInfo> getPluginsInfo()
{
	return g_engine.pluginManager.getPluginsInfo();
}

/* -------------------------------------------------------------------------- */

void updateWindow(ID pluginId, bool gui)
{
	m::Plugin* p = g_engine.model.findShared<m::Plugin>(pluginId);

	assert(p != nullptr);

	if (p->hasEditor())
		return;

	/* Get the parent window first: the plug-in list. Then, if it exists, get
    the child window - the actual pluginWindow. */

	v::gdPluginList* parent = static_cast<v::gdPluginList*>(g_ui.getSubwindow(*g_ui.mainWindow.get(), WID_FX_LIST));
	if (parent == nullptr)
		return;
	v::gdPluginWindow* child = static_cast<v::gdPluginWindow*>(g_ui.getSubwindow(*parent, pluginId + 1));
	if (child == nullptr)
		return;

	if (!gui)
		Fl::lock();
	child->updateParameters(!gui);
	if (!gui)
		Fl::unlock();
}

/* -------------------------------------------------------------------------- */

void addPlugin(int pluginListIndex, ID channelId)
{
	if (pluginListIndex >= g_engine.pluginManager.countAvailablePlugins())
		return;
	std::unique_ptr<m::Plugin> plugin    = g_engine.pluginManager.makePlugin(pluginListIndex, g_engine.kernelAudio.getSampleRate(), g_engine.kernelAudio.getBufferSize(), g_engine.sequencer);
	const m::Plugin*           pluginPtr = plugin.get();
	if (plugin != nullptr)
		g_engine.pluginHost.addPlugin(std::move(plugin));

	/* TODO - unfortunately JUCE wants mutable plugin objects due to the
	presence of the non-const processBlock() method. Why not const_casting
	only in the Plugin class? */
	g_engine.model.get().getChannel(channelId).plugins.push_back(const_cast<m::Plugin*>(pluginPtr));
	g_engine.model.swap(m::model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void swapPlugins(const m::Plugin& p1, const m::Plugin& p2, ID channelId)
{
	g_engine.pluginHost.swapPlugin(p1, p2, g_engine.model.get().getChannel(channelId).plugins);
	g_engine.model.swap(m::model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void sortPlugins(m::PluginManager::SortMethod method)
{
	g_engine.pluginManager.sortPlugins(method);
}

/* -------------------------------------------------------------------------- */

void freePlugin(const m::Plugin& plugin, ID channelId)
{
	u::vector::remove(g_engine.model.get().getChannel(channelId).plugins, &plugin);
	g_engine.model.swap(m::model::SwapType::HARD);

	g_engine.pluginHost.freePlugin(plugin);
}

/* -------------------------------------------------------------------------- */

void setProgram(ID pluginId, int programIndex)
{
	g_engine.pluginHost.setPluginProgram(pluginId, programIndex);
	updateWindow(pluginId, /*gui=*/true);
}

/* -------------------------------------------------------------------------- */

void toggleBypass(ID pluginId)
{
	g_engine.pluginHost.toggleBypass(pluginId);
}

/* -------------------------------------------------------------------------- */

void startDispatchLoop()
{
	g_ui.startJuceDispatchLoop();
}

void stopDispatchLoop()
{
	g_ui.stopJuceDispatchLoop();
}
} // namespace giada::c::plugin

#endif
