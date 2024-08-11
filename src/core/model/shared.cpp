/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/model/shared.h"
#include "core/channels/channelFactory.h"
#include "core/plugins/pluginFactory.h"
#include "core/plugins/pluginManager.h"
#include "core/waveFactory.h"
#include "utils/vector.h"
#ifdef G_DEBUG_MODE
#include <fmt/core.h>
#endif
#include <fmt/ostream.h>

namespace giada::m::model
{
namespace
{
template <typename T>
auto getIter_(const std::vector<std::unique_ptr<T>>& source, ID id)
{
	return u::vector::findIf(source, [id](const std::unique_ptr<T>& p)
	{ return p->id == id; });
}

/* -------------------------------------------------------------------------- */

template <typename S>
auto* get_(S& source, ID id)
{
	auto it = getIter_(source, id);
	return it == source.end() ? nullptr : it->get();
}

/* -------------------------------------------------------------------------- */

template <typename T>
typename T::element_type& add_(std::vector<T>& dest, T obj)
{
	dest.push_back(std::move(obj));
	return *dest.back().get();
}

/* -------------------------------------------------------------------------- */

template <typename D, typename T>
void remove_(D& dest, T& ref)
{
	u::vector::removeIf(dest, [&ref](const auto& other)
	{ return other.get() == &ref; });
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void Shared::init()
{
	m_sequencer = {};
	m_mixer     = {};
	m_channels.clear();
	m_waves.clear();
	m_plugins.clear();
}

/* -------------------------------------------------------------------------- */

LoadState Shared::load(const Patch& patch, PluginManager& pluginManager, const Sequencer& sequencer, int sampleRate, int bufferSize, Resampler::Quality rsmpQuality)
{
	init();

	LoadState state{patch};

	for (const Patch::Plugin& pplugin : patch.plugins)
	{
		std::unique_ptr<juce::AudioPluginInstance> pi = pluginManager.makeJucePlugin(pplugin.path, sampleRate, bufferSize);
		std::unique_ptr<Plugin>                    p  = pluginFactory::deserializePlugin(pplugin, std::move(pi), sequencer, sampleRate, bufferSize);
		if (!p->valid)
			state.missingPlugins.push_back(pplugin.path);
		getAllPlugins().push_back(std::move(p));
	}

	for (const Patch::Wave& pwave : patch.waves)
	{
		std::unique_ptr<Wave> w = waveFactory::deserializeWave(pwave, sampleRate, rsmpQuality);
		if (w != nullptr)
			getAllWaves().push_back(std::move(w));
		else
			state.missingWaves.push_back(pwave.path);
	}

	for (const Patch::Channel& pchannel : patch.channels)
	{
		std::vector<Plugin*>           plugins = findPlugins(pchannel.pluginIds);
		std::unique_ptr<ChannelShared> shared  = channelFactory::deserializeShared(pchannel, bufferSize, rsmpQuality);
		getAllChannels().push_back(std::move(shared));
	}

	return state;
}

/* -------------------------------------------------------------------------- */

void Shared::store(Patch& patch, const std::string& projectPath)
{
	for (const auto& p : getAllPlugins())
		patch.plugins.push_back(pluginFactory::serializePlugin(*p));

	for (auto& w : getAllWaves())
	{
		/* Update all existing file paths in Waves, so that they point to the
		project folder they belong to. */

		w->setPath(waveFactory::makeUniqueWavePath(projectPath, *w, getAllWaves()));
		waveFactory::save(*w, w->getPath()); // TODO - error checking

		patch.waves.push_back(waveFactory::serializeWave(*w));
	}
}

/* -------------------------------------------------------------------------- */

#ifdef G_DEBUG_MODE

void Shared::debug() const
{
	puts("shared::channels");

	for (int i = 0; const auto& c : m_channels)
		fmt::print("\t{}) - {}\n", i++, (void*)c.get());

	puts("shared::waves");

	for (int i = 0; const auto& w : m_waves)
		fmt::print("\t{}) {} - ID={} name='{}'\n", i++, (void*)w.get(), w->id, w->getPath());

	puts("shared::plugins");

	for (int i = 0; const auto& p : m_plugins)
		fmt::print("\t{}) {} - ID={}\n", i++, (void*)p.get(), p->id);
}

#endif // G_DEBUG_MODE

/* -------------------------------------------------------------------------- */

std::vector<std::unique_ptr<Wave>>&          Shared::getAllWaves() { return m_waves; };
std::vector<std::unique_ptr<Plugin>>&        Shared::getAllPlugins() { return m_plugins; }
std::vector<std::unique_ptr<ChannelShared>>& Shared::getAllChannels() { return m_channels; }

/* -------------------------------------------------------------------------- */

Plugin*        Shared::findPlugin(ID id) { return get_(m_plugins, id); }
Wave*          Shared::findWave(ID id) { return get_(m_waves, id); }
ChannelShared* Shared::findChannel(ID id) { return get_(m_channels, id); }

/* -------------------------------------------------------------------------- */

Wave&          Shared::addWave(std::unique_ptr<Wave> w) { return add_(m_waves, std::move(w)); }
Plugin&        Shared::addPlugin(std::unique_ptr<Plugin> p) { return add_(m_plugins, std::move(p)); }
ChannelShared& Shared::addChannel(std::unique_ptr<ChannelShared> cs) { return add_(m_channels, std::move(cs)); }

/* -------------------------------------------------------------------------- */

void Shared::removePlugin(const Plugin& p) { remove_(m_plugins, p); }
void Shared::removeWave(const Wave& w) { remove_(m_waves, w); }

/* -------------------------------------------------------------------------- */

void Shared::clearPlugins() { m_plugins.clear(); }
void Shared::clearWaves() { m_waves.clear(); }

/* -------------------------------------------------------------------------- */

std::vector<Plugin*> Shared::findPlugins(std::vector<ID> pluginIds)
{
	std::vector<Plugin*> out;
	for (ID id : pluginIds)
	{
		Plugin* plugin = findPlugin(id);
		if (plugin != nullptr)
			out.push_back(plugin);
	}
	return out;
}
} // namespace giada::m::model
