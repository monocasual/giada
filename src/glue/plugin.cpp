/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/plugins/plugin.h"
#include "src/core/engine.h"
#include "src/core/kernelAudio.h"
#include "src/core/mixer.h"
#include "src/core/model/model.h"
#include "src/core/plugins/pluginHost.h"
#include "src/core/plugins/pluginManager.h"
#include "src/glue/channel.h"
#include "src/glue/plugin.h"
#include "src/gui/dialogs/config.h"
#include "src/gui/dialogs/mainWindow.h"
#include "src/gui/dialogs/pluginList.h"
#include "src/gui/dialogs/pluginWindow.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include <FL/Fl.H>
#include <cassert>
#include <memory>

extern giada::v::Ui*     g_ui;
extern giada::m::Engine* g_engine;

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
, juceId(p.getJuceId())
, currentProgram(p.getCurrentProgram())
, uiScaling(g_ui->getScaling())
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
	return Plugins(g_engine->getChannelsApi().get(channelId));
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
	return g_engine->getPluginsApi().getInfo();
}

/* -------------------------------------------------------------------------- */

void updateWindow(ID pluginId, Thread t)
{
	const m::Plugin* p = g_engine->getPluginsApi().get(pluginId);

	assert(p != nullptr);

	if (p->hasEditor())
		return;

	v::gdPluginWindow* pluginWindow = static_cast<v::gdPluginWindow*>(g_ui->getSubwindow(v::Ui::getPluginWindowId(pluginId)));
	if (pluginWindow == nullptr)
		return;

	pluginWindow->updateParameters(t != Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

void addPlugin(int pluginListIndex, ID channelId)
{
	g_engine->getPluginsApi().add(pluginListIndex, channelId);
}

/* -------------------------------------------------------------------------- */

void swapPlugins(const m::Plugin& p1, const m::Plugin& p2, ID channelId)
{
	g_engine->getPluginsApi().swap(p1, p2, channelId);
}

/* -------------------------------------------------------------------------- */

void sortPlugins(m::PluginManager::SortMode mode)
{
	g_engine->getPluginsApi().sort(mode);
}

/* -------------------------------------------------------------------------- */

void freePlugin(const m::Plugin& plugin, ID channelId)
{
	g_engine->getPluginsApi().free(plugin, channelId);
}

/* -------------------------------------------------------------------------- */

void setProgram(ID pluginId, int programIndex)
{
	g_engine->getPluginsApi().setProgram(pluginId, programIndex);
	updateWindow(pluginId, Thread::MAIN); // Only Main thread so far
}

/* -------------------------------------------------------------------------- */

void setParameter(ID channelId, ID pluginId, int paramIndex, float value, Thread t)
{
	g_engine->getPluginsApi().setParameter(pluginId, paramIndex, value);
	channel::notifyChannelForMidiIn(t, channelId);

	g_ui->pumpEvent([pluginId, t]()
	{ c::plugin::updateWindow(pluginId, t); });
}

/* -------------------------------------------------------------------------- */

void toggleBypass(ID pluginId)
{
	g_engine->getPluginsApi().toggleBypass(pluginId);
}
} // namespace giada::c::plugin