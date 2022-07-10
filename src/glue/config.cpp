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
#include <FL/Fl_Tooltip.H>
#include <cstddef>

extern giada::v::Ui     g_ui;
extern giada::m::Engine g_engine;

namespace giada::c::config
{
namespace
{
AudioDeviceData getAudioDeviceData_(DeviceType type, size_t index, int channelsCount, int channelsStart)
{
	for (const m::KernelAudio::Device& device : g_engine.kernelAudio.getDevices())
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

	audioData.apis[G_SYS_API_NONE] = "(none)";

#if defined(G_OS_LINUX)

	if (g_engine.kernelAudio.hasAPI(RtAudio::LINUX_ALSA))
		audioData.apis[G_SYS_API_ALSA] = "ALSA";
	if (g_engine.kernelAudio.hasAPI(RtAudio::UNIX_JACK))
		audioData.apis[G_SYS_API_JACK] = "JACK";
	if (g_engine.kernelAudio.hasAPI(RtAudio::LINUX_PULSE))
		audioData.apis[G_SYS_API_PULSE] = "PulseAudio";

#elif defined(G_OS_FREEBSD)

	if (g_engine.kernelAudio.hasAPI(RtAudio::UNIX_JACK))
		audioData.apis[G_SYS_API_JACK] = "JACK";
	if (g_engine.kernelAudio.hasAPI(RtAudio::LINUX_PULSE))
		audioData.apis[G_SYS_API_PULSE] = "PulseAudio";

#elif defined(G_OS_WINDOWS)

	if (g_engine.kernelAudio.hasAPI(RtAudio::WINDOWS_DS))
		audioData.apis[G_SYS_API_DS] = "DirectSound";
	if (g_engine.kernelAudio.hasAPI(RtAudio::WINDOWS_ASIO))
		audioData.apis[G_SYS_API_ASIO] = "ASIO";
	if (g_engine.kernelAudio.hasAPI(RtAudio::WINDOWS_WASAPI))
		audioData.apis[G_SYS_API_WASAPI] = "WASAPI";

#elif defined(G_OS_MAC)

	if (g_engine.kernelAudio.hasAPI(RtAudio::MACOSX_CORE))
		audioData.apis[G_SYS_API_CORE] = "CoreAudio";

#endif

	std::vector<m::KernelAudio::Device> devices = g_engine.kernelAudio.getDevices();

	for (const m::KernelAudio::Device& device : devices)
	{
		if (device.maxOutputChannels > 0)
			audioData.outputDevices.push_back(AudioDeviceData(DeviceType::OUTPUT, device, G_MAX_IO_CHANS, 0));
		if (device.maxInputChannels > 0)
			audioData.inputDevices.push_back(AudioDeviceData(DeviceType::INPUT, device, 1, 0));
	}

	audioData.api             = g_engine.conf.data.soundSystem;
	audioData.bufferSize      = g_engine.conf.data.buffersize;
	audioData.sampleRate      = g_engine.conf.data.samplerate;
	audioData.limitOutput     = g_engine.conf.data.limitOutput;
	audioData.recTriggerLevel = g_engine.conf.data.recTriggerLevel;
	audioData.resampleQuality = g_engine.conf.data.rsmpQuality;
	audioData.outputDevice    = getAudioDeviceData_(DeviceType::OUTPUT,
        g_engine.conf.data.soundDeviceOut, g_engine.conf.data.channelsOutCount,
        g_engine.conf.data.channelsOutStart);
	audioData.inputDevice     = getAudioDeviceData_(DeviceType::INPUT,
        g_engine.conf.data.soundDeviceIn, g_engine.conf.data.channelsInCount,
        g_engine.conf.data.channelsInStart);

	return audioData;
}

/* -------------------------------------------------------------------------- */

MidiData getMidiData()
{
	MidiData midiData;

#if defined(G_OS_LINUX)

	if (g_engine.kernelMidi.hasAPI(RtMidi::LINUX_ALSA))
		midiData.apis[G_MIDI_API_ALSA] = "ALSA";
	if (g_engine.kernelMidi.hasAPI(RtMidi::UNIX_JACK))
		midiData.apis[G_MIDI_API_JACK] = "JACK";

#elif defined(G_OS_FREEBSD)

	if (g_engine.kernelMidi.hasAPI(RtMidi::UNIX_JACK))
		midiData.apis[G_MIDI_API_JACK] = "JACK";

#elif defined(G_OS_WINDOWS)

	if (g_engine.kernelMidi.hasAPI(RtMidi::WINDOWS_MM))
		midiData.apis[G_MIDI_API_MM] = "Multimedia MIDI";

#elif defined(G_OS_MAC)

	if (g_engine.kernelMidi.hasAPI(RtMidi::MACOSX_CORE))
		midiData.apis[G_MIDI_API_CORE] = "OSX Core MIDI";

#endif

	midiData.syncModes[G_MIDI_SYNC_NONE]    = "(disabled)";
	midiData.syncModes[G_MIDI_SYNC_CLOCK_M] = "MIDI Clock (master)";
	midiData.syncModes[G_MIDI_SYNC_MTC_M]   = "MTC (master)";

	midiData.midiMaps = g_engine.midiMapper.getMapFilesFound();
	midiData.midiMap  = u::vector::indexOf(midiData.midiMaps, g_engine.conf.data.midiMapPath);

	for (unsigned i = 0; i < g_engine.kernelMidi.countOutPorts(); i++)
		midiData.outPorts.push_back(g_engine.kernelMidi.getOutPortName(i));
	for (unsigned i = 0; i < g_engine.kernelMidi.countInPorts(); i++)
		midiData.inPorts.push_back(g_engine.kernelMidi.getInPortName(i));

	midiData.api      = g_engine.conf.data.midiSystem;
	midiData.syncMode = g_engine.conf.data.midiSync;
	midiData.outPort  = g_engine.conf.data.midiPortOut;
	midiData.inPort   = g_engine.conf.data.midiPortIn;

	return midiData;
}

/* -------------------------------------------------------------------------- */

PluginData getPluginData()
{
	PluginData pluginData;
	pluginData.numAvailablePlugins = g_engine.pluginManager.countAvailablePlugins();
	pluginData.pluginPath          = g_engine.conf.data.pluginPath;
	return pluginData;
}

/* -------------------------------------------------------------------------- */

MiscData getMiscData()
{
	MiscData miscData;
	miscData.logMode      = g_engine.conf.data.logMode;
	miscData.showTooltips = g_engine.conf.data.showTooltips;
	miscData.langMaps     = g_ui.langMapper.getMapFilesFound();
	miscData.langMap      = g_engine.conf.data.langMap;
	return miscData;
}
/* -------------------------------------------------------------------------- */

void save(const AudioData& data)
{
	g_engine.conf.data.soundSystem      = data.api;
	g_engine.conf.data.soundDeviceOut   = data.outputDevice.index;
	g_engine.conf.data.soundDeviceIn    = data.inputDevice.index;
	g_engine.conf.data.channelsOutCount = data.outputDevice.channelsCount;
	g_engine.conf.data.channelsOutStart = data.outputDevice.channelsStart;
	g_engine.conf.data.channelsInCount  = data.inputDevice.channelsCount;
	g_engine.conf.data.channelsInStart  = data.inputDevice.channelsStart;
	g_engine.conf.data.limitOutput      = data.limitOutput;
	g_engine.conf.data.rsmpQuality      = data.resampleQuality;
	g_engine.conf.data.buffersize       = data.bufferSize;
	g_engine.conf.data.recTriggerLevel  = data.recTriggerLevel;
	g_engine.conf.data.samplerate       = data.sampleRate;
	g_engine.updateMixerModel();
}

/* -------------------------------------------------------------------------- */

void save(const PluginData& data)
{
	g_engine.conf.data.pluginPath = data.pluginPath;
}

/* -------------------------------------------------------------------------- */

void save(const MidiData& data)
{
	g_engine.conf.data.midiSystem  = data.api;
	g_engine.conf.data.midiPortOut = data.outPort;
	g_engine.conf.data.midiPortIn  = data.inPort;
	//g_engine.conf.data.midiMapPath = data.midiMap >= 0 && data.midiMap < (int)data.midiMaps.size() ? data.midiMaps[data.midiMap] : "";
	g_engine.conf.data.midiMapPath = u::vector::atOr(data.midiMaps, data.midiMap, "");
	g_engine.conf.data.midiSync    = data.syncMode;
}

/* -------------------------------------------------------------------------- */

void save(const MiscData& data)
{
	g_engine.conf.data.logMode      = data.logMode;
	g_engine.conf.data.showTooltips = data.showTooltips;
	g_engine.conf.data.langMap      = data.langMap;
	Fl_Tooltip::enable(g_engine.conf.data.showTooltips); // TODO - move this to UI init
}

/* -------------------------------------------------------------------------- */

void scanPlugins(std::string dir, const std::function<void(float)>& progress)
{
	g_engine.pluginManager.scanDirs(dir, progress);
	g_engine.pluginManager.saveList(u::fs::join(u::fs::getHomePath(), "plugins.xml"));
}

/* -------------------------------------------------------------------------- */

void setPluginPathCb(void* data)
{
	v::gdBrowserDir* browser    = static_cast<v::gdBrowserDir*>(data);
	std::string&     pluginPath = g_engine.conf.data.pluginPath;

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
