/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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

#include "core/model/storage.h"
#include "core/channels/channelManager.h"
#include "core/conf.h"
#include "core/engine.h"
#include "core/kernelAudio.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/pluginManager.h"
#include "core/sequencer.h"
#include "core/waveManager.h"
#include "src/core/actions/actionRecorder.h"
#include <cassert>
#include <memory>

extern giada::m::Engine g_engine;

namespace giada::m::model
{
namespace
{
void loadChannels_(const std::vector<Patch::Channel>& channels, int samplerate)
{
	float samplerateRatio = g_engine.kernelAudio.getSampleRate() / static_cast<float>(samplerate);

	for (const Patch::Channel& pchannel : channels)
		g_engine.model.get().channels.push_back(
		    g_engine.channelManager.deserializeChannel(pchannel, samplerateRatio, g_engine.kernelAudio.getBufferSize()));
}

/* -------------------------------------------------------------------------- */

void loadActions_(const std::vector<Patch::Action>& pactions)
{
	g_engine.model.getAllShared<Actions::Map>() = std::move(g_engine.actionRecorder.deserializeActions(pactions));
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void store(Patch::Data& patch)
{
	const Layout& layout = g_engine.model.get();

	patch.bars       = layout.sequencer.bars;
	patch.beats      = layout.sequencer.beats;
	patch.bpm        = layout.sequencer.bpm;
	patch.quantize   = layout.sequencer.quantize;
	patch.metronome  = g_engine.sequencer.isMetronomeOn(); // TODO - addShared bool metronome to Layout
	patch.samplerate = g_engine.kernelAudio.getSampleRate();

#ifdef WITH_VST
	patch.plugins.clear();
	for (const auto& p : g_engine.model.getAllShared<PluginPtrs>())
		patch.plugins.push_back(g_engine.pluginManager.serializePlugin(*p));
#endif

	patch.actions = g_engine.actionRecorder.serializeActions(g_engine.model.getAllShared<Actions::Map>());

	patch.waves.clear();
	for (const auto& w : g_engine.model.getAllShared<WavePtrs>())
		patch.waves.push_back(g_engine.waveManager.serializeWave(*w));

	patch.channels.clear();
	for (const Channel& c : layout.channels)
		patch.channels.push_back(g_engine.channelManager.serializeChannel(c));
}

/* -------------------------------------------------------------------------- */

void store(Conf::Data& conf)
{
	const Layout& layout = g_engine.model.get();

	conf.midiInEnabled    = layout.midiIn.enabled;
	conf.midiInFilter     = layout.midiIn.filter;
	conf.midiInRewind     = layout.midiIn.rewind;
	conf.midiInStartStop  = layout.midiIn.startStop;
	conf.midiInActionRec  = layout.midiIn.actionRec;
	conf.midiInInputRec   = layout.midiIn.inputRec;
	conf.midiInMetronome  = layout.midiIn.metronome;
	conf.midiInVolumeIn   = layout.midiIn.volumeIn;
	conf.midiInVolumeOut  = layout.midiIn.volumeOut;
	conf.midiInBeatDouble = layout.midiIn.beatDouble;
	conf.midiInBeatHalf   = layout.midiIn.beatHalf;
}

/* -------------------------------------------------------------------------- */

LoadState load(const Patch::Data& patch)
{
	DataLock lock = g_engine.model.lockData();

	/* Clear and re-initialize channels first. */

	g_engine.model.get().channels = {};
	g_engine.model.getAllShared<ChannelSharedPtrs>().clear();

	LoadState state;

	/* Load external data first: plug-ins and waves. */

#ifdef WITH_VST
	g_engine.model.getAllShared<PluginPtrs>().clear();
	for (const Patch::Plugin& pplugin : patch.plugins)
	{
		std::unique_ptr<Plugin> p = g_engine.pluginManager.deserializePlugin(
		    pplugin, g_engine.kernelAudio.getSampleRate(), g_engine.kernelAudio.getBufferSize(), g_engine.sequencer);

		if (!p->valid)
			state.missingPlugins.push_back(pplugin.path);

		g_engine.model.getAllShared<PluginPtrs>().push_back(std::move(p));
	}
#endif

	g_engine.model.getAllShared<WavePtrs>().clear();
	for (const Patch::Wave& pwave : patch.waves)
	{
		std::unique_ptr<Wave> w = g_engine.waveManager.deserializeWave(pwave, g_engine.kernelAudio.getSampleRate(),
		    g_engine.conf.data.rsmpQuality);

		if (w != nullptr)
			g_engine.model.getAllShared<WavePtrs>().push_back(std::move(w));
		else
			state.missingWaves.push_back(pwave.path);
	}

	/* Then load up channels, actions and global properties. */

	loadChannels_(patch.channels, g_engine.patch.data.samplerate);
	loadActions_(patch.actions);

	g_engine.model.get().sequencer.status   = SeqStatus::STOPPED;
	g_engine.model.get().sequencer.bars     = patch.bars;
	g_engine.model.get().sequencer.beats    = patch.beats;
	g_engine.model.get().sequencer.bpm      = patch.bpm;
	g_engine.model.get().sequencer.quantize = patch.quantize;

	return state;
}

/* -------------------------------------------------------------------------- */

void load(const Conf::Data& c)
{
	g_engine.model.get().midiIn.enabled    = c.midiInEnabled;
	g_engine.model.get().midiIn.filter     = c.midiInFilter;
	g_engine.model.get().midiIn.rewind     = c.midiInRewind;
	g_engine.model.get().midiIn.startStop  = c.midiInStartStop;
	g_engine.model.get().midiIn.actionRec  = c.midiInActionRec;
	g_engine.model.get().midiIn.inputRec   = c.midiInInputRec;
	g_engine.model.get().midiIn.volumeIn   = c.midiInVolumeIn;
	g_engine.model.get().midiIn.volumeOut  = c.midiInVolumeOut;
	g_engine.model.get().midiIn.beatDouble = c.midiInBeatDouble;
	g_engine.model.get().midiIn.beatHalf   = c.midiInBeatHalf;
	g_engine.model.get().midiIn.metronome  = c.midiInMetronome;

	g_engine.model.swap(SwapType::NONE);
}
} // namespace giada::m::model
