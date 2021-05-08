/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "config.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/kernelAudio.h"
#include "deps/rtaudio/RtAudio.h"

namespace giada::c::config
{
AudioDeviceData::AudioDeviceData(const m::kernelAudio::Device& device)
: index(device.index)
, name(device.name)
, maxOutputChannels(device.maxOutputChannels)
, maxInputChannels(device.maxInputChannels)
, sampleRates(device.sampleRates)
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

AudioData getAudioData()
{
	AudioData audioData;

	audioData.apis[G_SYS_API_NONE] = "(none)";

#if defined(G_OS_LINUX)

	if (m::kernelAudio::hasAPI(RtAudio::LINUX_ALSA))
		audioData.apis[G_SYS_API_ALSA] = "ALSA";
	if (m::kernelAudio::hasAPI(RtAudio::UNIX_JACK))
		audioData.apis[G_SYS_API_JACK] = "Jack";
	if (m::kernelAudio::hasAPI(RtAudio::LINUX_PULSE))
		audioData.apis[G_SYS_API_PULSE] = "PulseAudio";

#elif defined(G_OS_FREEBSD)

	if (m::kernelAudio::hasAPI(RtAudio::UNIX_JACK))
		audioData.apis[G_SYS_API_JACK] = "Jack";
	if (m::kernelAudio::hasAPI(RtAudio::LINUX_PULSE))
		audioData.apis[G_SYS_API_PULSE] = "PulseAudio";

#elif defined(G_OS_WINDOWS)

	if (m::kernelAudio::hasAPI(RtAudio::WINDOWS_DS))
		audioData.apis[G_SYS_API_DS] = "DirectSound";
	if (m::kernelAudio::hasAPI(RtAudio::WINDOWS_ASIO))
		audioData.apis[G_SYS_API_ASIO] = "ASIO";
	if (m::kernelAudio::hasAPI(RtAudio::WINDOWS_WASAPI))
		audioData.apis[G_SYS_API_WASAPI] = "WASAPI";

#elif defined(G_OS_MAC)

	if (m::kernelAudio::hasAPI(RtAudio::MACOSX_CORE))
		audioData.apis[G_SYS_API_CORE] = "CoreAudio";

#endif

	std::vector<m::kernelAudio::Device> devices = m::kernelAudio::getDevices();

	for (const m::kernelAudio::Device& device : devices)
	{
		if (device.maxOutputChannels > 0)
			audioData.outputDevices.push_back(AudioDeviceData(device));
		if (device.maxInputChannels > 0)
			audioData.inputDevices.push_back(AudioDeviceData(device));
	}

	audioData.api             = m::conf::conf.soundSystem;
	audioData.bufferSize      = m::conf::conf.buffersize;
	audioData.sampleRate      = m::conf::conf.samplerate;
	audioData.limitOutput     = m::conf::conf.limitOutput;
	audioData.recTriggerLevel = m::conf::conf.recTriggerLevel;
	audioData.resampleQuality = m::conf::conf.rsmpQuality;

	if (m::conf::conf.soundDeviceOut != -1)
		audioData.outputDevice = {
		    m::conf::conf.soundDeviceOut,
		    2, // TODO channels count shit
		    m::conf::conf.channelsOut,
		    devices[m::conf::conf.soundDeviceOut].maxOutputChannels};

	if (m::conf::conf.soundDeviceIn != -1)
		audioData.inputDevice = {
		    m::conf::conf.soundDeviceIn,
		    m::conf::conf.channelsInCount,
		    m::conf::conf.channelsInStart,
		    devices[m::conf::conf.soundDeviceIn].maxInputChannels};

	return audioData;
}

/* -------------------------------------------------------------------------- */

void save(const AudioData& data)
{
	m::conf::conf.soundSystem     = data.api;
	m::conf::conf.soundDeviceOut  = data.outputDevice.index;
	m::conf::conf.soundDeviceIn   = data.inputDevice.index;
	m::conf::conf.channelsOut     = data.outputDevice.channelsStart;
	m::conf::conf.channelsInCount = data.inputDevice.channelsCount;
	m::conf::conf.channelsInStart = data.inputDevice.channelsStart;
	m::conf::conf.limitOutput     = data.limitOutput;
	m::conf::conf.rsmpQuality     = data.resampleQuality;
	m::conf::conf.buffersize      = data.bufferSize;
	m::conf::conf.recTriggerLevel = data.recTriggerLevel;
	m::conf::conf.samplerate      = data.sampleRate;
}
} // namespace giada::c::config