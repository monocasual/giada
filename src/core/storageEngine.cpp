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

#include "core/storageEngine.h"
#include "core/actions/actionFactory.h"
#include "core/channels/channelFactory.h"
#include "core/engine.h"
#include "core/midiSynchronizer.h"
#include "core/model/model.h"
#include "core/waveFactory.h"
#include "utils/fs.h"
#include "utils/log.h"

namespace giada::m
{
bool StorageEngine::LoadState::isGood() const
{
	return patch == G_FILE_OK && missingWaves.empty() && missingPlugins.empty();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

StorageEngine::StorageEngine(Engine& e, model::Model& m, Conf& c, Patch& p, PluginManager& pm,
    MidiSynchronizer& ms, Mixer& mx, ChannelManager& cm, KernelAudio& ka, Sequencer& s,
    ActionRecorder& ar)
: m_engine(e)
, m_model(m)
, m_conf(c)
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

bool StorageEngine::storeProject(const std::string& projectName, const std::string& projectPath,
    const v::Ui::State& uiState, std::function<void(float)> progress)
{
	progress(0.0f);

	if (!u::fs::mkdir(projectPath))
	{
		u::log::print("[StorageEngine::storeProject] Unable to make project directory!\n");
		return false;
	}

	u::log::print("[StorageEngine::storeProject] Project dir created: %s\n", projectPath);

	/* Update all existing file paths in Waves, so that they point to the project
	folder they belong to. */

	for (std::unique_ptr<Wave>& w : m_model.getAllShared<model::WavePtrs>())
	{
		w->setPath(waveFactory::makeUniqueWavePath(projectPath, *w, m_model.getAllShared<model::WavePtrs>()));
		waveFactory::save(*w, w->getPath()); // TODO - error checking
	}

	progress(0.3f);

	/* Write Model into Patch, then into file. */

	storePatch(projectName, uiState);

	progress(0.6f);

	const std::string patchPath = u::fs::join(projectPath, projectName + G_PATCH_EXT);

	if (!m_patch.write(patchPath))
		return false;

	/* Store the parent folder the project belongs to, in order to reuse it the 
	next time. */

	m_conf.data.patchPath = u::fs::getUpDir(u::fs::getUpDir(patchPath));

	u::log::print("[StorageEngine::storeProject] Project patch saved as %s\n", patchPath);

	progress(1.0f);

	return true;
}

/* -------------------------------------------------------------------------- */

StorageEngine::LoadState StorageEngine::loadProject(const std::string& projectPath, std::function<void(float)> progress)
{
	u::log::print("[StorageEngine::loadProject] Load project from %s\n", projectPath);

	progress(0.0f);

	/* Suspend MIDI clock output (if enabled). */

	m_midiSynchronizer.stopSendClock();

	/* Read the selected project's m_patch.data. */

	const std::string patchPath = u::fs::join(projectPath, u::fs::stripExt(u::fs::basename(projectPath)) + G_PATCH_EXT);

	m_patch.reset();
	if (int res = m_patch.read(patchPath, projectPath); res != G_FILE_OK)
		return {res};

	progress(0.3f);

	/* Then suspend Mixer, reset and fill the model. */

	m_mixer.disable();
	m_engine.reset();
	LoadState state = loadPatch();

	progress(0.6f);

	/* Prepare the engine. Recorder has to recompute the actions positions if 
	the current samplerate != patch samplerate. Clock needs to update frames
	in sequencer. */

	const int  sampleRate      = m_kernelAudio.getSampleRate();
	const int  maxFramesInLoop = m_sequencer.getMaxFramesInLoop(sampleRate);
	const bool hasSolos        = m_channelManager.hasSolos();

	m_mixer.updateSoloCount(hasSolos);
	m_actionRecorder.updateSamplerate(sampleRate, m_patch.data.samplerate);
	m_sequencer.recomputeFrames(sampleRate);
	m_mixer.allocRecBuffer(maxFramesInLoop);

	progress(0.9f);

	/* Store the parent folder the project belongs to, in order to reuse it the 
	next time. */

	m_conf.data.patchPath = u::fs::getUpDir(projectPath);

	/* Mixer is ready to go back online. */

	m_mixer.enable();

	/* Restore MIDI clock output. */

	m_midiSynchronizer.startSendClock(m_model.get().sequencer.bpm);

	progress(1.0f);

	state.patch = G_FILE_OK;
	return state;
}

/* -------------------------------------------------------------------------- */

void StorageEngine::storePatch(const std::string& projectName, const v::Ui::State& uiState)
{
	m_patch.data.columns.clear();
	for (auto const& [id, width] : uiState.columns)
		m_patch.data.columns.push_back({id, width});

	const model::Layout& layout = m_model.get();

	m_patch.data.name       = projectName;
	m_patch.data.bars       = layout.sequencer.bars;
	m_patch.data.beats      = layout.sequencer.beats;
	m_patch.data.bpm        = layout.sequencer.bpm;
	m_patch.data.quantize   = layout.sequencer.quantize;
	m_patch.data.metronome  = m_sequencer.isMetronomeOn(); // TODO - addShared bool metronome to Layout
	m_patch.data.samplerate = m_kernelAudio.getSampleRate();

	m_patch.data.plugins.clear();
	for (const auto& p : m_model.getAllShared<model::PluginPtrs>())
		m_patch.data.plugins.push_back(m_pluginManager.serializePlugin(*p));

	m_patch.data.actions = actionFactory::serializeActions(m_model.getAllShared<Actions::Map>());

	m_patch.data.waves.clear();
	for (const auto& w : m_model.getAllShared<model::WavePtrs>())
		m_patch.data.waves.push_back(waveFactory::serializeWave(*w));

	m_patch.data.channels.clear();
	for (const Channel& c : layout.channels)
		m_patch.data.channels.push_back(channelFactory::serializeChannel(c));
}

/* -------------------------------------------------------------------------- */

StorageEngine::LoadState StorageEngine::loadPatch()
{
	const int   sampleRate      = m_engine.getSampleRate();
	const int   bufferSize      = m_engine.getBufferSize();
	const float sampleRateRatio = sampleRate / static_cast<float>(m_patch.data.samplerate);

	/* Lock the model's data. Real-time thread can't read from it until this method 
	goes out of scope. */

	model::DataLock lock = m_model.lockData(model::SwapType::NONE);

	/* Clear and re-initialize channels first. */

	m_model.get().channels = {};
	m_model.getAllShared<model::ChannelSharedPtrs>().clear();

	LoadState state;

	/* Load external data first: plug-ins and waves. */

	m_model.getAllShared<model::PluginPtrs>().clear();
	for (const Patch::Plugin& pplugin : m_patch.data.plugins)
	{
		std::unique_ptr<Plugin> p = m_engine.getPluginsEngine().deserialize(pplugin);
		if (!p->valid)
			state.missingPlugins.push_back(pplugin.path);
		m_model.getAllShared<model::PluginPtrs>().push_back(std::move(p));
	}

	m_model.getAllShared<model::WavePtrs>().clear();
	for (const Patch::Wave& pwave : m_patch.data.waves)
	{
		std::unique_ptr<Wave> w = waveFactory::deserializeWave(pwave, sampleRate, m_conf.data.rsmpQuality);
		if (w != nullptr)
			m_model.getAllShared<model::WavePtrs>().push_back(std::move(w));
		else
			state.missingWaves.push_back(pwave.path);
	}

	/* Then load up channels, actions and global properties. */

	for (const Patch::Channel& pchannel : m_patch.data.channels)
	{
		channelFactory::Data data = m_engine.getChannelsEngine().deserializeChannel(pchannel, sampleRateRatio, bufferSize);
		m_model.get().channels.push_back(data.channel);
		m_model.addShared(std::move(data.shared));
	}

	m_model.getAllShared<Actions::Map>() = m_engine.getActionEditorEngine().deserializeActions(m_patch.data.actions);

	m_model.get().sequencer.status   = SeqStatus::STOPPED;
	m_model.get().sequencer.bars     = m_patch.data.bars;
	m_model.get().sequencer.beats    = m_patch.data.beats;
	m_model.get().sequencer.bpm      = m_patch.data.bpm;
	m_model.get().sequencer.quantize = m_patch.data.quantize;

	return state;
}
} // namespace giada::m
