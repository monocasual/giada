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

#include "core/model/storage.h"
#include "core/channels/channelFactory.h"
#include "core/conf.h"
#include "core/kernelAudio.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/pluginManager.h"
#include "core/sequencer.h"
#include "core/waveFactory.h"
#include "src/core/actions/actionRecorder.h"
#include <cassert>
#include <memory>

namespace giada::m::model
{
void store(Patch::Data& patch, Engine& engine)
{
	const Layout& layout = engine.model.get();

	patch.bars       = layout.sequencer.bars;
	patch.beats      = layout.sequencer.beats;
	patch.bpm        = layout.sequencer.bpm;
	patch.quantize   = layout.sequencer.quantize;
	patch.metronome  = engine.sequencer.isMetronomeOn(); // TODO - addShared bool metronome to Layout
	patch.samplerate = engine.getSampleRate();

	patch.plugins.clear();
	for (const auto& p : engine.model.getAllShared<PluginPtrs>())
		patch.plugins.push_back(engine.pluginManager.serializePlugin(*p));

	patch.actions = engine.actionRecorder.serializeActions(engine.model.getAllShared<Actions::Map>());

	patch.waves.clear();
	for (const auto& w : engine.model.getAllShared<WavePtrs>())
		patch.waves.push_back(engine.waveFactory.serializeWave(*w));

	patch.channels.clear();
	for (const Channel& c : layout.channels)
		patch.channels.push_back(engine.channelFactory.serializeChannel(c));
}

/* -------------------------------------------------------------------------- */

void store(Conf::Data& conf, Engine& engine)
{
	const Layout& layout = engine.model.get();

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

LoadState load(const Patch::Data& patch, Engine& engine)
{
	const int   sampleRate      = engine.getSampleRate();
	const int   bufferSize      = engine.getBufferSize();
	const float sampleRateRatio = sampleRate / static_cast<float>(patch.samplerate);

	DataLock lock = engine.model.lockData(SwapType::NONE);

	/* Clear and re-initialize channels first. */

	engine.model.get().channels = {};
	engine.model.getAllShared<ChannelSharedPtrs>().clear();

	LoadState state;

	/* Load external data first: plug-ins and waves. */

	engine.model.getAllShared<PluginPtrs>().clear();
	for (const Patch::Plugin& pplugin : patch.plugins)
	{
		std::unique_ptr<Plugin> p = engine.pluginManager.deserializePlugin(pplugin, sampleRate, bufferSize, engine.model.get().sequencer);
		if (!p->valid)
			state.missingPlugins.push_back(pplugin.path);
		engine.model.getAllShared<PluginPtrs>().push_back(std::move(p));
	}

	engine.model.getAllShared<WavePtrs>().clear();
	for (const Patch::Wave& pwave : patch.waves)
	{
		std::unique_ptr<Wave> w = engine.waveFactory.deserializeWave(pwave, sampleRate, engine.conf.data.rsmpQuality);
		if (w != nullptr)
			engine.model.getAllShared<WavePtrs>().push_back(std::move(w));
		else
			state.missingWaves.push_back(pwave.path);
	}

	/* Then load up channels, actions and global properties. */

	for (const Patch::Channel& pchannel : patch.channels)
		engine.model.get().channels.push_back(engine.channelFactory.deserializeChannel(pchannel, sampleRateRatio, bufferSize));

	engine.model.getAllShared<Actions::Map>() = engine.actionRecorder.deserializeActions(patch.actions);

	engine.model.get().sequencer.status   = SeqStatus::STOPPED;
	engine.model.get().sequencer.bars     = patch.bars;
	engine.model.get().sequencer.beats    = patch.beats;
	engine.model.get().sequencer.bpm      = patch.bpm;
	engine.model.get().sequencer.quantize = patch.quantize;

	return state;
}

/* -------------------------------------------------------------------------- */

void load(const Conf::Data& c, Engine& engine)
{
	engine.model.get().midiIn.enabled    = c.midiInEnabled;
	engine.model.get().midiIn.filter     = c.midiInFilter;
	engine.model.get().midiIn.rewind     = c.midiInRewind;
	engine.model.get().midiIn.startStop  = c.midiInStartStop;
	engine.model.get().midiIn.actionRec  = c.midiInActionRec;
	engine.model.get().midiIn.inputRec   = c.midiInInputRec;
	engine.model.get().midiIn.volumeIn   = c.midiInVolumeIn;
	engine.model.get().midiIn.volumeOut  = c.midiInVolumeOut;
	engine.model.get().midiIn.beatDouble = c.midiInBeatDouble;
	engine.model.get().midiIn.beatHalf   = c.midiInBeatHalf;
	engine.model.get().midiIn.metronome  = c.midiInMetronome;

	engine.model.swap(SwapType::NONE);
}
} // namespace giada::m::model
