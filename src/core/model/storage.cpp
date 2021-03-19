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
#include "core/kernelAudio.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/pluginManager.h"
#include "core/recorderHandler.h"
#include "core/sequencer.h"
#include "core/waveManager.h"
#include <cassert>

namespace giada::m::model
{
namespace
{
void loadChannels_(const std::vector<patch::Channel>& channels, int samplerate)
{
	float samplerateRatio = conf::conf.samplerate / static_cast<float>(samplerate);

	for (const patch::Channel& pchannel : channels)
		get().channels.push_back(channelManager::deserializeChannel(pchannel, samplerateRatio));
}

/* -------------------------------------------------------------------------- */

void loadActions_(const std::vector<patch::Action>& pactions)
{
	getAll<Actions>() = std::move(recorderHandler::deserializeActions(pactions));
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void store(patch::Patch& patch)
{
	const Layout& layout = get();

	patch.bars       = layout.clock.bars;
	patch.beats      = layout.clock.beats;
	patch.bpm        = layout.clock.bpm;
	patch.quantize   = layout.clock.quantize;
	patch.metronome  = sequencer::isMetronomeOn(); // TODO - add bool metronome to Layout
	patch.samplerate = conf::conf.samplerate;

#ifdef WITH_VST
	for (const auto& p : getAll<PluginPtrs>())
		patch.plugins.push_back(pluginManager::serializePlugin(*p));
#endif

	patch.actions = recorderHandler::serializeActions(getAll<Actions>());

	for (const auto& w : getAll<WavePtrs>())
		patch.waves.push_back(waveManager::serializeWave(*w));

	for (const channel::Data& c : layout.channels)
		patch.channels.push_back(channelManager::serializeChannel(c));
}

/* -------------------------------------------------------------------------- */

void store(conf::Conf& conf)
{
	const Layout& layout = get();

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

void load(const patch::Patch& patch)
{
	DataLock lock;

	/* Clear and re-initialize channels first. */

	get().channels = {};
	getAll<ChannelBufferPtrs>().clear();
	getAll<ChannelStatePtrs>().clear();

	/* Load external data first: plug-ins and waves. */

#ifdef WITH_VST
	getAll<PluginPtrs>().clear();
	for (const patch::Plugin& pplugin : patch.plugins)
		getAll<PluginPtrs>().push_back(pluginManager::deserializePlugin(pplugin, patch.version));
#endif

	getAll<WavePtrs>().clear();
	for (const patch::Wave& pwave : patch.waves)
	{
		std::unique_ptr<Wave> w = waveManager::deserializeWave(pwave, conf::conf.samplerate,
		    conf::conf.rsmpQuality);
		if (w != nullptr)
			getAll<WavePtrs>().push_back(std::move(w));
	}

	/* Then load up channels, actions and global properties. */

	loadChannels_(patch.channels, patch::patch.samplerate);
	loadActions_(patch.actions);

	get().clock.status   = ClockStatus::STOPPED;
	get().clock.bars     = patch.bars;
	get().clock.beats    = patch.beats;
	get().clock.bpm      = patch.bpm;
	get().clock.quantize = patch.quantize;
}

/* -------------------------------------------------------------------------- */

void load(const conf::Conf& c)
{
	get().midiIn.enabled    = c.midiInEnabled;
	get().midiIn.filter     = c.midiInFilter;
	get().midiIn.rewind     = c.midiInRewind;
	get().midiIn.startStop  = c.midiInStartStop;
	get().midiIn.actionRec  = c.midiInActionRec;
	get().midiIn.inputRec   = c.midiInInputRec;
	get().midiIn.volumeIn   = c.midiInVolumeIn;
	get().midiIn.volumeOut  = c.midiInVolumeOut;
	get().midiIn.beatDouble = c.midiInBeatDouble;
	get().midiIn.beatHalf   = c.midiInBeatHalf;
	get().midiIn.metronome  = c.midiInMetronome;

	swap(SwapType::NONE);
}
} // namespace giada::m::model
