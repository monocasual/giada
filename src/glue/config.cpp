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
namespace
{
AudioDeviceData getAudioDeviceData_(DeviceType type, size_t index, int channelsCount, int channelsStart)
{
	for (const m::kernelAudio::Device& device : m::kernelAudio::getDevices())
		if (device.index == index)
			return AudioDeviceData(type, device, channelsCount, channelsStart);
	return AudioDeviceData();
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

AudioDeviceData::AudioDeviceData(DeviceType type, const m::kernelAudio::Device& device,
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
			audioData.outputDevices.push_back(AudioDeviceData(DeviceType::OUTPUT, device, G_MAX_IO_CHANS, 0));
		if (device.maxInputChannels > 0)
			audioData.inputDevices.push_back(AudioDeviceData(DeviceType::INPUT, device, 1, 0));
	}

	audioData.api             = m::conf::conf.soundSystem;
	audioData.bufferSize      = m::conf::conf.buffersize;
	audioData.sampleRate      = m::conf::conf.samplerate;
	audioData.limitOutput     = m::conf::conf.limitOutput;
	audioData.recTriggerLevel = m::conf::conf.recTriggerLevel;
	audioData.resampleQuality = m::conf::conf.rsmpQuality;
	audioData.outputDevice    = getAudioDeviceData_(DeviceType::OUTPUT,
        m::conf::conf.soundDeviceOut, m::conf::conf.channelsOutCount,
        m::conf::conf.channelsOutStart);
	audioData.inputDevice     = getAudioDeviceData_(DeviceType::INPUT,
        m::conf::conf.soundDeviceIn, m::conf::conf.channelsInCount,
        m::conf::conf.channelsInStart);

	return audioData;
}

/* -------------------------------------------------------------------------- */

void save(const AudioData& data)
{
	m::conf::conf.soundSystem      = data.api;
	m::conf::conf.soundDeviceOut   = data.outputDevice.index;
	m::conf::conf.soundDeviceIn    = data.inputDevice.index;
	m::conf::conf.channelsOutCount = data.outputDevice.channelsCount;
	m::conf::conf.channelsOutStart = data.outputDevice.channelsStart;
	m::conf::conf.channelsInCount  = data.inputDevice.channelsCount;
	m::conf::conf.channelsInStart  = data.inputDevice.channelsStart;
	m::conf::conf.limitOutput      = data.limitOutput;
	m::conf::conf.rsmpQuality      = data.resampleQuality;
	m::conf::conf.buffersize       = data.bufferSize;
	m::conf::conf.recTriggerLevel  = data.recTriggerLevel;
	m::conf::conf.samplerate       = data.sampleRate;
}
} // namespace giada::c::config