/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/model/model.h"
#include "core/actions/actionFactory.h"
#include "core/channels/channelFactory.h"
#include "core/model/document.h"
#include "core/plugins/pluginFactory.h"
#include "core/plugins/pluginManager.h"
#include "core/waveFactory.h"
#include "utils/log.h"
#include "utils/string.h"
#include <cassert>
#include <memory>
#ifdef G_DEBUG_MODE
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
	document.channels         = {};

	swap(SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void Model::load(const Conf& conf)
{
	Document& document = get();

	document.kernelAudio.api                     = conf.soundSystem;
	document.kernelAudio.deviceOut.id            = conf.soundDeviceOut;
	document.kernelAudio.deviceOut.channelsCount = conf.channelsOutCount;
	document.kernelAudio.deviceOut.channelsStart = conf.channelsOutStart;
	document.kernelAudio.deviceIn.id             = conf.soundDeviceIn;
	document.kernelAudio.deviceIn.channelsCount  = conf.channelsInCount;
	document.kernelAudio.deviceIn.channelsStart  = conf.channelsInStart;
	document.kernelAudio.samplerate              = conf.samplerate;
	document.kernelAudio.buffersize              = conf.buffersize;
	document.kernelAudio.limitOutput             = conf.limitOutput;
	document.kernelAudio.rsmpQuality             = conf.rsmpQuality;
	document.kernelAudio.recTriggerLevel         = conf.recTriggerLevel;

	document.kernelMidi.api         = conf.midiSystem;
	document.kernelMidi.portOut     = conf.midiPortOut;
	document.kernelMidi.portIn      = conf.midiPortIn;
	document.kernelMidi.midiMapPath = conf.midiMapPath;
	document.kernelMidi.sync        = conf.midiSync;

	document.mixer.inputRecMode   = conf.inputRecMode;
	document.mixer.recTriggerMode = conf.recTriggerMode;

	document.midiIn.enabled    = conf.midiInEnabled;
	document.midiIn.filter     = conf.midiInFilter;
	document.midiIn.rewind     = conf.midiInRewind;
	document.midiIn.startStop  = conf.midiInStartStop;
	document.midiIn.actionRec  = conf.midiInActionRec;
	document.midiIn.inputRec   = conf.midiInInputRec;
	document.midiIn.metronome  = conf.midiInMetronome;
	document.midiIn.volumeIn   = conf.midiInVolumeIn;
	document.midiIn.volumeOut  = conf.midiInVolumeOut;
	document.midiIn.beatDouble = conf.midiInBeatDouble;
	document.midiIn.beatHalf   = conf.midiInBeatHalf;

	document.behaviors.chansStopOnSeqHalt         = conf.chansStopOnSeqHalt;
	document.behaviors.treatRecsAsLoops           = conf.treatRecsAsLoops;
	document.behaviors.inputMonitorDefaultOn      = conf.inputMonitorDefaultOn;
	document.behaviors.overdubProtectionDefaultOn = conf.overdubProtectionDefaultOn;

	swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

LoadState Model::load(const Patch& patch, PluginManager& pluginManager, int sampleRate, int bufferSize, Resampler::Quality rsmpQuality)
{
	const float sampleRateRatio = sampleRate / static_cast<float>(patch.samplerate);

	/* Lock the shared data. Real-time thread can't read from it until this method
	goes out of scope. */

	const SharedLock lock     = lockShared(SwapType::NONE);
	Document&        document = get();

	const LoadState state = m_shared.load(patch, pluginManager, document.sequencer, sampleRate, bufferSize, rsmpQuality);

	document.channels = {};
	for (const Patch::Channel& pchannel : patch.channels)
	{
		Wave*                wave    = findWave(pchannel.waveId);
		std::vector<Plugin*> plugins = m_shared.findPlugins(pchannel.pluginIds);
		ChannelShared*       shared  = m_shared.findChannel(pchannel.id);
		assert(shared != nullptr);

		Channel channel = channelFactory::deserializeChannel(pchannel, *shared, sampleRateRatio, wave, plugins);
		document.channels.add(channel);
	}

	document.actions.set(actionFactory::deserializeActions(patch.actions));

	document.sequencer.status    = SeqStatus::STOPPED;
	document.sequencer.bars      = patch.bars;
	document.sequencer.beats     = patch.beats;
	document.sequencer.bpm       = patch.bpm;
	document.sequencer.quantize  = patch.quantize;
	document.sequencer.metronome = patch.metronome;

	return state;

	// Swap is performed when 'lock' goes out of scope
}

/* -------------------------------------------------------------------------- */

void Model::store(Conf& conf) const
{
	const Document& document = get();

	conf.soundSystem      = document.kernelAudio.api;
	conf.soundDeviceOut   = document.kernelAudio.deviceOut.id;
	conf.channelsOutCount = document.kernelAudio.deviceOut.channelsCount;
	conf.channelsOutStart = document.kernelAudio.deviceOut.channelsStart;
	conf.soundDeviceIn    = document.kernelAudio.deviceIn.id;
	conf.channelsInCount  = document.kernelAudio.deviceIn.channelsCount;
	conf.channelsInStart  = document.kernelAudio.deviceIn.channelsStart;
	conf.samplerate       = document.kernelAudio.samplerate;
	conf.buffersize       = document.kernelAudio.buffersize;
	conf.limitOutput      = document.kernelAudio.limitOutput;
	conf.rsmpQuality      = document.kernelAudio.rsmpQuality;
	conf.recTriggerLevel  = document.kernelAudio.recTriggerLevel;

	conf.midiSystem  = document.kernelMidi.api;
	conf.midiPortOut = document.kernelMidi.portOut;
	conf.midiPortIn  = document.kernelMidi.portIn;
	conf.midiMapPath = document.kernelMidi.midiMapPath;
	conf.midiSync    = document.kernelMidi.sync;

	conf.inputRecMode   = document.mixer.inputRecMode;
	conf.recTriggerMode = document.mixer.recTriggerMode;

	conf.midiInEnabled    = document.midiIn.enabled;
	conf.midiInFilter     = document.midiIn.filter;
	conf.midiInRewind     = document.midiIn.rewind;
	conf.midiInStartStop  = document.midiIn.startStop;
	conf.midiInActionRec  = document.midiIn.actionRec;
	conf.midiInInputRec   = document.midiIn.inputRec;
	conf.midiInMetronome  = document.midiIn.metronome;
	conf.midiInVolumeIn   = document.midiIn.volumeIn;
	conf.midiInVolumeOut  = document.midiIn.volumeOut;
	conf.midiInBeatDouble = document.midiIn.beatDouble;
	conf.midiInBeatHalf   = document.midiIn.beatHalf;

	conf.chansStopOnSeqHalt         = document.behaviors.chansStopOnSeqHalt;
	conf.treatRecsAsLoops           = document.behaviors.treatRecsAsLoops;
	conf.inputMonitorDefaultOn      = document.behaviors.inputMonitorDefaultOn;
	conf.overdubProtectionDefaultOn = document.behaviors.overdubProtectionDefaultOn;
}

/* -------------------------------------------------------------------------- */

void Model::store(Patch& patch, const std::string& projectPath)
{
	/* Lock the shared data. Real-time thread can't read from it until this method
	goes out of scope. Even if it's mostly a read-only operation, some Wave
	objects need to be updated at some point. */

	const SharedLock lock = lockShared(SwapType::NONE);

	const Document& document = get();

	patch.bars      = document.sequencer.bars;
	patch.beats     = document.sequencer.beats;
	patch.bpm       = document.sequencer.bpm;
	patch.quantize  = document.sequencer.quantize;
	patch.metronome = document.sequencer.metronome;

	for (const auto& p : getAllPlugins())
		patch.plugins.push_back(pluginFactory::serializePlugin(*p));

	patch.actions = actionFactory::serializeActions(document.actions.getAll());

	for (auto& w : getAllWaves())
	{
		/* Update all existing file paths in Waves, so that they point to the 
		project folder they belong to. */

		w->setPath(waveFactory::makeUniqueWavePath(projectPath, *w, getAllWaves()));
		waveFactory::save(*w, w->getPath()); // TODO - error checking

		patch.waves.push_back(waveFactory::serializeWave(*w));
	}

	for (const Channel& c : document.channels.getAll())
		patch.channels.push_back(channelFactory::serializeChannel(c));
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

bool Model::isLocked() const
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

#ifdef G_DEBUG_MODE

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
