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

#ifndef G_MODEL_H
#define G_MODEL_H

#include "core/channels/channel.h"
#include "core/const.h"
#include "core/model/behaviors.h"
#include "core/model/channels.h"
#include "core/model/kernelAudio.h"
#include "core/model/kernelMidi.h"
#include "core/model/midiIn.h"
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
struct Layout
{
#ifdef G_DEBUG_MODE
	void debug() const;
#endif

	/* locked
	If locked, Mixer won't process channels. This is used to allow editing the 
	shared data (e.g. Plugins, Waves) by the rendering engine without data races. */

	bool locked = false;

	KernelAudio kernelAudio;
	KernelMidi  kernelMidi;
	Sequencer   sequencer;
	Mixer       mixer;
	MidiIn      midiIn;
	Channels    channels;
	Behaviors   behaviors;
};

/* LayoutLock
Alias for a REALTIME scoped lock provided by the Swapper class. Use this in the
real-time thread to lock the Layout. */

using AtomicSwapper = mcl::AtomicSwapper<Layout, /*size=*/6>;
using LayoutLock    = AtomicSwapper::RtLock;

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

/* LoadState
Contains information about the model state after a patch has been loaded. */

struct LoadState
{
	bool isGood() const;

	Patch                    patch;
	std::vector<std::string> missingWaves   = {};
	std::vector<std::string> missingPlugins = {};
};

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

	/* init
	Initializes the internal layout. All values go back to default. */

	void init();

	/* reset
	Resets the internal layout to default. Configuration data (e.g. KernelAudio)
	are left untouched.  */

	void reset();

	/* load (1)
	Loads data from a Conf object. */

	void load(const Conf&);

	/* load (2) 
	Loads data from a Patch object. */

	LoadState load(const Patch&, PluginManager&, int sampleRate, int bufferSize, Resampler::Quality);

	/* store
	Stores data into a Conf object. */

	void store(Conf&) const;

	/* store
	Stores data into a Patch object. */

	void store(Patch&, const std::string& projectPath);

	bool registerThread(Thread, bool realtime) const;

	/* get_RT
	Returns a LayoutLock object for REALTIME processing. Access layout by 
	calling LayoutLock::get() method (returns ready-only Layout). */

	LayoutLock get_RT() const;

	/* get
	Returns a reference to the NON-REALTIME layout structure. */

	Layout&       get();
	const Layout& get() const;

	void set(const Layout&);

	/* swap
	Swap non-rt layout with the rt one. See 'SwapType' notes above. */

	void swap(SwapType t);

	/* getAll[*] */

	std::vector<std::unique_ptr<Wave>>&          getAllWaves();
	std::vector<std::unique_ptr<Plugin>>&        getAllPlugins();
	Actions::Map&                                getAllActions();
	std::vector<std::unique_ptr<ChannelShared>>& getAllChannelsShared();

	/* find[*]
	Finds something in the shared data given an ID. Returns nullptr if the
	object is not found. */

	Plugin* findPlugin(ID);
	Wave*   findWave(ID);

	/* addShared
	Adds some shared data (by moving it). Returns a reference to the last added 
	shared item. */

	template <typename T>
	typename T::element_type& addShared(T);

	Wave& addWave(std::unique_ptr<Wave>);

	template <typename T>
	void removeShared(const T&);

	template <typename T>
	void clearShared();

#ifdef G_DEBUG_MODE
	void debug();
#endif

	/* onSwap
	Callbacks fired when the layout has been swapped. Useful for listening to 
	model changes. */

	std::function<void(SwapType)> onSwap;

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

	std::vector<Plugin*> findPlugins(std::vector<ID> pluginIds);

	AtomicSwapper m_swapper;
	Shared        m_shared;
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
