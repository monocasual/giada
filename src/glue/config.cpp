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
#include <fmt/core.h>

extern giada::v::Ui*     g_ui;
extern giada::m::Engine* g_engine;

namespace giada::c::config
{
AudioDeviceData::AudioDeviceData(DeviceType type, const m::KernelAudio::Device& device)
: type(type)
, id(device.id)
, name(device.name)
, channelsMax(type == DeviceType::OUTPUT ? device.maxOutputChannels : device.maxInputChannels)
, sampleRates(device.sampleRates)
, channelsCount(device.channelsCount)
, channelsStart(device.channelsStart)
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void AudioData::setOutputDevice(unsigned int id)
{
	for (AudioDeviceData& d : outputDevices)
	{
		if (id != d.id)
			continue;
		outputDevice = d;
	}
}

/* -------------------------------------------------------------------------- */

void AudioData::setInputDevice(unsigned int id)
{
	for (AudioDeviceData& d : inputDevices)
	{
		if (id == d.id)
		{
			inputDevice = d;
			return;
		}
	}
	inputDevice = {};
}

/* -------------------------------------------------------------------------- */

void AudioData::toggleInputDevice(bool v)
{
	if (inputDevices.empty())
		return;
	setInputDevice(v ? inputDevices[0].id : 0);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string MidiData::getMidiMapByIndex(int i)
{
	return u::vector::atOr(midiMaps, i, "");
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

AudioData getAudioData()
{
	AudioData audioData;

	audioData.apis[RtAudio::Api::RTAUDIO_DUMMY] = "(Dummy)";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::LINUX_ALSA))
		audioData.apis[RtAudio::Api::LINUX_ALSA] = "ALSA";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::UNIX_JACK))
		audioData.apis[RtAudio::Api::UNIX_JACK] = "JACK";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::LINUX_PULSE))
		audioData.apis[RtAudio::Api::LINUX_PULSE] = "PulseAudio";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::WINDOWS_DS))
		audioData.apis[RtAudio::Api::WINDOWS_DS] = "DirectSound";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::WINDOWS_ASIO))
		audioData.apis[RtAudio::Api::WINDOWS_ASIO] = "ASIO";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::WINDOWS_WASAPI))
		audioData.apis[RtAudio::Api::WINDOWS_WASAPI] = "WASAPI";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::MACOSX_CORE))
		audioData.apis[RtAudio::Api::MACOSX_CORE] = "CoreAudio";

	for (const m::KernelAudio::Device& device : g_engine->getConfigApi().audio_getAvailableDevices())
	{
		if (device.maxOutputChannels > 0)
			audioData.outputDevices.push_back(AudioDeviceData(DeviceType::OUTPUT, device));
		if (device.maxInputChannels > 0)
			audioData.inputDevices.push_back(AudioDeviceData(DeviceType::INPUT, device));
	}

	audioData.api             = g_engine->getConfigApi().audio_getAPI();
	audioData.bufferSize      = g_engine->getConfigApi().audio_getBufferSize();
	audioData.sampleRate      = g_engine->getConfigApi().audio_getSampleRate();
	audioData.limitOutput     = g_engine->getConfigApi().audio_isLimitOutput();
	audioData.recTriggerLevel = g_engine->getConfigApi().audio_getRecTriggerLevel();
	audioData.resampleQuality = static_cast<int>(g_engine->getConfigApi().audio_getResamplerQuality());
	audioData.outputDevice    = AudioDeviceData(DeviceType::OUTPUT, g_engine->getConfigApi().audio_getCurrentOutDevice());
	audioData.inputDevice     = AudioDeviceData(DeviceType::INPUT, g_engine->getConfigApi().audio_getCurrentInDevice());

	return audioData;
}

/* -------------------------------------------------------------------------- */

MidiData getMidiData()
{
	MidiData midiData;

	if (g_engine->getConfigApi().midi_hasAPI(RtMidi::Api::RTMIDI_DUMMY))
		midiData.apis[RtMidi::Api::RTMIDI_DUMMY] = "(Dummy)";
	if (g_engine->getConfigApi().midi_hasAPI(RtMidi::Api::LINUX_ALSA))
		midiData.apis[RtMidi::Api::LINUX_ALSA] = "ALSA";
	if (g_engine->getConfigApi().midi_hasAPI(RtMidi::Api::UNIX_JACK))
		midiData.apis[RtMidi::Api::UNIX_JACK] = "JACK";
	if (g_engine->getConfigApi().midi_hasAPI(RtMidi::Api::WINDOWS_MM))
		midiData.apis[RtMidi::Api::WINDOWS_MM] = "Multimedia MIDI";
	if (g_engine->getConfigApi().midi_hasAPI(RtMidi::Api::MACOSX_CORE))
		midiData.apis[RtMidi::Api::MACOSX_CORE] = "OSX Core MIDI";

	midiData.syncModes[G_MIDI_SYNC_NONE]         = "(disabled)";
	midiData.syncModes[G_MIDI_SYNC_CLOCK_MASTER] = "MIDI Clock (master)";
	midiData.syncModes[G_MIDI_SYNC_CLOCK_SLAVE]  = "MIDI Clock (slave)";

	midiData.midiMaps = g_engine->getConfigApi().midi_getMidiMapFilesFound();
	midiData.midiMap  = g_engine->getConfigApi().midi_getCurrentMidiMapPath();
	midiData.outPorts = g_engine->getConfigApi().midi_getOutPorts();
	midiData.inPorts  = g_engine->getConfigApi().midi_getInPorts();
	midiData.api      = g_engine->getConfigApi().midi_getAPI();
	midiData.syncMode = g_engine->getConfigApi().midi_getSyncMode();
	midiData.outPort  = g_engine->getConfigApi().midi_getCurrentOutPort();
	midiData.inPort   = g_engine->getConfigApi().midi_getCurrentInPort();

	return midiData;
}

/* -------------------------------------------------------------------------- */

PluginData getPluginData()
{
	PluginData pluginData;
	pluginData.numAvailablePlugins = g_engine->getPluginsApi().countAvailablePlugins();
	pluginData.pluginPath          = g_ui->model.pluginPath;
	return pluginData;
}

/* -------------------------------------------------------------------------- */

MiscData getMiscData()
{
	MiscData miscData;
	miscData.logMode      = g_ui->model.logMode;
	miscData.showTooltips = g_ui->model.showTooltips;
	miscData.langMaps     = g_ui->getLangMapFilesFound();
	miscData.langMap      = g_ui->model.langMap;
	miscData.uiScaling    = g_ui->model.uiScaling;
	return miscData;
}
/* -------------------------------------------------------------------------- */

BehaviorsData getBehaviorsData()
{
	const m::model::Behaviors& behaviors = g_engine->getConfigApi().behaviors_getData();

	BehaviorsData behaviorsData = {
	    behaviors.chansStopOnSeqHalt,
	    behaviors.treatRecsAsLoops,
	    behaviors.inputMonitorDefaultOn,
	    behaviors.overdubProtectionDefaultOn};

	return behaviorsData;
}

/* -------------------------------------------------------------------------- */

void changeAudioAPI(RtAudio::Api api)
{
	g_engine->getConfigApi().audio_setAPI(api);
}

/* -------------------------------------------------------------------------- */

void changeMidiAPI(RtMidi::Api api)
{
	g_engine->getConfigApi().midi_setAPI(api);
}

/* -------------------------------------------------------------------------- */

void setMidiMapPath(const std::string& midiMapPath)
{
	g_engine->getConfigApi().midi_setMidiMapPath(midiMapPath);
}

/* -------------------------------------------------------------------------- */

void setMidiSyncMode(int syncMode)
{
	g_engine->getConfigApi().midi_setSyncMode(syncMode);
}

/* -------------------------------------------------------------------------- */

void apply(const AudioData& data)
{
	bool res = g_engine->getConfigApi().audio_openStream(
	    {
	        data.outputDevice.id,
	        data.outputDevice.channelsCount,
	        data.outputDevice.channelsStart,
	    },
	    {
	        data.inputDevice.id,
	        data.inputDevice.channelsCount,
	        data.inputDevice.channelsStart,
	    },
	    data.sampleRate, data.bufferSize);

	if (!res)
	{
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_INIT_WRONGSYSTEM));
		return;
	}

	g_engine->getConfigApi().audio_storeData(data.limitOutput,
	    static_cast<m::Resampler::Quality>(data.resampleQuality), data.recTriggerLevel);
}

/* -------------------------------------------------------------------------- */

void save(const PluginData& data)
{
	g_ui->model.pluginPath = data.pluginPath;
}

/* -------------------------------------------------------------------------- */

void apply(const MidiData& data)
{
	const m::KernelMidi::Result outRes = g_engine->getConfigApi().midi_openOutPort(data.outPort);
	const m::KernelMidi::Result inRes  = g_engine->getConfigApi().midi_openInPort(data.inPort);

	if (outRes.success && inRes.success)
		return;

	const std::string message = fmt::format("{}\n\n{}\n{}",
	    g_ui->getI18Text(v::LangMap::CONFIG_MIDI_LABEL_WRONGMIDI),
	    outRes.message,
	    inRes.message);

	v::gdAlert(message.c_str(), /*resizable=*/true);
}

/* -------------------------------------------------------------------------- */

void save(const MiscData& data)
{
	g_ui->model.logMode      = data.logMode;
	g_ui->model.showTooltips = data.showTooltips;
	g_ui->model.langMap      = data.langMap;
	g_ui->model.uiScaling    = std::clamp(data.uiScaling, G_MIN_UI_SCALING, G_MAX_UI_SCALING);
}

/* -------------------------------------------------------------------------- */

void save(const BehaviorsData& data)
{
	g_engine->getConfigApi().behaviors_storeData({data.chansStopOnSeqHalt,
	    data.treatRecsAsLoops,
	    data.inputMonitorDefaultOn,
	    data.overdubProtectionDefaultOn});
}

/* -------------------------------------------------------------------------- */

void scanPlugins(std::string dir, const std::function<bool(float)>& progress)
{
	g_engine->getPluginsApi().scan(dir, progress);
}

/* -------------------------------------------------------------------------- */

void setPluginPathCb(void* data)
{
	v::gdBrowserDir* browser    = static_cast<v::gdBrowserDir*>(data);
	std::string      pluginPath = g_ui->model.pluginPath;

	if (browser->getCurrentPath() == "")
	{
		v::gdAlert(g_ui->getI18Text(v::LangMap::CONFIG_PLUGINS_INVALIDPATH));
		return;
	}

	if (!pluginPath.empty() && pluginPath.back() != ';')
		pluginPath += ";";
	pluginPath += browser->getCurrentPath();

	g_ui->model.pluginPath = pluginPath;

	browser->do_callback();

	v::gdConfig* configWin = static_cast<v::gdConfig*>(g_ui->getSubwindow(WID_CONFIG));
	configWin->tabPlugins->rebuild();
}
} // namespace giada::c::config
