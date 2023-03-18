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
KernelAudio::KernelAudio(model::Model& model)
: onAudioCallback(nullptr)
, m_model(model)
{
}

/* -------------------------------------------------------------------------- */

void KernelAudio::init()
{
	const model::KernelAudio& kernelAudio = m_model.get().kernelAudio;

	u::log::print("[KA] using API %d\n", kernelAudio.soundSystem);

	if (m_rtAudio != nullptr)
		closeDevice();

	m_rtAudio = std::make_unique<RtAudio>(kernelAudio.soundSystem);

	m_rtAudio->setErrorCallback([](RtAudioErrorType type, const std::string& msg) {
		u::log::print("[KA] RtAudio error %d: %s\n", type, msg.c_str());
	});
}

/* -------------------------------------------------------------------------- */

int KernelAudio::openDevice()
{
	assert(onAudioCallback != nullptr);
	assert(m_rtAudio != nullptr);

	/* Make a local copy of model::KernelAudio data: the model will be updated 
	later on with some changes, if the stream has been opened successfully. */

	model::KernelAudio kernelAudio = m_model.get().kernelAudio;

	u::log::print("[KA] Opening device out=%d, in=%d, samplerate=%d\n",
	    kernelAudio.soundDeviceOut, kernelAudio.soundDeviceIn, kernelAudio.samplerate);

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

	outParams.deviceId     = kernelAudio.soundDeviceOut == G_DEFAULT_SOUNDDEV_OUT ? m_rtAudio->getDefaultOutputDevice() : kernelAudio.soundDeviceOut;
	outParams.nChannels    = kernelAudio.channelsOutCount;
	outParams.firstChannel = kernelAudio.channelsOutStart;

	/* Input device can be disabled. Unlike the output, here we are using all
	channels and let the user choose which one to record from in the configuration
	panel. */

	if (kernelAudio.soundDeviceIn != -1)
	{
		inParams.deviceId     = kernelAudio.soundDeviceIn;
		inParams.nChannels    = kernelAudio.channelsInCount;
		inParams.firstChannel = kernelAudio.channelsInStart;
	}

	RtAudio::StreamOptions options;
	options.streamName      = G_APP_NAME;
	options.numberOfBuffers = 4; // TODO - wtf?

#ifdef WITH_AUDIO_JACK

	/* If JACK, use its own sample rate, not the one coming from the conf
	object. */

	if (kernelAudio.soundSystem == RtAudio::Api::UNIX_JACK)
	{
		assert(m_devices.size() > 0);
		assert(m_devices[0].sampleRates.size() > 0);

		kernelAudio.samplerate = m_devices[0].sampleRates[0];
		u::log::print("[KA] JACK in use, samplerate=%d\n", kernelAudio.samplerate);
	}

#endif

	m_callbackInfo = {
	    /* kernelAudio      = */ this,
	    /* ready            = */ true,
	    /* withJack         = */ getAPI() == RtAudio::Api::UNIX_JACK,
	    /* outBuf           = */ nullptr, // filled later on in audio callback
	    /* inBuf            = */ nullptr, // filled later on in audio callback
	    /* bufferSize       = */ 0,       // filled later on in audio callback
	    /* sampleRate       = */ kernelAudio.samplerate,
	    /* channelsOutCount = */ kernelAudio.channelsOutCount,
	    /* channelsInCount  = */ kernelAudio.channelsInCount};

	RtAudioErrorType res = m_rtAudio->openStream(
	    &outParams,                                            // output params
	    kernelAudio.soundDeviceIn != -1 ? &inParams : nullptr, // input params if inDevice is selected
	    RTAUDIO_FLOAT32,                                       // audio format
	    kernelAudio.samplerate,                                // sample rate
	    &kernelAudio.buffersize,                               // buffer size in bytes. Might be changed to the actual value used by the soundcard
	    &audioCallback,                                        // audio callback
	    &m_callbackInfo,                                       // user data passed to callback
	    &options);

	if (res == RtAudioErrorType::RTAUDIO_NO_ERROR)
	{
		kernelAudio.ready = true;

		m_model.get().kernelAudio = kernelAudio;
		m_model.swap(model::SwapType::NONE);
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

bool         KernelAudio::isReady() const { return m_model.get().kernelAudio.ready; }
unsigned int KernelAudio::getBufferSize() const { return m_model.get().kernelAudio.buffersize; }
int          KernelAudio::getSampleRate() const { return m_model.get().kernelAudio.samplerate; }
int          KernelAudio::getChannelsOutCount() const { return m_model.get().kernelAudio.channelsOutCount; }
int          KernelAudio::getChannelsInCount() const { return m_model.get().kernelAudio.channelsInCount; }
bool         KernelAudio::isInputEnabled() const { return m_model.get().kernelAudio.soundDeviceIn != -1; }

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

RtAudio::Api KernelAudio::getAPI() const { return m_model.get().kernelAudio.soundSystem; }

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
