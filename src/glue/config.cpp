/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/glue/config.h"
#include "src/core/conf.h"
#include "src/core/const.h"
#include "src/core/engine.h"
#include "src/core/kernelAudio.h"
#include "src/core/kernelMidi.h"
#include "src/deps/rtaudio/RtAudio.h"
#include "src/gui/dialogs/browser/browserDir.h"
#include "src/gui/dialogs/config.h"
#include "src/gui/dialogs/warnings.h"
#include "src/gui/elems/config/tabPlugins.h"
#include "src/gui/ui.h"
#include "src/utils/fs.h"
#include "src/utils/vector.h"
#include <fmt/core.h>

extern giada::v::Ui*     g_ui;
extern giada::m::Engine* g_engine;

namespace giada::c::config
{
namespace
{
void printMidiErrorIfAny_(const m::KernelMidi::Result& result)
{
	if (result.success)
		return;

	const std::string message = fmt::format("{}\n\n{}",
	    g_ui->getI18Text(v::LangMap::CONFIG_MIDI_LABEL_WRONGMIDI),
	    result.message);

	v::gdAlert(message.c_str(), /*resizable=*/true);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

AudioDeviceData::AudioDeviceData(DeviceType type, const m::KernelAudio::Device& device)
: type(type)
, id(device.id)
, name(device.name)
, channelsMax(type == DeviceType::OUTPUT ? device.maxOutputChannels : device.maxInputChannels)
, sampleRates(device.sampleRates)
, selectedChannelsCount(channelsMax)
, selectedChannelsStart(device.channelsStart)
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void AudioData::setOutputDevice(int id)
{
	for (AudioDeviceData& d : availableOutputDevices)
	{
		if (id != d.id)
			continue;
		selectedOutputDevice = d;
	}
}

/* -------------------------------------------------------------------------- */

void AudioData::setInputDevice(int id)
{
	for (AudioDeviceData& d : availableInputDevices)
	{
		if (id == d.id)
		{
			selectedInputDevice = d;
			return;
		}
	}
	selectedInputDevice = {};
}

/* -------------------------------------------------------------------------- */

void AudioData::toggleInputDevice(bool v)
{
	if (availableInputDevices.empty())
		return;
	setInputDevice(v ? availableInputDevices[0].id : 0);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string MidiData::getMidiMapByIndex(int i)
{
	return u::vector::atOr(availableMidiMaps, i, "");
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

AudioData getAudioData()
{
	AudioData audioData;

	audioData.availableApis[RtAudio::Api::RTAUDIO_DUMMY] = "(Dummy)";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::LINUX_ALSA))
		audioData.availableApis[RtAudio::Api::LINUX_ALSA] = "ALSA";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::UNIX_JACK))
		audioData.availableApis[RtAudio::Api::UNIX_JACK] = "JACK";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::LINUX_PULSE))
		audioData.availableApis[RtAudio::Api::LINUX_PULSE] = "PulseAudio";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::WINDOWS_DS))
		audioData.availableApis[RtAudio::Api::WINDOWS_DS] = "DirectSound";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::WINDOWS_ASIO))
		audioData.availableApis[RtAudio::Api::WINDOWS_ASIO] = "ASIO";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::WINDOWS_WASAPI))
		audioData.availableApis[RtAudio::Api::WINDOWS_WASAPI] = "WASAPI";
	if (g_engine->getConfigApi().audio_hasAPI(RtAudio::Api::MACOSX_CORE))
		audioData.availableApis[RtAudio::Api::MACOSX_CORE] = "CoreAudio";

	for (const m::KernelAudio::Device& device : g_engine->getConfigApi().audio_getAvailableDevices())
	{
		if (device.maxOutputChannels > 0)
			audioData.availableOutputDevices.push_back(AudioDeviceData(DeviceType::OUTPUT, device));
		if (device.maxInputChannels > 0)
			audioData.availableInputDevices.push_back(AudioDeviceData(DeviceType::INPUT, device));
	}

	audioData.selectedApi             = g_engine->getConfigApi().audio_getAPI();
	audioData.selectedBufferSize      = g_engine->getConfigApi().audio_getBufferSize();
	audioData.selectedSampleRate      = g_engine->getConfigApi().audio_getSampleRate();
	audioData.selectedLimitOutput     = g_engine->getConfigApi().audio_isLimitOutput();
	audioData.selectedRecTriggerLevel = g_engine->getConfigApi().audio_getRecTriggerLevel();
	audioData.selectedResampleQuality = static_cast<int>(g_engine->getConfigApi().audio_getResamplerQuality());
	audioData.selectedOutputDevice    = AudioDeviceData(DeviceType::OUTPUT, g_engine->getConfigApi().audio_getCurrentOutDevice());
	audioData.selectedInputDevice     = AudioDeviceData(DeviceType::INPUT, g_engine->getConfigApi().audio_getCurrentInDevice());

	return audioData;
}

/* -------------------------------------------------------------------------- */

MidiData getMidiData()
{
	MidiData midiData;

	if (g_engine->getConfigApi().midi_hasAPI(RtMidi::Api::RTMIDI_DUMMY))
		midiData.availableApis[RtMidi::Api::RTMIDI_DUMMY] = "(Dummy)";
	if (g_engine->getConfigApi().midi_hasAPI(RtMidi::Api::LINUX_ALSA))
		midiData.availableApis[RtMidi::Api::LINUX_ALSA] = "ALSA";
	if (g_engine->getConfigApi().midi_hasAPI(RtMidi::Api::UNIX_JACK))
		midiData.availableApis[RtMidi::Api::UNIX_JACK] = "JACK";
	if (g_engine->getConfigApi().midi_hasAPI(RtMidi::Api::WINDOWS_MM))
		midiData.availableApis[RtMidi::Api::WINDOWS_MM] = "Multimedia MIDI";
	if (g_engine->getConfigApi().midi_hasAPI(RtMidi::Api::MACOSX_CORE))
		midiData.availableApis[RtMidi::Api::MACOSX_CORE] = "OSX Core MIDI";

	midiData.availableSyncModes[G_MIDI_SYNC_NONE]         = "(disabled)";
	midiData.availableSyncModes[G_MIDI_SYNC_CLOCK_MASTER] = "MIDI Clock (master)";
	midiData.availableSyncModes[G_MIDI_SYNC_CLOCK_SLAVE]  = "MIDI Clock (slave)";

	midiData.availableMidiMaps   = g_engine->getConfigApi().midi_getMidiMapFilesFound();
	midiData.selectedMidiMap     = g_engine->getConfigApi().midi_getCurrentMidiMapPath();
	midiData.availableOutDevices = g_engine->getConfigApi().midi_getOutDevices();
	midiData.availableInDevices  = g_engine->getConfigApi().midi_getInDevices();
	midiData.selectedApi         = g_engine->getConfigApi().midi_getAPI();
	midiData.selectedSyncMode    = g_engine->getConfigApi().midi_getSyncMode();

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

bool openMidiDevice(DeviceType type, std::size_t index)
{
	auto&      api = g_engine->getConfigApi();
	const auto res = type == DeviceType::OUTPUT ? api.midi_openOutDevice(index) : api.midi_openInDevice(index);
	printMidiErrorIfAny_(res);
	return res.success;
}

/* -------------------------------------------------------------------------- */

void closeMidiDevice(DeviceType type, std::size_t index)
{
	auto& api = g_engine->getConfigApi();
	type == DeviceType::OUTPUT ? api.midi_closeOutDevice(index) : api.midi_closeInDevice(index);
}

/* -------------------------------------------------------------------------- */

void apply(const AudioData& data)
{
	bool res = g_engine->getConfigApi().audio_openStream(
	    {
	        data.selectedOutputDevice.id,
	        data.selectedOutputDevice.selectedChannelsCount,
	        data.selectedOutputDevice.selectedChannelsStart,
	    },
	    {
	        data.selectedInputDevice.id,
	        data.selectedInputDevice.selectedChannelsCount,
	        data.selectedInputDevice.selectedChannelsStart,
	    },
	    data.selectedSampleRate, data.selectedBufferSize);

	if (!res)
	{
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_INIT_WRONGSYSTEM));
		return;
	}

	g_engine->getConfigApi().audio_storeData(data.selectedLimitOutput,
	    static_cast<m::Resampler::Quality>(data.selectedResampleQuality), data.selectedRecTriggerLevel);
}

/* -------------------------------------------------------------------------- */

void save(const PluginData& data)
{
	g_ui->model.pluginPath = data.pluginPath;
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
