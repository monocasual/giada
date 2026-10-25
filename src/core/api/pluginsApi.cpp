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

#include "src/core/api/pluginsApi.h"
#include "src/core/channels/channelManager.h"
#include "src/core/engine.h"
#include "src/core/kernelAudio.h"
#include "src/core/mixer.h"
#include "src/core/plugins/pluginFactory.h"
#include "src/deps/mcl-utils/src/container.hpp"
#include "src/deps/mcl-utils/src/fs.hpp"
#include "src/utils/fs.h"

namespace utils = mcl::utils;

namespace giada::m
{
PluginsApi::PluginsApi(model::Model& m, PluginManager& pm, PluginHost& ph, model_DEPR_::Model& m_DEPR_)
: m_model(m)
, m_pluginManager(pm)
, m_pluginHost(ph)
, m_model_DEPR_(m_DEPR_)
{
}

/* -------------------------------------------------------------------------- */

const Plugin* PluginsApi::get(ID pluginId) const
{
	return m_model_DEPR_.findPlugin(pluginId);
}

/* -------------------------------------------------------------------------- */

std::vector<PluginInfo> PluginsApi::getInfo() const
{
	return m_pluginManager.getPluginsInfo();
}

/* -------------------------------------------------------------------------- */

int PluginsApi::countAvailablePlugins() const
{
	return m_pluginManager.countAvailablePlugins();
}

/* -------------------------------------------------------------------------- */

void PluginsApi::add(const std::string& juceId, ID channelId)
{
	m_model.writeDocumentAndAssets(model::SwapType::HARD,
	    [this, juceId, channelId](model::Document& doc, model::Assets& assets)
	{
		const int sampleRate = doc.kernelAudio.samplerate;
		const int bufferSize = doc.kernelAudio.buffersize;

		std::shared_ptr<m::model::Plugin> plugin   = m_pluginManager.makePlugin(juceId, sampleRate, bufferSize);
		const ID                          pluginId = plugin->id;
		if (pluginId.isValid())
			assets.plugins.set(pluginId, std::move(plugin));

		doc.tracks.getChannel(channelId).plugins.push_back(pluginId);
	});
}

/* -------------------------------------------------------------------------- */

void PluginsApi::swap(ID pluginId1, ID pluginId2, ID channelId)
{
	auto&             plugins = m_model_DEPR_.get().tracks.getChannel(channelId).plugins;
	const std::size_t index1  = utils::container::indexOfIf(plugins, [pluginId1](const Plugin* p)
	 { return p->id == pluginId1; });
	const std::size_t index2  = utils::container::indexOfIf(plugins, [pluginId2](const Plugin* p)
	 { return p->id == pluginId2; });
	std::swap(plugins.at(index1), plugins.at(index2));
	m_model_DEPR_.swap(model_DEPR_::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void PluginsApi::sort(PluginSortMode mode)
{
	m_pluginManager.sortPlugins(mode);
}

/* -------------------------------------------------------------------------- */

void PluginsApi::free(ID pluginId, ID channelId)
{
	utils::container::removeIf(m_model_DEPR_.get().tracks.getChannel(channelId).plugins, [pluginId](const m::Plugin* p)
	{
		return p->id == pluginId;
	});
	m_model_DEPR_.swap(model_DEPR_::SwapType::HARD);
	m_pluginHost.freePlugin(pluginId);
}

/* -------------------------------------------------------------------------- */

void PluginsApi::setProgram(ID pluginId, int programIndex)
{
	m_pluginHost.setPluginProgram(pluginId, programIndex);
}

/* -------------------------------------------------------------------------- */

void PluginsApi::toggleBypass(ID pluginId)
{
	m_pluginHost.toggleBypass(pluginId);
}

/* -------------------------------------------------------------------------- */

void PluginsApi::setParameter(ID pluginId, int paramIndex, float value)
{
	m_pluginHost.setPluginParameter(pluginId, paramIndex, value);
}

/* -------------------------------------------------------------------------- */

void PluginsApi::scan(const std::string& dir, const std::function<bool(float)>& progress)
{
	m_pluginManager.scanDirs(dir, progress);
	m_pluginManager.saveList(utils::fs::join(u::fs::getConfigDirPath(), "plugins.xml"));
}
} // namespace giada::m
