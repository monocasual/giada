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
	return u::vector::findIf(source, [id](const std::unique_ptr<T>& p) { return p->id == id; });
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
	u::vector::removeIf(dest, [&ref](const auto& other) { return other.get() == &ref; });
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void Shared::init()
{
	sequencer = {};
	mixer     = {};
	channels.clear();
	waves.clear();
	plugins.clear();
}

/* -------------------------------------------------------------------------- */

#ifdef G_DEBUG_MODE

void Shared::debug() const
{
	puts("shared::channels");

	for (int i = 0; const auto& c : channels)
		fmt::print("\t{}) - {}\n", i++, (void*)c.get());

	puts("shared::waves");

	for (int i = 0; const auto& w : waves)
		fmt::print("\t{}) {} - ID={} name='{}'\n", i++, (void*)w.get(), w->id, w->getPath());

	puts("shared::plugins");

	for (int i = 0; const auto& p : plugins)
		fmt::print("\t{}) {} - ID={}\n", i++, (void*)p.get(), p->id);
}

#endif // G_DEBUG_MODE

/* -------------------------------------------------------------------------- */

std::vector<std::unique_ptr<Wave>>&          Shared::getAllWaves() { return waves; };
std::vector<std::unique_ptr<Plugin>>&        Shared::getAllPlugins() { return plugins; }
std::vector<std::unique_ptr<ChannelShared>>& Shared::getAllChannels() { return channels; }

/* -------------------------------------------------------------------------- */

Plugin*        Shared::findPlugin(ID id) { return get_(plugins, id); }
Wave*          Shared::findWave(ID id) { return get_(waves, id); }
ChannelShared* Shared::findChannel(ID id) { return get_(channels, id); }

/* -------------------------------------------------------------------------- */

Wave&          Shared::addWave(std::unique_ptr<Wave> w) { return add_(waves, std::move(w)); }
Plugin&        Shared::addPlugin(std::unique_ptr<Plugin> p) { return add_(plugins, std::move(p)); }
ChannelShared& Shared::addChannel(std::unique_ptr<ChannelShared> cs) { return add_(channels, std::move(cs)); }

/* -------------------------------------------------------------------------- */

void Shared::removePlugin(const Plugin& p) { remove_(plugins, p); }
void Shared::removeWave(const Wave& w) { remove_(waves, w); }

/* -------------------------------------------------------------------------- */

void Shared::clearPlugins() { plugins.clear(); }
void Shared::clearWaves() { waves.clear(); }

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
