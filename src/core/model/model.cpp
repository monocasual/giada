/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "utils/gui.h"
#include "utils/log.h"
#include <cassert>
#include <memory>
#ifdef G_DEBUG_MODE
#include "core/channels/channelFactory.h"
#include <fmt/core.h>
#endif
#include <fmt/ostream.h>

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

Model::Model()
: onSwap(nullptr)
{
}

/* -------------------------------------------------------------------------- */

void Model::reset()
{
	m_shared = {};

	m_layout.forEachData([this](Layout& layout) {
		layout                  = {};
		layout.sequencer.shared = &m_shared.sequencerShared;
		layout.mixer.shared     = &m_shared.mixerShared;
	});

	swap(SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

bool Model::registerThread(Thread t, bool realtime) const
{
	return m_layout.registerThread(u::gui::toString(t), realtime);
}

/* -------------------------------------------------------------------------- */

Layout&    Model::get() { return m_layout.get(); }
LayoutLock Model::get_RT() const { return LayoutLock(m_layout); }

void Model::swap(SwapType t)
{
	m_layout.swap();
	if (onSwap != nullptr)
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
	return m_layout.isRtLocked();
}

/* -------------------------------------------------------------------------- */

template <typename T>
T& Model::getAllShared()
{
	if constexpr (std::is_same_v<T, PluginPtrs>)
		return m_shared.plugins;
	if constexpr (std::is_same_v<T, WavePtrs>)
		return m_shared.waves;
	if constexpr (std::is_same_v<T, Actions::Map>)
		return m_shared.actions;
	if constexpr (std::is_same_v<T, ChannelSharedPtrs>)
		return m_shared.channelsShared;

	assert(false);
}

template PluginPtrs&        Model::getAllShared<PluginPtrs>();
template WavePtrs&          Model::getAllShared<WavePtrs>();
template Actions::Map&      Model::getAllShared<Actions::Map>();
template ChannelSharedPtrs& Model::getAllShared<ChannelSharedPtrs>();

/* -------------------------------------------------------------------------- */

template <typename T>
T* Model::findShared(ID id)
{
	if constexpr (std::is_same_v<T, Plugin>)
		return get_(m_shared.plugins, id);
	if constexpr (std::is_same_v<T, Wave>)
		return get_(m_shared.waves, id);

	assert(false);
}

template Plugin* Model::findShared<Plugin>(ID id);
template Wave*   Model::findShared<Wave>(ID id);

/* -------------------------------------------------------------------------- */

template <typename T>
void Model::addShared(T obj)
{
	if constexpr (std::is_same_v<T, PluginPtr>)
		m_shared.plugins.push_back(std::move(obj));
	if constexpr (std::is_same_v<T, WavePtr>)
		m_shared.waves.push_back(std::move(obj));
	if constexpr (std::is_same_v<T, ChannelSharedPtr>)
		m_shared.channelsShared.push_back(std::move(obj));
}

template void Model::addShared<PluginPtr>(PluginPtr p);
template void Model::addShared<WavePtr>(WavePtr p);
template void Model::addShared<ChannelSharedPtr>(ChannelSharedPtr p);

/* -------------------------------------------------------------------------- */

template <typename T>
void Model::removeShared(const T& ref)
{
	if constexpr (std::is_same_v<T, Plugin>)
		remove_(m_shared.plugins, ref);
	if constexpr (std::is_same_v<T, Wave>)
		remove_(m_shared.waves, ref);
}

template void Model::removeShared<Plugin>(const Plugin& t);
template void Model::removeShared<Wave>(const Wave& t);

/* -------------------------------------------------------------------------- */

template <typename T>
T& Model::backShared()
{
	if constexpr (std::is_same_v<T, Plugin>)
		return *m_shared.plugins.back().get();
	if constexpr (std::is_same_v<T, Wave>)
		return *m_shared.waves.back().get();
	if constexpr (std::is_same_v<T, ChannelShared>)
		return *m_shared.channelsShared.back().get();
}

template Plugin&        Model::backShared<Plugin>();
template Wave&          Model::backShared<Wave>();
template ChannelShared& Model::backShared<ChannelShared>();

/* -------------------------------------------------------------------------- */

template <typename T>
void Model::clearShared()
{
	if constexpr (std::is_same_v<T, PluginPtrs>)
		m_shared.plugins.clear();
	if constexpr (std::is_same_v<T, WavePtrs>)
		m_shared.waves.clear();
}

template void Model::clearShared<PluginPtrs>();
template void Model::clearShared<WavePtrs>();

/* -------------------------------------------------------------------------- */

#ifdef G_DEBUG_MODE

void Model::debug()
{
	puts("======== SYSTEM STATUS ========");

	puts("-------------------------------");
	m_layout.debug();
	puts("-------------------------------");

	fmt::print("thread.name={}\n", m_layout.thread.name);
	fmt::print("thread.index={}\n", m_layout.thread.index);
	fmt::print("thread.revision={}\n", m_layout.thread.revision);
	fmt::print("thread.registered={}\n", m_layout.thread.registered);
	fmt::print("thread.realtime={}\n", m_layout.thread.realtime);

	get().mixer.debug();
	get().channels.debug();

	puts("model::channelsShared");

	for (int i = 0; const auto& c : m_shared.channelsShared)
	{
		fmt::print("\t{}) - {}\n", i++, (void*)c.get());
	}

	puts("model::shared.waves");

	for (int i = 0; const auto& w : m_shared.waves)
		fmt::print("\t{}) {} - ID={} name='{}'\n", i++, (void*)w.get(), w->id, w->getPath());

	puts("model::shared.actions");

	for (const auto& [frame, actions] : getAllShared<Actions::Map>())
	{
		fmt::print("\tframe: {}\n", frame);
		for (const Action& a : actions)
			fmt::print("\t\t({}) - ID={}, frame={}, channel={}, value=0x{}, prevId={}, prev={}, nextId={}, next={}\n",
			    (void*)&a, a.id, a.frame, a.channelId, a.event.getRaw(), a.prevId, (void*)a.prev, a.nextId, (void*)a.next);
	}

	puts("model::shared.plugins");

	for (int i = 0; const auto& p : m_shared.plugins)
		fmt::print("\t{}) {} - ID={}\n", i++, (void*)p.get(), p->id);
}

#endif // G_DEBUG_MODE
} // namespace giada::m::model
