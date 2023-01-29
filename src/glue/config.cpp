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

#include "glue/config.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/engine.h"
#include "core/kernelAudio.h"
#include "core/kernelMidi.h"
#include "core/midiMapper.h"
#include "core/plugins/pluginManager.h"
#include "deps/rtaudio/RtAudio.h"
#include "gui/dialogs/browser/browserDir.h"
#include "gui/dialogs/config.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/config/tabPlugins.h"
#include "gui/ui.h"
#include "utils/fs.h"
#include "utils/vector.h"
#include <cstddef>

extern giada::v::Ui     g_ui;
extern giada::m::Engine g_engine;

namespace giada::c::config
{
namespace
{
AudioDeviceData getAudioDeviceData_(DeviceType type, size_t index, int channelsCount, int channelsStart)
{
	for (const m::KernelAudio::Device& device : g_engine.getAudioDevices())
		if (device.index == index)
			return AudioDeviceData(type, device, channelsCount, channelsStart);
	return AudioDeviceData();
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

AudioDeviceData::AudioDeviceData(DeviceType type, const m::KernelAudio::Device& device,
    int channelsCount, int channelsStart)
: type(type)
, index(device.index)
, name(device.name)
, channelsMax(type == DeviceType::OUTPUT ? device.maxOutputChannels : device.maxInputChannels)
, sampleRates(device.sampleRates)
, channelsCount(channelsCount)
, channelsStart(channelsStart)
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void AudioData::setOutputDevice(int index)
{
	for (AudioDeviceData& d : outputDevices)
	{
		if (index != d.index)
			continue;
		outputDevice = d;
	}
}

/* -------------------------------------------------------------------------- */

void AudioData::setInputDevice(int index)
{
	for (AudioDeviceData& d : inputDevices)
	{
		if (index == d.index)
		{
			inputDevice = d;
			return;
		}
	}
	inputDevice = {};
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

AudioData getAudioData()
{
	AudioData audioData;

	audioData.apis[RtAudio::Api::RTAUDIO_DUMMY] = "(Dummy)";
	if (g_engine.hasAudioAPI(RtAudio::Api::LINUX_ALSA))
		audioData.apis[RtAudio::Api::LINUX_ALSA] = "ALSA";
	if (g_engine.hasAudioAPI(RtAudio::Api::UNIX_JACK))
		audioData.apis[RtAudio::Api::UNIX_JACK] = "JACK";
	if (g_engine.hasAudioAPI(RtAudio::Api::LINUX_PULSE))
		audioData.apis[RtAudio::Api::LINUX_PULSE] = "PulseAudio";
	if (g_engine.hasAudioAPI(RtAudio::Api::WINDOWS_DS))
		audioData.apis[RtAudio::Api::WINDOWS_DS] = "DirectSound";
	if (g_engine.hasAudioAPI(RtAudio::Api::WINDOWS_ASIO))
		audioData.apis[RtAudio::Api::WINDOWS_ASIO] = "ASIO";
	if (g_engine.hasAudioAPI(RtAudio::Api::WINDOWS_WASAPI))
		audioData.apis[RtAudio::Api::WINDOWS_WASAPI] = "WASAPI";
	if (g_engine.hasAudioAPI(RtAudio::Api::MACOSX_CORE))
		audioData.apis[RtAudio::Api::MACOSX_CORE] = "CoreAudio";

	for (const m::KernelAudio::Device& device : g_engine.getAudioDevices())
	{
		if (device.maxOutputChannels > 0)
			audioData.outputDevices.push_back(AudioDeviceData(DeviceType::OUTPUT, device, G_MAX_IO_CHANS, 0));
		if (device.maxInputChannels > 0)
			audioData.inputDevices.push_back(AudioDeviceData(DeviceType::INPUT, device, 1, 0));
	}

	audioData.api             = g_engine.getConf().soundSystem;
	audioData.bufferSize      = g_engine.getConf().buffersize;
	audioData.sampleRate      = g_engine.getConf().samplerate;
	audioData.limitOutput     = g_engine.getConf().limitOutput;
	audioData.recTriggerLevel = g_engine.getConf().recTriggerLevel;
	audioData.resampleQuality = static_cast<int>(g_engine.getConf().rsmpQuality);
	audioData.outputDevice    = getAudioDeviceData_(DeviceType::OUTPUT,
	       g_engine.getConf().soundDeviceOut, g_engine.getConf().channelsOutCount,
	       g_engine.getConf().channelsOutStart);
	audioData.inputDevice     = getAudioDeviceData_(DeviceType::INPUT,
	        g_engine.getConf().soundDeviceIn, g_engine.getConf().channelsInCount,
	        g_engine.getConf().channelsInStart);

	return audioData;
}

/* -------------------------------------------------------------------------- */

MidiData getMidiData()
{
	MidiData midiData;

	midiData.apis[RtMidi::Api::RTMIDI_DUMMY] = "(Dummy)";
	if (g_engine.hasMidiAPI(RtMidi::Api::LINUX_ALSA))
		midiData.apis[RtMidi::Api::LINUX_ALSA] = "ALSA";
	if (g_engine.hasMidiAPI(RtMidi::Api::UNIX_JACK))
		midiData.apis[RtMidi::Api::UNIX_JACK] = "JACK";
	if (g_engine.hasMidiAPI(RtMidi::Api::WINDOWS_MM))
		midiData.apis[RtMidi::Api::WINDOWS_MM] = "Multimedia MIDI";
	if (g_engine.hasMidiAPI(RtMidi::Api::MACOSX_CORE))
		midiData.apis[RtMidi::Api::MACOSX_CORE] = "OSX Core MIDI";

	midiData.syncModes[G_MIDI_SYNC_NONE]         = "(disabled)";
	midiData.syncModes[G_MIDI_SYNC_CLOCK_MASTER] = "MIDI Clock (master)";
	midiData.syncModes[G_MIDI_SYNC_CLOCK_SLAVE]  = "MIDI Clock (slave)";

	midiData.midiMaps = g_engine.getMidiMapFilesFound();
	midiData.midiMap  = u::vector::indexOf(midiData.midiMaps, g_engine.getConf().midiMapPath);
	midiData.outPorts = g_engine.getMidiOutPorts();
	midiData.inPorts  = g_engine.getMidiInPorts();
	midiData.api      = g_engine.getConf().midiSystem;
	midiData.syncMode = g_engine.getConf().midiSync;
	midiData.outPort  = g_engine.getConf().midiPortOut;
	midiData.inPort   = g_engine.getConf().midiPortIn;

	return midiData;
}

/* -------------------------------------------------------------------------- */

PluginData getPluginData()
{
	PluginData pluginData;
	pluginData.numAvailablePlugins = g_engine.getPluginsApi().countAvailablePlugins();
	pluginData.pluginPath          = g_engine.getConf().pluginPath;
	return pluginData;
}

/* -------------------------------------------------------------------------- */

MiscData getMiscData()
{
	MiscData miscData;
	miscData.logMode      = g_engine.getConf().logMode;
	miscData.showTooltips = g_engine.getConf().showTooltips;
	miscData.langMaps     = g_ui.langMapper.getMapFilesFound();
	miscData.langMap      = g_engine.getConf().langMap;
	miscData.uiScaling    = g_engine.getConf().uiScaling;
	return miscData;
}
/* -------------------------------------------------------------------------- */

void save(const AudioData& data)
{
	g_engine.getConf().soundSystem      = data.api;
	g_engine.getConf().soundDeviceOut   = data.outputDevice.index;
	g_engine.getConf().soundDeviceIn    = data.inputDevice.index;
	g_engine.getConf().channelsOutCount = data.outputDevice.channelsCount;
	g_engine.getConf().channelsOutStart = data.outputDevice.channelsStart;
	g_engine.getConf().channelsInCount  = data.inputDevice.channelsCount;
	g_engine.getConf().channelsInStart  = data.inputDevice.channelsStart;
	g_engine.getConf().limitOutput      = data.limitOutput;
	g_engine.getConf().rsmpQuality      = static_cast<m::Resampler::Quality>(data.resampleQuality);
	g_engine.getConf().buffersize       = data.bufferSize;
	g_engine.getConf().recTriggerLevel  = data.recTriggerLevel;
	g_engine.getConf().samplerate       = data.sampleRate;
	g_engine.updateMixerModel();
}

/* -------------------------------------------------------------------------- */

void save(const PluginData& data)
{
	g_engine.getConf().pluginPath = data.pluginPath;
}

/* -------------------------------------------------------------------------- */

void save(const MidiData& data)
{
	g_engine.getConf().midiSystem  = data.api;
	g_engine.getConf().midiPortOut = data.outPort;
	g_engine.getConf().midiPortIn  = data.inPort;
	g_engine.getConf().midiMapPath = u::vector::atOr(data.midiMaps, data.midiMap, "");
	g_engine.getConf().midiSync    = data.syncMode;
}

/* -------------------------------------------------------------------------- */

void save(const MiscData& data)
{
	g_engine.getConf().logMode      = data.logMode;
	g_engine.getConf().showTooltips = data.showTooltips;
	g_engine.getConf().langMap      = data.langMap;
	g_engine.getConf().uiScaling    = std::clamp(data.uiScaling, G_MIN_UI_SCALING, G_MAX_UI_SCALING);
}

/* -------------------------------------------------------------------------- */

void scanPlugins(std::string dir, const std::function<void(float)>& progress)
{
	g_engine.getPluginsApi().scan(dir, progress);
}

/* -------------------------------------------------------------------------- */

void setPluginPathCb(void* data)
{
	v::gdBrowserDir* browser    = static_cast<v::gdBrowserDir*>(data);
	std::string&     pluginPath = g_engine.getConf().pluginPath;

	if (browser->getCurrentPath() == "")
	{
		v::gdAlert(g_ui.langMapper.get(v::LangMap::CONFIG_PLUGINS_INVALIDPATH));
		return;
	}

	if (!pluginPath.empty() && pluginPath.back() != ';')
		pluginPath += ";";
	pluginPath += browser->getCurrentPath();

	browser->do_callback();

	v::gdConfig* configWin = static_cast<v::gdConfig*>(g_ui.getSubwindow(*g_ui.mainWindow.get(), WID_CONFIG));
	configWin->tabPlugins->rebuild();
}
} // namespace giada::c::config
