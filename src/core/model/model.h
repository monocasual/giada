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

#ifndef G_MODEL_H
#define G_MODEL_H

#include "core/channels/channel.h"
#include "core/const.h"
#include "core/model/mixer.h"
#include "core/model/recorder.h"
#include "core/model/sequencer.h"
#include "core/plugins/plugin.h"
#include "core/recorder.h"
#include "core/wave.h"
#include "deps/mcl-atomic-swapper/src/atomic-swapper.hpp"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "src/core/actions/actions.h"
#include "utils/vector.h"

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
	channel::Data&       getChannel(ID id);
	const channel::Data& getChannel(ID id) const;

	Sequencer                  sequencer;
	Mixer                      mixer;
	Recorder                   recorder;
	MidiIn                     midiIn;
	std::vector<channel::Data> channels;

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

#ifdef WITH_VST
using PluginPtr = std::unique_ptr<Plugin>;
#endif
using WavePtr          = std::unique_ptr<Wave>;
using ChannelBufferPtr = std::unique_ptr<channel::Buffer>;
using ChannelStatePtr  = std::unique_ptr<channel::State>;

#ifdef WITH_VST
using PluginPtrs = std::vector<PluginPtr>;
#endif
using WavePtrs          = std::vector<WavePtr>;
using ChannelBufferPtrs = std::vector<ChannelBufferPtr>;
using ChannelStatePtrs  = std::vector<ChannelStatePtr>;

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

	// TODO - are ID-based objects still necessary?

	template <typename T>
	T& getAll();

	/* find
	Finds something (Plugins or Waves) given an ID. Returns nullptr if the 
	object is not found. */

	template <typename T>
	T* find(ID id);

	/* add
	Adds something (by moving it). */

	template <typename T>
	void add(T);

	template <typename T>
	void remove(const T&);

	template <typename T>
	T& back();

	template <typename T>
	void clear();

#ifdef G_DEBUG_MODE
	void debug();
#endif

	/* onSwap
	Optional callback fired when the layout has been swapped. Useful for 
	listening to model changes. */

	std::function<void(SwapType)> onSwap = nullptr;

private:
	struct States
	{
		Sequencer::State                             sequencer;
		Mixer::State                                 mixer;
		Recorder::State                              recorder;
		std::vector<std::unique_ptr<channel::State>> channels;
	};

	struct Buffers
	{
		Mixer::Buffer mixer;
	};

	struct Data
	{
		std::vector<std::unique_ptr<channel::Buffer>> channels;
		std::vector<std::unique_ptr<Wave>>            waves;
		Actions::Map                                  actions;
#ifdef WITH_VST
		std::vector<std::unique_ptr<Plugin>> plugins;
#endif
	};

	mcl::AtomicSwapper<Layout> m_layout;
	States                     m_states;
	Buffers                    m_buffers;
	Data                       m_data;
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
