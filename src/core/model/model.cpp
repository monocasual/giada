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

#include "core/model/model.h"
#include <cassert>
#include <memory>
#ifdef G_DEBUG_MODE
#include "core/channels/channelManager.h"
#endif

using namespace mcl;

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

template <typename D, typename T>
void remove_(D& dest, T& ref)
{
	u::vector::removeIf(dest, [&ref](const auto& other) { return other.get() == &ref; });
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

DataLock::DataLock(Model& m, SwapType t)
: m_model(m)
, m_swapType(t)
{
	m_model.get().locked = true;
	m_model.swap(SwapType::NONE);
}

DataLock::~DataLock()
{
	m_model.get().locked = false;
	m_model.swap(m_swapType);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Channel& Layout::getChannel(ID id)
{
	return const_cast<Channel&>(const_cast<const Layout*>(this)->getChannel(id));
}

const Channel& Layout::getChannel(ID id) const
{
	auto it = std::find_if(channels.begin(), channels.end(), [id](const Channel& c) {
		return c.id == id;
	});
	assert(it != channels.end());
	return *it;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Model::Model()
: onSwap(nullptr)
{
	reset();
}

/* -------------------------------------------------------------------------- */

void Model::reset()
{
	m_shared = {};

	get().sequencer.shared = &m_shared.sequencerShared;
	get().mixer.shared     = &m_shared.mixerShared;
	get().recorder.shared  = &m_shared.recorderShared;

	swap(SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

Layout&       Model::get() { return m_layout.get(); }
const Layout& Model::get() const { return m_layout.get(); }

LayoutLock Model::get_RT()
{
	return LayoutLock(m_layout);
}

void Model::swap(SwapType t)
{
	m_layout.swap();
	if (onSwap)
		onSwap(t);
}

/* -------------------------------------------------------------------------- */

DataLock Model::lockData(SwapType t)
{
	return DataLock(*this, t);
}

/* -------------------------------------------------------------------------- */

bool Model::isLocked() const
{
	return m_layout.isLocked();
}

/* -------------------------------------------------------------------------- */

template <typename T>
T& Model::getAllShared()
{
#ifdef WITH_VST
	if constexpr (std::is_same_v<T, PluginPtrs>)
		return m_shared.plugins;
#endif
	if constexpr (std::is_same_v<T, WavePtrs>)
		return m_shared.waves;
	if constexpr (std::is_same_v<T, Actions::Map>)
		return m_shared.actions;
	if constexpr (std::is_same_v<T, ChannelSharedPtrs>)
		return m_shared.channelsShared;

	assert(false);
}

#ifdef WITH_VST
template PluginPtrs& Model::getAllShared<PluginPtrs>();
#endif
template WavePtrs&          Model::getAllShared<WavePtrs>();
template Actions::Map&      Model::getAllShared<Actions::Map>();
template ChannelSharedPtrs& Model::getAllShared<ChannelSharedPtrs>();

/* -------------------------------------------------------------------------- */

template <typename T>
T* Model::findShared(ID id)
{
#ifdef WITH_VST
	if constexpr (std::is_same_v<T, Plugin>)
		return get_(m_shared.plugins, id);
#endif
	if constexpr (std::is_same_v<T, Wave>)
		return get_(m_shared.waves, id);

	assert(false);
}

#ifdef WITH_VST
template Plugin* Model::findShared<Plugin>(ID id);
#endif
template Wave* Model::findShared<Wave>(ID id);

/* -------------------------------------------------------------------------- */

template <typename T>
void Model::addShared(T obj)
{
#ifdef WITH_VST
	if constexpr (std::is_same_v<T, PluginPtr>)
		m_shared.plugins.push_back(std::move(obj));
#endif
	if constexpr (std::is_same_v<T, WavePtr>)
		m_shared.waves.push_back(std::move(obj));
	if constexpr (std::is_same_v<T, ChannelSharedPtr>)
		m_shared.channelsShared.push_back(std::move(obj));
}

#ifdef WITH_VST
template void Model::addShared<PluginPtr>(PluginPtr p);
#endif
template void Model::addShared<WavePtr>(WavePtr p);
template void Model::addShared<ChannelSharedPtr>(ChannelSharedPtr p);

/* -------------------------------------------------------------------------- */

template <typename T>
void Model::removeShared(const T& ref)
{
#ifdef WITH_VST
	if constexpr (std::is_same_v<T, Plugin>)
		remove_(m_shared.plugins, ref);
#endif
	if constexpr (std::is_same_v<T, Wave>)
		remove_(m_shared.waves, ref);
}

#ifdef WITH_VST
template void Model::removeShared<Plugin>(const Plugin& t);
#endif
template void Model::removeShared<Wave>(const Wave& t);

/* -------------------------------------------------------------------------- */

template <typename T>
T& Model::backShared()
{
#ifdef WITH_VST
	if constexpr (std::is_same_v<T, Plugin>)
		return *m_shared.plugins.back().get();
#endif
	if constexpr (std::is_same_v<T, Wave>)
		return *m_shared.waves.back().get();
	if constexpr (std::is_same_v<T, ChannelShared>)
		return *m_shared.channelsShared.back().get();
}

#ifdef WITH_VST
template Plugin& Model::backShared<Plugin>();
#endif
template Wave&          Model::backShared<Wave>();
template ChannelShared& Model::backShared<ChannelShared>();

/* -------------------------------------------------------------------------- */

template <typename T>
void Model::clearShared()
{
#ifdef WITH_VST
	if constexpr (std::is_same_v<T, PluginPtrs>)
		m_shared.plugins.clear();
#endif
	if constexpr (std::is_same_v<T, WavePtrs>)
		m_shared.waves.clear();
}

#ifdef WITH_VST
template void Model::clearShared<PluginPtrs>();
#endif
template void Model::clearShared<WavePtrs>();

/* -------------------------------------------------------------------------- */

#ifdef G_DEBUG_MODE

void Model::debug()
{
	puts("======== SYSTEM STATUS ========");

	puts("model::layout.channels");

	int i = 0;
	for (const Channel& c : get().channels)
	{
		printf("\t%d) - ID=%d name='%s' type=%d columnID=%d shared=%p\n",
		    i++, c.id, c.name.c_str(), (int)c.type, c.columnId, (void*)&c.shared);
#ifdef WITH_VST
		if (c.plugins.size() > 0)
		{
			puts("\t\tplugins:");
			for (const auto& p : c.plugins)
				printf("\t\t\t%p - ID=%d\n", (void*)p, p->id);
		}
#endif
	}

	puts("model::state.channels");

	i = 0;
	for (const auto& c : m_shared.channelsShared)
	{
		printf("\t%d) - %p\n", i++, (void*)c.get());
	}

	puts("model::data.waves");

	i = 0;
	for (const auto& w : m_shared.waves)
		printf("\t%d) %p - ID=%d name='%s'\n", i++, (void*)w.get(), w->id, w->getPath().c_str());

	puts("model::data.actions");

	for (const auto& [frame, actions] : getAllShared<Actions::Map>())
	{
		printf("\tframe: %d\n", frame);
		for (const Action& a : actions)
			printf("\t\t(%p) - ID=%d, frame=%d, channel=%d, value=0x%X, prevId=%d, prev=%p, nextId=%d, next=%p\n",
			    (void*)&a, a.id, a.frame, a.channelId, a.event.getRaw(), a.prevId, (void*)a.prev, a.nextId, (void*)a.next);
	}

#ifdef WITH_VST

	puts("model::data.plugins");

	i = 0;
	for (const auto& p : m_shared.plugins)
		printf("\t%d) %p - ID=%d\n", i++, (void*)p.get(), p->id);

#endif
}

#endif // G_DEBUG_MODE
} // namespace giada::m::model
