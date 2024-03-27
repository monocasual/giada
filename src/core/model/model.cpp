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
namespace
{
template <typename T>
auto getIter_(const std::vector<std::unique_ptr<T>>& source, ID id)
{
	return u::vector::findIf(source, [id](const std::unique_ptr<T>& p) { return p->id == id; });
}

/* -------------------------------------------------------------------------- */

template <typename S>
auto* get_(S& source, ID id)
{
	auto it = getIter_(source, id);
	return it == source.end() ? nullptr : it->get();
}

/* -------------------------------------------------------------------------- */

template <typename T>
typename T::element_type& add_(std::vector<T>& dest, T obj, Model& model)
{
	DataLock lock = model.lockData(SwapType::NONE);
	dest.push_back(std::move(obj));
	return *dest.back().get();
}

/* -------------------------------------------------------------------------- */

template <typename D, typename T>
void remove_(D& dest, T& ref, Model& model)
{
	DataLock lock = model.lockData(SwapType::NONE);
	u::vector::removeIf(dest, [&ref](const auto& other) { return other.get() == &ref; });
}

/* -------------------------------------------------------------------------- */

template <typename T>
void clear_(std::vector<T>& dest, Model& model)
{
	DataLock lock = model.lockData(SwapType::NONE);
	dest.clear();
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

bool LoadState::isGood() const
{
	return patch.status == G_FILE_OK && missingWaves.empty() && missingPlugins.empty();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Model::Model()
: onSwap(nullptr)
{
}

/* -------------------------------------------------------------------------- */

void Model::init()
{
	m_shared = {};

	Document& document        = get();
	document                  = {};
	document.sequencer.shared = &m_shared.sequencerShared;
	document.mixer.shared     = &m_shared.mixerShared;

	swap(SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void Model::reset()
{
	m_shared = {};

	Document& document        = get();
	document.sequencer        = {};
	document.sequencer.shared = &m_shared.sequencerShared;
	document.mixer            = {};
	document.mixer.shared     = &m_shared.mixerShared;
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

	DataLock  lock     = lockData(SwapType::NONE);
	Document& document = get();
	LoadState state{patch};

	/* Clear and re-initialize stuff first. */

	document.channels = {};
	getAllChannelsShared().clear();
	getAllPlugins().clear();
	getAllWaves().clear();

	/* Load external data first: plug-ins and waves. */

	for (const Patch::Plugin& pplugin : patch.plugins)
	{
		std::unique_ptr<juce::AudioPluginInstance> pi = pluginManager.makeJucePlugin(pplugin.path, sampleRate, bufferSize);
		std::unique_ptr<Plugin>                    p  = pluginFactory::deserializePlugin(pplugin, std::move(pi), document.sequencer, sampleRate, bufferSize);
		if (!p->valid)
			state.missingPlugins.push_back(pplugin.path);
		getAllPlugins().push_back(std::move(p));
	}

	for (const Patch::Wave& pwave : patch.waves)
	{
		std::unique_ptr<Wave> w = waveFactory::deserializeWave(pwave, sampleRate, rsmpQuality);
		if (w != nullptr)
			getAllWaves().push_back(std::move(w));
		else
			state.missingWaves.push_back(pwave.path);
	}

	/* Then load up channels, actions and global properties. */

	for (const Patch::Channel& pchannel : patch.channels)
	{
		Wave*                wave    = findWave(pchannel.waveId);
		std::vector<Plugin*> plugins = findPlugins(pchannel.pluginIds);
		channelFactory::Data data    = channelFactory::deserializeChannel(pchannel, sampleRateRatio, bufferSize, rsmpQuality, wave, plugins);
		document.channels.add(data.channel);
		getAllChannelsShared().push_back(std::move(data.shared));
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

	DataLock lock = lockData(SwapType::NONE);

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

DataLock Model::lockData(SwapType t)
{
	return DataLock(*this, t);
}

/* -------------------------------------------------------------------------- */

bool Model::isLocked() const
{
	return m_swapper.isRtLocked();
}

/* -------------------------------------------------------------------------- */

std::vector<std::unique_ptr<Wave>>&          Model::getAllWaves() { return m_shared.waves; };
std::vector<std::unique_ptr<Plugin>>&        Model::getAllPlugins() { return m_shared.plugins; }
std::vector<std::unique_ptr<ChannelShared>>& Model::getAllChannelsShared() { return m_shared.channelsShared; }

/* -------------------------------------------------------------------------- */

Plugin* Model::findPlugin(ID id) { return get_(m_shared.plugins, id); }
Wave*   Model::findWave(ID id) { return get_(m_shared.waves, id); }

/* -------------------------------------------------------------------------- */

Wave&          Model::addWave(std::unique_ptr<Wave> w) { return add_(m_shared.waves, std::move(w), *this); }
Plugin&        Model::addPlugin(std::unique_ptr<Plugin> p) { return add_(m_shared.plugins, std::move(p), *this); }
ChannelShared& Model::addChannelShared(std::unique_ptr<ChannelShared> cs) { return add_(m_shared.channelsShared, std::move(cs), *this); }

/* -------------------------------------------------------------------------- */

void Model::removePlugin(const Plugin& p) { remove_(m_shared.plugins, p, *this); }
void Model::removeWave(const Wave& w) { remove_(m_shared.waves, w, *this); }

/* -------------------------------------------------------------------------- */

void Model::clearPlugins() { clear_(m_shared.plugins, *this); }
void Model::clearWaves() { clear_(m_shared.waves, *this); }

/* -------------------------------------------------------------------------- */

std::vector<Plugin*> Model::findPlugins(std::vector<ID> pluginIds)
{
	std::vector<Plugin*> out;
	for (ID id : pluginIds)
	{
		Plugin* plugin = findPlugin(id);
		if (plugin != nullptr)
			out.push_back(plugin);
	}
	return out;
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

	puts("model::channelsShared");

	for (int i = 0; const auto& c : m_shared.channelsShared)
	{
		fmt::print("\t{}) - {}\n", i++, (void*)c.get());
	}

	puts("model::shared.waves");

	for (int i = 0; const auto& w : m_shared.waves)
		fmt::print("\t{}) {} - ID={} name='{}'\n", i++, (void*)w.get(), w->id, w->getPath());

	puts("model::shared.plugins");

	for (int i = 0; const auto& p : m_shared.plugins)
		fmt::print("\t{}) {} - ID={}\n", i++, (void*)p.get(), p->id);
}

#endif // G_DEBUG_MODE
} // namespace giada::m::model
