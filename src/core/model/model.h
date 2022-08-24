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

#ifndef G_MODEL_H
#define G_MODEL_H

#include "core/channels/channel.h"
#include "core/const.h"
#include "core/model/mixer.h"
#include "core/model/sequencer.h"
#include "core/plugins/plugin.h"
#include "core/wave.h"
#include "deps/mcl-atomic-swapper/src/atomic-swapper.hpp"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "src/core/actions/actions.h"
#include "utils/vector.h"
#include <memory>

namespace giada::m::model
{
struct MidiIn
{
	bool     enabled    = false;
	int      filter     = -1;
	uint32_t rewind     = 0x0;
	uint32_t startStop  = 0x0;
	uint32_t actionRec  = 0x0;
	uint32_t inputRec   = 0x0;
	uint32_t volumeIn   = 0x0;
	uint32_t volumeOut  = 0x0;
	uint32_t beatDouble = 0x0;
	uint32_t beatHalf   = 0x0;
	uint32_t metronome  = 0x0;
};

struct Layout
{
	Channel&       getChannel(ID id);
	const Channel& getChannel(ID id) const;

	Sequencer            sequencer;
	Mixer                mixer;
	MidiIn               midiIn;
	std::vector<Channel> channels;

	/* locked
	If locked, Mixer won't process channels. This is used to allow editing the 
	data (e.g. Actions or Plugins) a channel points to without data races. */

	bool locked = false;
};

/* LayoutLock
Alias for a REALTIME scoped lock provided by the Swapper class. Use this in the
real-time thread to lock the Layout. */

using LayoutLock = mcl::AtomicSwapper<Layout>::RtLock;

/* SwapType
Type of Layout change. 
	Hard: the structure has changed (e.g. add a new channel);
	Soft: a property has changed (e.g. change volume);
	None: something has changed but we don't care. 
Used by model listeners to determine the type of change that occurred in the 
layout. */

enum class SwapType
{
	HARD,
	SOFT,
	NONE
};

using PluginPtr        = std::unique_ptr<Plugin>;
using WavePtr          = std::unique_ptr<Wave>;
using ChannelSharedPtr = std::unique_ptr<ChannelShared>;

using PluginPtrs        = std::vector<PluginPtr>;
using WavePtrs          = std::vector<WavePtr>;
using ChannelSharedPtrs = std::vector<ChannelSharedPtr>;

/* -------------------------------------------------------------------------- */

class DataLock;
class Model
{
public:
	Model();

	bool isLocked() const;

	/* lockData
	Returns a scoped locker DataLock object. Use this when you want to lock
	the model: a locked model won't be processed by Mixer. */

	[[nodiscard]] DataLock lockData(SwapType t = SwapType::HARD);

	/* reser
	Resets the internal layout to default. */

	void reset();

	/* get_RT
	Returns a LayoutLock object for REALTIME processing. Access layout by 
	calling LayoutLock::get() method (returns ready-only Layout). */

	LayoutLock get_RT();

	/* get
	Returns a reference to the NON-REALTIME layout structure. */

	Layout&       get();
	const Layout& get() const;

	/* swap
	Swap non-rt layout with the rt one. See 'SwapType' notes above. */

	void swap(SwapType t);

	template <typename T>
	T& getAllShared();

	/* findShared
	Finds something in the shared data given an ID. Returns nullptr if the
	object is not found. */

	template <typename T>
	T* findShared(ID id);

	/* addShared
	Adds some shared data (by moving it). */

	template <typename T>
	void addShared(T);

	template <typename T>
	void removeShared(const T&);

	/* backShared
	Returns a reference to the last added shared item. */

	template <typename T>
	T& backShared();

	template <typename T>
	void clearShared();

#ifdef G_DEBUG_MODE
	void debug();
#endif

	/* onSwap
	Optional callback fired when the layout has been swapped. Useful for 
	listening to model changes. */

	std::function<void(SwapType)> onSwap = nullptr;

private:
	struct Shared
	{
		Sequencer::Shared                           sequencerShared;
		Mixer::Shared                               mixerShared;
		std::vector<std::unique_ptr<ChannelShared>> channelsShared;

		std::vector<std::unique_ptr<Wave>>   waves;
		Actions::Map                         actions;
		std::vector<std::unique_ptr<Plugin>> plugins;
	};

	mcl::AtomicSwapper<Layout> m_layout;
	Shared                     m_shared;
};

/* -------------------------------------------------------------------------- */

class DataLock
{
public:
	DataLock(Model&, SwapType t);
	~DataLock();

private:
	Model&   m_model;
	SwapType m_swapType;
};
} // namespace giada::m::model

#endif
