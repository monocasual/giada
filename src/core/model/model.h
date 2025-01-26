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

#ifndef G_MODEL_H
#define G_MODEL_H

#include "core/channels/channel.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/model/actions.h"
#include "core/model/behaviors.h"
#include "core/model/channels.h"
#include "core/model/kernelAudio.h"
#include "core/model/kernelMidi.h"
#include "core/model/loadState.h"
#include "core/model/midiIn.h"
#include "core/model/mixer.h"
#include "core/model/sequencer.h"
#include "core/model/shared.h"
#include "core/model/sharedLock.h"
#include "core/model/types.h"
#include "core/plugins/plugin.h"
#include "core/wave.h"
#include "deps/mcl-atomic-swapper/src/atomic-swapper.hpp"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "utils/vector.h"
#include <memory>

namespace giada::m::model
{
struct Document;
class Model
{
public:
	Model();

	/* isRtLocked
	Returns true if the realtime thread has its own copy of data locked down,
	as it's reading it. */

	bool isRtLocked() const;

	/* lockShared
	Returns a scoped locker SharedLock object. Use this when you want to lock
	the shared data: it won't be processed by Mixer. */

	[[nodiscard]] SharedLock lockShared(SwapType t = SwapType::HARD);

	/* init
	Initializes the internal Document. All values go back to default. */

	void init();

	/* reset
	Resets the internal Document to default. Configuration data (e.g. KernelAudio)
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
	Returns a DocumentLock object for REALTIME processing. Access Document by
	calling DocumentLock::get() method (returns ready-only Document). */

	DocumentLock get_RT() const;

	/* get
	Returns a reference to the NON-REALTIME Document structure. */

	Document&       get();
	const Document& get() const;

	/* swap
	Swap non-rt Document with the rt one. See 'SwapType' notes above. */

	void swap(SwapType t);

	/* getAll[*] */

	std::vector<std::unique_ptr<Wave>>&          getAllWaves();
	std::vector<std::unique_ptr<Plugin>>&        getAllPlugins();
	std::vector<std::unique_ptr<ChannelShared>>& getAllChannelsShared();

	/* find[*]
	Finds something in the shared data given an ID. Returns nullptr if the
	object is not found. */

	Plugin* findPlugin(ID);
	Wave*   findWave(ID);

	/* add[*]
	Adds some shared data (by moving it). Returns a reference to the last added
	shared item. */

	Wave&          addWave(std::unique_ptr<Wave>);
	Plugin&        addPlugin(std::unique_ptr<Plugin>);
	ChannelShared& addChannelShared(std::unique_ptr<ChannelShared>);

	void removePlugin(const Plugin&);
	void removeWave(const Wave&);
	void removeChannelShared(const ChannelShared&);

	void clearPlugins();
	void clearWaves();

#ifdef G_DEBUG_MODE
	void debug();
#endif

	/* onSwap
	Callbacks fired when the Document has been swapped. Useful for listening to
	model changes. */

	std::function<void(SwapType)> onSwap;

private:
	AtomicSwapper m_swapper;
	Shared        m_shared;
};
} // namespace giada::m::model

#endif
