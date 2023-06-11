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
#include "core/waveFactory.h"
#include "utils/fs.h"
#include "utils/log.h"

namespace giada::m
{
bool StorageApi::LoadState::isGood() const
{
	return patch == G_FILE_OK && missingWaves.empty() && missingPlugins.empty();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

StorageApi::StorageApi(Engine& e, model::Model& m, Patch& p, PluginManager& pm,
    MidiSynchronizer& ms, Mixer& mx, ChannelManager& cm, KernelAudio& ka, Sequencer& s,
    ActionRecorder& ar)
: m_engine(e)
, m_model(m)
, m_patch(p)
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
    std::function<void(float)> progress)
{
	progress(0.0f);

	if (!u::fs::mkdir(projectPath))
	{
		u::log::print("[StorageApi::storeProject] Unable to make project directory!\n");
		return false;
	}

	u::log::print("[StorageApi::storeProject] Project dir created: {}\n", projectPath);

	/* Update all existing file paths in Waves, so that they point to the project
	folder they belong to. */

	for (std::unique_ptr<Wave>& w : m_model.getAllWaves())
	{
		w->setPath(waveFactory::makeUniqueWavePath(projectPath, *w, m_model.getAllWaves()));
		waveFactory::save(*w, w->getPath()); // TODO - error checking
	}

	progress(0.3f);

	/* Write Model into Patch, then into file. */

	storePatch(uiModel);

	progress(0.6f);

	const std::string patchPath = u::fs::join(projectPath, uiModel.projectName + G_PATCH_EXT);

	if (!patchFactory::serialize(m_patch, patchPath))
		return false;

	u::log::print("[StorageApi::storeProject] Project patch saved as {}\n", patchPath);

	progress(1.0f);

	return true;
}

/* -------------------------------------------------------------------------- */

StorageApi::LoadState StorageApi::loadProject(const std::string& projectPath, PluginManager::SortMethod pluginSortMethod,
    std::function<void(float)> progress)
{
	u::log::print("[StorageApi::loadProject] Load project from {}\n", projectPath);

	progress(0.0f);

	/* Suspend MIDI clock output (if enabled). */

	m_midiSynchronizer.stopSendClock();

	/* Read the selected project's patch */

	const std::string patchPath = u::fs::join(projectPath, u::fs::stripExt(u::fs::basename(projectPath)) + G_PATCH_EXT);

	m_patch = patchFactory::deserialize(patchPath);
	if (m_patch.status != G_FILE_OK)
		return {m_patch.status};

	progress(0.3f);

	/* Then suspend Mixer, reset and fill the model. */

	m_mixer.disable();
	m_engine.reset(pluginSortMethod);
	LoadState state = loadPatch();

	progress(0.6f);

	/* Prepare the engine. Recorder has to recompute the actions positions if
	the current samplerate != patch samplerate. Clock needs to update frames
	in sequencer. */

	const int  sampleRate      = m_kernelAudio.getSampleRate();
	const int  maxFramesInLoop = m_sequencer.getMaxFramesInLoop(sampleRate);
	const bool hasSolos        = m_channelManager.hasSolos();

	m_mixer.updateSoloCount(hasSolos);
	m_actionRecorder.updateSamplerate(sampleRate, m_patch.samplerate);
	m_sequencer.recomputeFrames(sampleRate);
	m_mixer.allocRecBuffer(maxFramesInLoop);

	progress(0.9f);

	/* Mixer is ready to go back online. */

	m_mixer.enable();

	/* Restore MIDI clock output. */

	m_midiSynchronizer.startSendClock(m_model.get().sequencer.bpm);

	progress(1.0f);

	state.patch = G_FILE_OK;
	return state;
}

/* -------------------------------------------------------------------------- */

void StorageApi::storePatch(const v::Model& uiModel)
{
	m_patch.columns.clear();
	for (const v::Model::Column& column : uiModel.columns)
		m_patch.columns.push_back({column.id, column.width});

	const model::Layout& layout = m_model.get();

	m_patch.name       = uiModel.projectName;
	m_patch.bars       = layout.sequencer.bars;
	m_patch.beats      = layout.sequencer.beats;
	m_patch.bpm        = layout.sequencer.bpm;
	m_patch.quantize   = layout.sequencer.quantize;
	m_patch.metronome  = m_sequencer.isMetronomeOn(); // TODO - addShared bool metronome to Layout
	m_patch.samplerate = m_kernelAudio.getSampleRate();

	m_patch.plugins.clear();
	for (const auto& p : m_model.getAllPlugins())
		m_patch.plugins.push_back(m_pluginManager.serializePlugin(*p));

	m_patch.actions = actionFactory::serializeActions(m_model.getAllActions());

	m_patch.waves.clear();
	for (const auto& w : m_model.getAllWaves())
		m_patch.waves.push_back(waveFactory::serializeWave(*w));

	m_patch.channels.clear();
	for (const Channel& c : layout.channels.getAll())
		m_patch.channels.push_back(channelFactory::serializeChannel(c));
}

/* -------------------------------------------------------------------------- */

StorageApi::LoadState StorageApi::loadPatch()
{
	const int   sampleRate      = m_kernelAudio.getSampleRate();
	const int   bufferSize      = m_kernelAudio.getBufferSize();
	const float sampleRateRatio = sampleRate / static_cast<float>(m_patch.samplerate);

	/* Lock the model's data. Real-time thread can't read from it until this method
	goes out of scope. */

	model::DataLock lock = m_model.lockData(model::SwapType::NONE);

	/* Clear and re-initialize channels first. */

	m_model.get().channels = {};
	m_model.getAllChannelsShared().clear();

	LoadState state;

	/* Load external data first: plug-ins and waves. */

	m_model.getAllPlugins().clear();
	for (const Patch::Plugin& pplugin : m_patch.plugins)
	{
		std::unique_ptr<Plugin> p = m_engine.getPluginsApi().deserialize(pplugin);
		if (!p->valid)
			state.missingPlugins.push_back(pplugin.path);
		m_model.getAllPlugins().push_back(std::move(p));
	}

	m_model.getAllWaves().clear();
	for (const Patch::Wave& pwave : m_patch.waves)
	{
		std::unique_ptr<Wave> w = waveFactory::deserializeWave(pwave, sampleRate, m_model.get().kernelAudio.rsmpQuality);
		if (w != nullptr)
			m_model.getAllWaves().push_back(std::move(w));
		else
			state.missingWaves.push_back(pwave.path);
	}

	/* Then load up channels, actions and global properties. */

	for (const Patch::Channel& pchannel : m_patch.channels)
	{
		channelFactory::Data data = m_engine.getChannelsApi().deserializeChannel(pchannel, sampleRateRatio, bufferSize);
		m_model.get().channels.add(data.channel);
		m_model.addShared(std::move(data.shared));
	}

	m_model.getAllActions() = m_engine.getActionEditorApi().deserializeActions(m_patch.actions);

	m_model.get().sequencer.status   = SeqStatus::STOPPED;
	m_model.get().sequencer.bars     = m_patch.bars;
	m_model.get().sequencer.beats    = m_patch.beats;
	m_model.get().sequencer.bpm      = m_patch.bpm;
	m_model.get().sequencer.quantize = m_patch.quantize;

	return state;
}
} // namespace giada::m
