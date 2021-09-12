/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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
#ifdef G_DEBUG_MODE
#include "core/channels/channelManager.h"
#endif

using namespace mcl;

namespace giada::m::model
{
namespace
{
struct State
{
	Clock::State                                 clock;
	Mixer::State                                 mixer;
	std::vector<std::unique_ptr<channel::State>> channels;
};

struct Data
{
	std::vector<std::unique_ptr<channel::Buffer>> channels;
	std::vector<std::unique_ptr<Wave>>            waves;
	recorder::ActionMap                           actions;
#ifdef WITH_VST
	std::vector<std::unique_ptr<Plugin>> plugins;
#endif
};

/* -------------------------------------------------------------------------- */

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

std::function<void(SwapType)> onSwap_ = nullptr;

AtomicSwapper<Layout> layout;
State                 state;
Data                  data;

/* -------------------------------------------------------------------------- */

DataLock::DataLock(SwapType t)
: m_swapType(t)
{
	get().locked = true;
	swap(SwapType::NONE);
}

DataLock::~DataLock()
{
	get().locked = false;
	swap(m_swapType);
}

/* -------------------------------------------------------------------------- */

channel::Data& Layout::getChannel(ID id)
{
	return const_cast<channel::Data&>(const_cast<const Layout*>(this)->getChannel(id));
}

const channel::Data& Layout::getChannel(ID id) const
{
	auto it = std::find_if(channels.begin(), channels.end(), [id](const channel::Data& c) {
		return c.id == id;
	});
	assert(it != channels.end());
	return *it;
}

/* -------------------------------------------------------------------------- */

void init()
{
	get().clock.state = &state.clock;
	get().mixer.state = &state.mixer;
	swap(SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

Layout& get()
{
	return layout.get();
}

Lock get_RT()
{
	return Lock(layout);
}

void swap(SwapType t)
{
	layout.swap();
	if (onSwap_)
		onSwap_(t);
}

void onSwap(std::function<void(SwapType)> f)
{
	onSwap_ = f;
}

/* -------------------------------------------------------------------------- */

bool isLocked()
{
	return layout.isLocked();
}

/* -------------------------------------------------------------------------- */

template <typename T>
T& getAll()
{
#ifdef WITH_VST
	if constexpr (std::is_same_v<T, PluginPtrs>)
		return data.plugins;
#endif
	if constexpr (std::is_same_v<T, WavePtrs>)
		return data.waves;
	if constexpr (std::is_same_v<T, Actions>)
		return data.actions;
	if constexpr (std::is_same_v<T, ChannelBufferPtrs>)
		return data.channels;
	if constexpr (std::is_same_v<T, ChannelStatePtrs>)
		return state.channels;

	assert(false);
}

#ifdef WITH_VST
template PluginPtrs& getAll<PluginPtrs>();
#endif
template WavePtrs&          getAll<WavePtrs>();
template Actions&           getAll<Actions>();
template ChannelBufferPtrs& getAll<ChannelBufferPtrs>();
template ChannelStatePtrs&  getAll<ChannelStatePtrs>();

/* -------------------------------------------------------------------------- */

template <typename T>
T* find(ID id)
{
#ifdef WITH_VST
	if constexpr (std::is_same_v<T, Plugin>)
		return get_(data.plugins, id);
#endif
	if constexpr (std::is_same_v<T, Wave>)
		return get_(data.waves, id);

	assert(false);
}

#ifdef WITH_VST
template Plugin* find<Plugin>(ID id);
#endif
template Wave* find<Wave>(ID id);

/* -------------------------------------------------------------------------- */

template <typename T>
void add(T obj)
{
#ifdef WITH_VST
	if constexpr (std::is_same_v<T, PluginPtr>)
		data.plugins.push_back(std::move(obj));
#endif
	if constexpr (std::is_same_v<T, WavePtr>)
		data.waves.push_back(std::move(obj));
	if constexpr (std::is_same_v<T, ChannelBufferPtr>)
		data.channels.push_back(std::move(obj));
	if constexpr (std::is_same_v<T, ChannelStatePtr>)
		state.channels.push_back(std::move(obj));
}

#ifdef WITH_VST
template void add<PluginPtr>(PluginPtr p);
#endif
template void add<WavePtr>(WavePtr p);
template void add<ChannelBufferPtr>(ChannelBufferPtr p);
template void add<ChannelStatePtr>(ChannelStatePtr p);

/* -------------------------------------------------------------------------- */

template <typename T>
void remove(const T& ref)
{
#ifdef WITH_VST
	if constexpr (std::is_same_v<T, Plugin>)
		remove_(data.plugins, ref);
#endif
	if constexpr (std::is_same_v<T, Wave>)
		remove_(data.waves, ref);
}

#ifdef WITH_VST
template void remove<Plugin>(const Plugin& t);
#endif
template void remove<Wave>(const Wave& t);

/* -------------------------------------------------------------------------- */

template <typename T>
T& back()
{
#ifdef WITH_VST
	if constexpr (std::is_same_v<T, Plugin>)
		return *data.plugins.back().get();
#endif
	if constexpr (std::is_same_v<T, Wave>)
		return *data.waves.back().get();
	if constexpr (std::is_same_v<T, channel::State>)
		return *state.channels.back().get();
	if constexpr (std::is_same_v<T, channel::Buffer>)
		return *data.channels.back().get();
}

#ifdef WITH_VST
template Plugin& back<Plugin>();
#endif
template Wave&            back<Wave>();
template channel::State&  back<channel::State>();
template channel::Buffer& back<channel::Buffer>();

/* -------------------------------------------------------------------------- */

template <typename T>
void clear()
{
#ifdef WITH_VST
	if constexpr (std::is_same_v<T, PluginPtrs>)
		data.plugins.clear();
#endif
	if constexpr (std::is_same_v<T, WavePtrs>)
		data.waves.clear();
}

#ifdef WITH_VST
template void clear<PluginPtrs>();
#endif
template void clear<WavePtrs>();

/* -------------------------------------------------------------------------- */

#ifdef G_DEBUG_MODE

void debug()
{
	puts("======== SYSTEM STATUS ========");

	puts("model::layout");

	int i = 0;
	for (const channel::Data& c : get().channels)
	{
		printf("\t%d) - ID=%d name='%s' type=%d columnID=%d state=%p\n",
		    i++, c.id, c.name.c_str(), (int)c.type, c.columnId, (void*)&c.state);
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
	for (const auto& c : state.channels)
	{
		printf("\t%d) - %p\n", i++, (void*)c.get());
	}

	puts("model::data.waves");

	i = 0;
	for (const auto& w : data.waves)
		printf("\t%d) %p - ID=%d name='%s'\n", i++, (void*)w.get(), w->id, w->getPath().c_str());

	puts("model::data.actions");

	for (const auto& [frame, actions] : getAll<Actions>())
	{
		printf("\tframe: %d\n", frame);
		for (const Action& a : actions)
			printf("\t\t(%p) - ID=%d, frame=%d, channel=%d, value=0x%X, prevId=%d, prev=%p, nextId=%d, next=%p\n",
			    (void*)&a, a.id, a.frame, a.channelId, a.event.getRaw(), a.prevId, (void*)a.prev, a.nextId, (void*)a.next);
	}

#ifdef WITH_VST

	puts("model::data.plugins");

	i = 0;
	for (const auto& p : data.plugins)
		printf("\t%d) %p - ID=%d\n", i++, (void*)p.get(), p->id);

#endif
}

#endif // G_DEBUG_MODE
} // namespace giada::m::model
