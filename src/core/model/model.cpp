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

#include "src/core/model/model.h"
#include "src/core/actions/actionFactory.h"
#include "src/core/channels/channelFactory.h"
#include "src/core/model/document.h"
#include "src/core/plugins/pluginFactory.h"
#include "src/core/plugins/pluginManager.h"
#include "src/core/waveFactory.h"
#include "src/utils/log.h"
#include "src/utils/string.h"
#include <cassert>
#include <memory>
#if G_DEBUG_MODE
#include <fmt/core.h>
#endif
#include <fmt/ostream.h>

using namespace mcl;

namespace giada::m::model
{
Model::Model()
: onSwap(nullptr)
{
}

/* -------------------------------------------------------------------------- */

void Model::init()
{
	m_shared.init();

	Document& document        = get();
	document                  = {};
	document.sequencer.shared = &m_shared.m_sequencer;
	document.mixer.shared     = &m_shared.m_mixer;

	swap(SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void Model::reset()
{
	m_shared.init();

	Document& document        = get();
	document.sequencer        = {};
	document.sequencer.shared = &m_shared.m_sequencer;
	document.mixer            = {};
	document.mixer.shared     = &m_shared.m_mixer;
	document.tracks           = {};

	swap(SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void Model::load(const Conf& conf)
{
	get().load(conf);
	swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

LoadState Model::load(const Patch& patch, PluginManager& pluginManager, int sampleRate, int bufferSize, Resampler::Quality rsmpQuality)
{
	const float sampleRateRatio = sampleRate / static_cast<float>(patch.samplerate);

	/* Lock the shared data. Real-time thread can't read from it until this method
	goes out of scope. */

	const SharedLock lock  = lockShared(SwapType::NONE);
	const LoadState  state = m_shared.load(patch, pluginManager, get().sequencer, sampleRate, bufferSize, rsmpQuality);
	get().load(patch, m_shared, sampleRateRatio);

	return state;

	// Swap is performed when 'lock' goes out of scope
}

/* -------------------------------------------------------------------------- */

void Model::store(Conf& conf) const
{
	get().store(conf);
}

/* -------------------------------------------------------------------------- */

void Model::store(Patch& patch, const std::string& projectPath)
{
	get().store(patch);

	/* Lock the shared data before storing it. Real-time thread can't read from
	it until this method goes out of scope. Even if it's mostly a read-only operation,
	some Wave objects need to be updated at some point. */

	const SharedLock lock = lockShared(SwapType::NONE);

	m_shared.store(patch, projectPath);
}

/* -------------------------------------------------------------------------- */

bool Model::registerThread(Thread t, bool realtime) const
{
	return m_swapper.registerThread(u::string::toString(t), realtime);
}

/* -------------------------------------------------------------------------- */

Document&       Model::get() { return m_swapper.get(); }
const Document& Model::get() const { return m_swapper.get(); }
DocumentLock    Model::get_RT() const { return DocumentLock(m_swapper); }

/* -------------------------------------------------------------------------- */

void Model::swap(SwapType t)
{
	m_swapper.swap();
	if (onSwap != nullptr)
		onSwap(t);
}

/* -------------------------------------------------------------------------- */

SharedLock Model::lockShared(SwapType t)
{
	return SharedLock(*this, t);
}

/* -------------------------------------------------------------------------- */

bool Model::isRtLocked() const
{
	return m_swapper.isRtLocked();
}

/* -------------------------------------------------------------------------- */

std::vector<std::unique_ptr<Wave>>&          Model::getAllWaves() { return m_shared.getAllWaves(); };
std::vector<std::unique_ptr<Plugin>>&        Model::getAllPlugins() { return m_shared.getAllPlugins(); }
std::vector<std::unique_ptr<ChannelShared>>& Model::getAllChannelsShared() { return m_shared.getAllChannels(); }

/* -------------------------------------------------------------------------- */

Plugin* Model::findPlugin(ID id) { return m_shared.findPlugin(id); }
Wave*   Model::findWave(ID id) { return m_shared.findWave(id); }

/* -------------------------------------------------------------------------- */

Wave& Model::addWave(std::unique_ptr<Wave> w)
{
	const SharedLock lock = lockShared(SwapType::NONE);
	return m_shared.addWave(std::move(w));
}

Plugin& Model::addPlugin(std::unique_ptr<Plugin> p)
{
	const SharedLock lock = lockShared(SwapType::NONE);
	return m_shared.addPlugin(std::move(p));
}

ChannelShared& Model::addChannelShared(std::unique_ptr<ChannelShared> cs)
{
	const SharedLock lock = lockShared(SwapType::NONE);
	return m_shared.addChannel(std::move(cs));
}

/* -------------------------------------------------------------------------- */

void Model::removePlugin(const Plugin& p)
{
	const SharedLock lock = lockShared(SwapType::NONE);
	m_shared.removePlugin(p);
}

void Model::removeWave(const Wave& w)
{
	const SharedLock lock = lockShared(SwapType::NONE);
	m_shared.removeWave(w);
}

void Model::removeChannelShared(const ChannelShared& c)
{
	const SharedLock lock = lockShared(SwapType::NONE);
	m_shared.removeChannel(c);
}

/* -------------------------------------------------------------------------- */

void Model::clearPlugins()
{
	const SharedLock lock = lockShared(SwapType::NONE);
	m_shared.clearPlugins();
}

void Model::clearWaves()
{
	const SharedLock lock = lockShared(SwapType::NONE);
	m_shared.clearWaves();
}

/* -------------------------------------------------------------------------- */

#if G_DEBUG_MODE

void Model::debug()
{
	puts("======== SYSTEM STATUS ========");

	puts("-------------------------------");
	m_swapper.debug();
	puts("-------------------------------");

	get().debug();
	m_shared.debug();
}

#endif // G_DEBUG_MODE
} // namespace giada::m::model
