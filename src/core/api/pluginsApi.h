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

#ifndef G_PLUGINS_API_H
#define G_PLUGINS_API_H

#include "src/core/plugins/pluginManager.h"
#include "src/core/types.h"

namespace giada::m::model
{
class Model;
}

namespace giada::m
{
class MidiEvent;

class KernelAudio;
class ChannelManager;
class PluginHost;
class Plugin;
class PluginsApi
{
public:
	PluginsApi(KernelAudio&, PluginManager&, PluginHost&, model::Model&);

	const Plugin*           get(ID pluginId) const;
	std::vector<PluginInfo> getInfo() const;
	int                     countAvailablePlugins() const;

	void add(const std::string& juceId, ID channelId);
	void swap(const Plugin&, const Plugin&, ID channelId);
	void sort(PluginSortMode);
	void free(const Plugin&, ID channelId);
	void setProgram(ID pluginId, int programIndex);
	void toggleBypass(ID pluginId);
	void setParameter(ID pluginId, int paramIndex, float value);

	void scan(const std::string& dir, const std::function<bool(float)>& progress);
	void process(mcl::AudioBuffer& outBuf, const std::vector<Plugin*>&, juce::MidiBuffer* events = nullptr);

private:
	KernelAudio&   m_kernelAudio;
	PluginManager& m_pluginManager;
	PluginHost&    m_pluginHost;
	model::Model&  m_model;
};
} // namespace giada::m

#endif
