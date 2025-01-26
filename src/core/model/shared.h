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

#ifndef G_MODEL_SHARED_H
#define G_MODEL_SHARED_H

#include "core/model/channels.h"
#include "core/model/loadState.h"
#include "core/model/mixer.h"
#include "core/model/sequencer.h"
#include "core/plugins/plugin.h"
#include "core/wave.h"

namespace giada::m
{
class PluginManager;
}

namespace giada::m::model
{
class Shared
{
	friend class Model;

public:
	/* init
	Initializes the shared data. All values go back to default. */

	void init();

	/* load
	Loads shared data from a Patch object. */

	LoadState load(const Patch&, PluginManager&, const Sequencer&, int sampleRate, int bufferSize, Resampler::Quality);

	/* store
	Stores shared data into a Patch object. */

	void store(Patch&, const std::string& projectPath);

#ifdef G_DEBUG_MODE
	void
	debug() const;
#endif

	std::vector<std::unique_ptr<Wave>>&          getAllWaves();
	std::vector<std::unique_ptr<Plugin>>&        getAllPlugins();
	std::vector<std::unique_ptr<ChannelShared>>& getAllChannels();

	/* find[*]
	Finds something in the shared data given an ID. Returns nullptr if the
	object is not found. */

	Plugin*        findPlugin(ID);
	Wave*          findWave(ID);
	ChannelShared* findChannel(ID);

	/* add[*]
	Adds some shared data (by moving it). Returns a reference to the last added
	shared item. */

	Wave&          addWave(std::unique_ptr<Wave>);
	Plugin&        addPlugin(std::unique_ptr<Plugin>);
	ChannelShared& addChannel(std::unique_ptr<ChannelShared>);

	void removePlugin(const Plugin&);
	void removeWave(const Wave&);
	void removeChannel(const ChannelShared&);

	void clearPlugins();
	void clearWaves();

	std::vector<Plugin*> findPlugins(std::vector<ID> pluginIds);

private:
	Sequencer::Shared                           m_sequencer;
	Mixer::Shared                               m_mixer;
	std::vector<std::unique_ptr<ChannelShared>> m_channels;

	std::vector<std::unique_ptr<Wave>>   m_waves;
	std::vector<std::unique_ptr<Plugin>> m_plugins;
};
} // namespace giada::m::model

#endif
