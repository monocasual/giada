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

#ifndef G_PLUGINS_ENGINE_H
#define G_PLUGINS_ENGINE_H

#include "core/plugins/pluginManager.h"
#include "core/types.h"

namespace giada::m::model
{
class Model;
}

namespace giada::m
{
class MidiEvent;
class Engine;
class KernelAudio;
class ChannelManager;
class PluginHost;
class Plugin;
class PluginsEngine
{
public:
	PluginsEngine(KernelAudio&, ChannelManager&, PluginManager&, PluginHost&, model::Model&);

	const Plugin*                          get(ID pluginId) const;
	std::vector<PluginManager::PluginInfo> getInfo() const;

	void add(int pluginListIndex, ID channelId);
	void swap(const Plugin&, const Plugin&, ID channelId);
	void sort(PluginManager::SortMethod);
	void free(const Plugin&, ID channelId);
	void setProgram(ID pluginId, int programIndex);
	void toggleBypass(ID pluginId);
	void setParameter(ID pluginId, int paramIndex, float value);

private:
	KernelAudio&    m_kernelAudio;
	ChannelManager& m_channelManager;
	PluginManager&  m_pluginManager;
	PluginHost&     m_pluginHost;
	model::Model&   m_model;
};
} // namespace giada::m

#endif
