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

#ifndef G_STORAGE_API_H
#define G_STORAGE_API_H

#include "core/model/model.h"
#include "core/types.h"
#include "gui/model.h"
#include <functional>
#include <string>
#include <vector>

namespace giada::m
{
class Engine;
class MidiDispatcher;
class MidiSynchronizer;
class ChannelManager;
class KernelAudio;
class Sequencer;
class ActionRecorder;
class StorageApi
{
public:
	struct LoadState
	{
		bool isGood() const;

		int                      patch          = G_FILE_OK;
		std::vector<std::string> missingWaves   = {};
		std::vector<std::string> missingPlugins = {};
	};

	StorageApi(Engine&, model::Model&, Patch&, PluginManager&, MidiSynchronizer&,
	    Mixer&, ChannelManager&, KernelAudio&, Sequencer&, ActionRecorder&);

	/* storeProject
	Saves the current project. Returns true on success. */

	bool storeProject(const std::string& projectPath, const v::Model&,
	    std::function<void(float)>       progress);

	/* loadProject
	Loads a new project. Returns a LoadState object containing the operation
	state. */

	LoadState loadProject(const std::string& projectPath, PluginManager::SortMethod, std::function<void(float)> progress);

private:
	void      storePatch(const v::Model&);
	LoadState loadPatch(const Patch&);

	Engine&           m_engine;
	model::Model&     m_model;
	Patch&            m_patch;
	PluginManager&    m_pluginManager;
	MidiSynchronizer& m_midiSynchronizer;
	Mixer&            m_mixer;
	ChannelManager&   m_channelManager;
	KernelAudio&      m_kernelAudio;
	Sequencer&        m_sequencer;
	ActionRecorder&   m_actionRecorder;
};
} // namespace giada::m

#endif
