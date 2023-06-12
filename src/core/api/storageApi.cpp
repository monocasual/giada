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

#include "storageApi.h"
#include "core/actions/actionFactory.h"
#include "core/channels/channelFactory.h"
#include "core/engine.h"
#include "core/midiSynchronizer.h"
#include "core/model/model.h"
#include "core/patchFactory.h"
#include "core/plugins/pluginFactory.h"
#include "core/waveFactory.h"
#include "utils/fs.h"
#include "utils/log.h"

namespace giada::m
{
StorageApi::StorageApi(Engine& e, model::Model& m, PluginManager& pm, MidiSynchronizer& ms,
    Mixer& mx, ChannelManager& cm, KernelAudio& ka, Sequencer& s, ActionRecorder& ar)
: m_engine(e)
, m_model(m)
, m_pluginManager(pm)
, m_midiSynchronizer(ms)
, m_mixer(mx)
, m_channelManager(cm)
, m_kernelAudio(ka)
, m_sequencer(s)
, m_actionRecorder(ar)
{
}

/* -------------------------------------------------------------------------- */

bool StorageApi::storeProject(const std::string& projectPath, const v::Model& uiModel,
    std::function<void(float)> progress) const
{
	progress(0.0f);

	if (!u::fs::mkdir(projectPath))
	{
		u::log::print("[StorageApi::storeProject] Unable to make project directory!\n");
		return false;
	}

	u::log::print("[StorageApi::storeProject] Project dir created: {}\n", projectPath);

	progress(0.3f);

	/* Write Model into Patch, then into file. */

	Patch patch;

	for (const v::Model::Column& column : uiModel.columns)
		patch.columns.push_back({column.id, column.width});

	patch.name       = uiModel.projectName;
	patch.metronome  = m_sequencer.isMetronomeOn(); // TODO - addShared bool metronome to Layout
	patch.samplerate = m_kernelAudio.getSampleRate();

	m_model.store(patch, projectPath);

	progress(0.6f);

	const std::string patchPath = u::fs::join(projectPath, patch.name + G_PATCH_EXT);

	if (!patchFactory::serialize(patch, patchPath))
		return false;

	u::log::print("[StorageApi::storeProject] Project patch saved as {}\n", patchPath);

	progress(1.0f);

	return true;
}

/* -------------------------------------------------------------------------- */

model::LoadState StorageApi::loadProject(const std::string& projectPath, PluginManager::SortMethod pluginSortMethod,
    std::function<void(float)> progress)
{
	u::log::print("[StorageApi::loadProject] Load project from {}\n", projectPath);

	progress(0.0f);

	/* Read the selected project's patch. */

	const std::string patchPath = u::fs::join(projectPath, u::fs::stripExt(u::fs::basename(projectPath)) + G_PATCH_EXT);
	const Patch       patch     = patchFactory::deserialize(patchPath);

	if (patch.status != G_FILE_OK)
		return {};

	progress(0.3f);

	/* Then suspend Mixer, MIDI synch and reset the engine. */

	m_midiSynchronizer.stopSendClock();
	m_mixer.disable();
	m_engine.reset(pluginSortMethod);

	/* Load the patch into Model. */

	const int                sampleRate  = m_kernelAudio.getSampleRate();
	const int                bufferSize  = m_kernelAudio.getBufferSize();
	const Resampler::Quality rsmpQuality = m_kernelAudio.getResamplerQuality();
	const model::LoadState   state       = m_model.load(patch, m_pluginManager, sampleRate, bufferSize, rsmpQuality);

	progress(0.6f);

	/* Prepare the engine. Recorder has to recompute the actions positions if
	the current samplerate != patch samplerate. Clock needs to update frames
	in sequencer. */

	const int  maxFramesInLoop = m_sequencer.getMaxFramesInLoop(sampleRate);
	const bool hasSolos        = m_channelManager.hasSolos();

	m_mixer.updateSoloCount(hasSolos);
	m_actionRecorder.updateSamplerate(sampleRate, patch.samplerate);
	m_sequencer.recomputeFrames(sampleRate);
	m_mixer.allocRecBuffer(maxFramesInLoop);

	progress(0.9f);

	/* Bring everything back online. */

	m_mixer.enable();
	m_midiSynchronizer.startSendClock(m_model.get().sequencer.bpm);

	progress(1.0f);

	return state;
}
} // namespace giada::m
