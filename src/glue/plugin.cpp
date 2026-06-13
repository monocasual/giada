/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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
namespace
{
v::gdPluginWindow* getPluginWindowByPluginId_(ID pluginId)
{
	const m::Plugin* p = g_engine->getPluginsApi().get(pluginId);

	assert(p != nullptr);

	if (p->hasEditor())
		return nullptr;
	return static_cast<v::gdPluginWindow*>(g_ui->getSubwindow(v::Ui::getPluginWindowId(pluginId)));
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Param::Param(const m::Plugin::Parameter& p, ID pluginId, ID channelId)
: index(p.index)
, pluginId(pluginId)
, channelId(channelId)
, name(p.name)
, m_parameter(p)
{
}

/* -------------------------------------------------------------------------- */

std::string Param::getValueAsText() const { return m_parameter.getValueAsText(); }
std::string Param::getLabel() const { return m_parameter.getLabel(); }
float       Param::getValue() const { return m_parameter.getValue(); }

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
	for (const m::Plugin::Parameter& param : p.getParameters())
		parameters.emplace_back(param, p.id, channelId);
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

Param getParam(std::size_t index, const m::Plugin& plugin, ID channelId)
{
	return Param(plugin.getParameters()[index], plugin.id, channelId);
}

std::vector<PluginInfo> getPluginsInfo()
{
	return g_engine->getPluginsApi().getInfo();
}

/* -------------------------------------------------------------------------- */

void updateWindow(ID pluginId, Thread t)
{
	v::gdPluginWindow* pluginWindow = getPluginWindowByPluginId_(pluginId);
	if (pluginWindow == nullptr)
		return;
	pluginWindow->updateParameters(t != Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

void updateParameter(ID pluginId, std::size_t paramIndex, Thread t)
{
	v::gdPluginWindow* pluginWindow = getPluginWindowByPluginId_(pluginId);
	if (pluginWindow == nullptr)
		return;
	pluginWindow->updateParameter(paramIndex, t != Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

void addPlugin(const std::string& juceId, ID channelId)
{
	g_engine->getPluginsApi().add(juceId, channelId);
}

/* -------------------------------------------------------------------------- */

void swapPlugins(ID pluginId1, ID pluginId2, ID channelId)
{
	g_engine->getPluginsApi().swap(pluginId1, pluginId2, channelId);
}

/* -------------------------------------------------------------------------- */

void sortPlugins(PluginSortMode mode)
{
	g_engine->getPluginsApi().sort(mode);
}

/* -------------------------------------------------------------------------- */

void freePlugin(ID pluginId, ID channelId)
{
	g_engine->getPluginsApi().free(pluginId, channelId);
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

	g_ui->pumpEvent([pluginId, paramIndex, t]()
	{ c::plugin::updateParameter(pluginId, paramIndex, t); });
}

/* -------------------------------------------------------------------------- */

void toggleBypass(ID pluginId)
{
	g_engine->getPluginsApi().toggleBypass(pluginId);
}
} // namespace giada::c::plugin