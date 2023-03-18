/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * KernelAudio
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

#include "core/kernelAudio.h"
#include "core/const.h"
#include "core/model/kernelAudio.h"
#include "core/model/model.h"
#include "utils/log.h"
#include "utils/vector.h"
#include <cassert>
#include <cstddef>

namespace giada::m
{
KernelAudio::KernelAudio()
: onAudioCallback(nullptr)
, m_ready(false)
, m_inputEnabled(false)
, m_realBufferSize(0)
, m_realSampleRate(0)
, m_channelsOutCount(0)
, m_channelsInCount(0)
{
}

/* -------------------------------------------------------------------------- */

int KernelAudio::openDevice(const model::KernelAudio& model)
{
	assert(onAudioCallback != nullptr);

	u::log::print("[KA] using API %d\n", model.soundSystem);

	m_api     = model.soundSystem;
	m_rtAudio = std::make_unique<RtAudio>(m_api);

	m_rtAudio->setErrorCallback([](RtAudioErrorType type, const std::string& msg) {
		u::log::print("[KA] RtAudio error %d: %s\n", type, msg.c_str());
	});

	u::log::print("[KA] Opening device out=%d, in=%d, samplerate=%d\n",
	    model.soundDeviceOut, model.soundDeviceIn, model.samplerate);

	m_devices = fetchDevices();
	printDevices(m_devices);

	/* Abort here if devices found are zero. */

	if (m_devices.size() == 0)
	{
		closeDevice();
		return 0;
	}

	RtAudio::StreamParameters outParams;
	RtAudio::StreamParameters inParams;

	outParams.deviceId     = model.soundDeviceOut == G_DEFAULT_SOUNDDEV_OUT ? m_rtAudio->getDefaultOutputDevice() : model.soundDeviceOut;
	outParams.nChannels    = model.channelsOutCount;
	outParams.firstChannel = model.channelsOutStart;

	/* Input device can be disabled. Unlike the output, here we are using all
	channels and let the user choose which one to record from in the configuration
	panel. */

	if (model.soundDeviceIn != -1)
	{
		inParams.deviceId     = model.soundDeviceIn;
		inParams.nChannels    = model.channelsInCount;
		inParams.firstChannel = model.channelsInStart;
		m_inputEnabled.store(true);
	}
	else
		m_inputEnabled.store(false);

	RtAudio::StreamOptions options;
	options.streamName      = G_APP_NAME;
	options.numberOfBuffers = 4; // TODO - wtf?

	m_realBufferSize   = model.buffersize;
	m_realSampleRate   = model.samplerate;
	m_channelsOutCount = model.channelsOutCount;
	m_channelsInCount  = model.channelsInCount;

#ifdef WITH_AUDIO_JACK

	/* If JACK, use its own sample rate, not the one coming from the conf
	object. */

	if (m_api == RtAudio::Api::UNIX_JACK)
	{
		assert(m_devices.size() > 0);
		assert(m_devices[0].sampleRates.size() > 0);

		m_realSampleRate = m_devices[0].sampleRates[0];
		u::log::print("[KA] JACK in use, samplerate=%d\n", m_realSampleRate);
	}

#endif

	m_callbackInfo = {
	    /* kernelAudio      = */ this,
	    /* ready            = */ true,
	    /* withJack         = */ getAPI() == RtAudio::Api::UNIX_JACK,
	    /* outBuf           = */ nullptr, // filled later on in audio callback
	    /* inBuf            = */ nullptr, // filled later on in audio callback
	    /* bufferSize       = */ 0,       // filled later on in audio callback
	    /* sampleRate       = */ m_realSampleRate,
	    /* channelsOutCount = */ m_channelsOutCount,
	    /* channelsInCount  = */ m_channelsInCount};

	RtAudioErrorType res = m_rtAudio->openStream(
	    &outParams,                                      // output params
	    model.soundDeviceIn != -1 ? &inParams : nullptr, // input params if inDevice is selected
	    RTAUDIO_FLOAT32,                                 // audio format
	    m_realSampleRate,                                // sample rate
	    &m_realBufferSize,                               // buffer size in byte
	    &audioCallback,                                  // audio callback
	    &m_callbackInfo,                                 // user data passed to callback
	    &options);

	if (res == RtAudioErrorType::RTAUDIO_NO_ERROR)
	{
		m_ready.store(true);
		return 1;
	}
	else
	{
		closeDevice();
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

int KernelAudio::startStream()
{
	if (m_rtAudio->startStream() == RtAudioErrorType::RTAUDIO_NO_ERROR)
	{
		u::log::print("[KA] Start stream - latency = %lu\n", m_rtAudio->getStreamLatency());
		return 1;
	}
	return 0;
}

/* -------------------------------------------------------------------------- */

int KernelAudio::stopStream()
{
	if (m_rtAudio->stopStream() == RtAudioErrorType::RTAUDIO_NO_ERROR)
	{
		u::log::print("[KA] Stop stream\n");
		return 1;
	}
	return 0;
}

/* -------------------------------------------------------------------------- */

void KernelAudio::closeDevice()
{
	if (!m_rtAudio->isStreamOpen())
		return;
	m_rtAudio->stopStream();
	m_rtAudio->closeStream();
	m_rtAudio.reset(nullptr);
}

/* -------------------------------------------------------------------------- */

bool KernelAudio::isReady() const
{
	return m_ready.load();
}

/* -------------------------------------------------------------------------- */

int  KernelAudio::getBufferSize() const { return static_cast<int>(m_realBufferSize); }
int  KernelAudio::getSampleRate() const { return m_realSampleRate; }
int  KernelAudio::getChannelsOutCount() const { return m_channelsOutCount; }
int  KernelAudio::getChannelsInCount() const { return m_channelsInCount; }
bool KernelAudio::isInputEnabled() const { return m_inputEnabled.load(); }

/* -------------------------------------------------------------------------- */

const std::vector<m::KernelAudio::Device>& KernelAudio::getDevices() const
{
	return m_devices;
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_AUDIO_JACK
jack_client_t* KernelAudio::getJackHandle() const
{
	return static_cast<jack_client_t*>(m_rtAudio->HACK__getJackClient());
}
#endif

/* -------------------------------------------------------------------------- */

bool KernelAudio::hasAPI(int API) const
{
	std::vector<RtAudio::Api> APIs;
	RtAudio::getCompiledApi(APIs);
	for (unsigned i = 0; i < APIs.size(); i++)
		if (APIs.at(i) == API)
			return true;
	return false;
}

RtAudio::Api KernelAudio::getAPI() const { return m_api; }

/* -------------------------------------------------------------------------- */

void KernelAudio::logCompiledAPIs()
{
	std::vector<RtAudio::Api> APIs;
	RtAudio::getCompiledApi(APIs);

	u::log::print("[KA] Compiled RtAudio APIs: %d\n", APIs.size());

	for (const RtAudio::Api& api : APIs)
	{
		switch (api)
		{
		case RtAudio::Api::LINUX_ALSA:
			u::log::print("  ALSA\n");
			break;
		case RtAudio::Api::LINUX_PULSE:
			u::log::print("  PulseAudio\n");
			break;
		case RtAudio::Api::UNIX_JACK:
			u::log::print("  JACK\n");
			break;
		case RtAudio::Api::MACOSX_CORE:
			u::log::print("  CoreAudio\n");
			break;
		case RtAudio::Api::WINDOWS_WASAPI:
			u::log::print("  WASAPI\n");
			break;
		case RtAudio::Api::WINDOWS_ASIO:
			u::log::print("  ASIO\n");
			break;
		case RtAudio::Api::WINDOWS_DS:
			u::log::print("  DirectSound\n");
			break;
		case RtAudio::Api::RTAUDIO_DUMMY:
			u::log::print("  Dummy\n");
			break;
		default:
			u::log::print("  (unknown)\n");
			break;
		}
	}
}

/* -------------------------------------------------------------------------- */

m::KernelAudio::Device KernelAudio::fetchDevice(size_t deviceIndex) const
{
	RtAudio::DeviceInfo info = m_rtAudio->getDeviceInfo(deviceIndex);

	if (!info.probed)
	{
		u::log::print("[KA] Can't probe device %d\n", deviceIndex);
		return {deviceIndex};
	}

	return {
	    deviceIndex,
	    true,
	    info.name,
	    static_cast<int>(info.outputChannels),
	    static_cast<int>(info.inputChannels),
	    static_cast<int>(info.duplexChannels),
	    info.isDefaultOutput,
	    info.isDefaultInput,
	    u::vector::cast<int>(info.sampleRates)};
}

/* -------------------------------------------------------------------------- */

std::vector<m::KernelAudio::Device> KernelAudio::fetchDevices() const
{
	std::vector<Device> out;
	for (unsigned i = 0; i < m_rtAudio->getDeviceCount(); i++)
		out.push_back(fetchDevice(i));
	return out;
}

/* -------------------------------------------------------------------------- */

void KernelAudio::printDevices(const std::vector<m::KernelAudio::Device>& devices) const
{
	u::log::print("[KA] %d device(s) found\n", devices.size());
	for (const m::KernelAudio::Device& d : devices)
	{
		u::log::print("  %d) %s\n", d.index, d.name);
		u::log::print("      ins=%d outs=%d duplex=%d\n", d.maxInputChannels, d.maxOutputChannels, d.maxDuplexChannels);
		u::log::print("      isDefaultOut=%d isDefaultIn=%d\n", d.isDefaultOut, d.isDefaultIn);
		u::log::print("      sampleRates:\n\t");
		for (int s : d.sampleRates)
			u::log::print("%d ", s);
		u::log::print("\n");
	}
}

/* -------------------------------------------------------------------------- */

int KernelAudio::audioCallback(void* outBuf, void* inBuf, unsigned bufferSize,
    double /*streamTime*/, RtAudioStreamStatus /*status*/, void*   data)
{
	CallbackInfo info = *static_cast<CallbackInfo*>(data);
	info.outBuf       = outBuf;
	info.inBuf        = inBuf;
	info.bufferSize   = bufferSize;
	return info.kernelAudio->onAudioCallback(info);
}
} // namespace giada::m
