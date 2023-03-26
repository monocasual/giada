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
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "utils/log.h"
#include "utils/string.h"
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

void KernelAudio::init(RtAudio::Api api)
{
	u::log::print("[KA] using API %s\n", u::string::toString(api).c_str());

	if (m_rtAudio != nullptr)
		shutdown();

	m_rtAudio = std::make_unique<RtAudio>(api);

	m_rtAudio->setErrorCallback([](RtAudioErrorType type, const std::string& msg) {
		u::log::print("[KA] RtAudio error %d: %s\n", type, msg.c_str());
	});

	m_model.get().kernelAudio.soundSystem = api;
	m_model.swap(model::SwapType::NONE);

	printDevices(getAvailableDevices());
}

/* -------------------------------------------------------------------------- */

bool KernelAudio::openStream(const StreamInfo& info)
{
	assert(onAudioCallback != nullptr);
	assert(m_rtAudio != nullptr);

	u::log::print("[KA] Opening device out=%d, in=%d, samplerate=%d\n",
	    info.deviceOut, info.deviceIn, info.sampleRate);

	/* Abort here if devices found are zero. */

	if (m_rtAudio->getDeviceCount() == 0)
		return false;

	/* Close stream before opening another one. Closing a stream frees any 
	associated stream memory. */

	if (m_rtAudio->isStreamOpen())
		m_rtAudio->closeStream();

	RtAudio::StreamParameters outParams;
	RtAudio::StreamParameters inParams;

	outParams.deviceId     = info.deviceOut == G_DEFAULT_SOUNDDEV_OUT ? m_rtAudio->getDefaultOutputDevice() : info.deviceOut;
	outParams.nChannels    = info.channelsOutCount;
	outParams.firstChannel = info.channelsOutStart;

	/* Input device can be disabled. Unlike the output, here we are using all
	channels and let the user choose which one to record from in the configuration
	panel. */

	if (info.deviceIn != -1)
	{
		inParams.deviceId     = info.deviceIn;
		inParams.nChannels    = info.channelsInCount;
		inParams.firstChannel = info.channelsInStart;
	}

	RtAudio::StreamOptions options;
	options.streamName      = G_APP_NAME;
	options.numberOfBuffers = 4; // TODO - wtf?

	unsigned int actualSampleRate = info.sampleRate;
	unsigned int actualBufferSize = info.bufferSize;

	/* Take a mutable ref of model::KernelAudio data: the model will be updated 
	later on with some changes, if the stream has been opened successfully. */

	model::KernelAudio& kernelAudio = m_model.get().kernelAudio;

#ifdef WITH_AUDIO_JACK

	/* If JACK, use its own sample rate. */

	if (kernelAudio.soundSystem == RtAudio::Api::UNIX_JACK)
	{
		const Device jackDevice = fetchDevice(0); // JACK has only one device

		assert(jackDevice.probed);
		assert(jackDevice.sampleRates.size() > 0);

		const unsigned int jackSampleRate = jackDevice.sampleRates[0];

		u::log::print("[KA] JACK in use, samplerate=%d\n", jackSampleRate);

		actualSampleRate = jackSampleRate;
	}

	m_callbackInfo = {
	    /* rtAudio */ this,
	    /* channelsOutCount */ info.channelsOutCount,
	    /* channelsInCount */ info.channelsInCount};

#endif

	RtAudioErrorType res = m_rtAudio->openStream(
	    &outParams,                                // output params
	    info.deviceIn != -1 ? &inParams : nullptr, // input params if inDevice i selected
	    RTAUDIO_FLOAT32,                           // audio format
	    actualSampleRate,                          // sample rate
	    &actualBufferSize,                         // buffer size in bytes. Might be changed to the actual value used by the soundcard
	    &audioCallback,                            // audio callback
	    &m_callbackInfo,                           // user data passed to callback
	    &options);

	if (res != RtAudioErrorType::RTAUDIO_NO_ERROR)
		return false;

	kernelAudio.ready            = true;
	kernelAudio.soundDeviceOut   = info.deviceOut;
	kernelAudio.soundDeviceIn    = info.deviceIn;
	kernelAudio.channelsOutCount = info.channelsOutCount;
	kernelAudio.channelsOutStart = info.channelsOutStart;
	kernelAudio.channelsInCount  = info.channelsInCount;
	kernelAudio.channelsInStart  = info.channelsInStart;
	kernelAudio.samplerate       = actualSampleRate;
	kernelAudio.buffersize       = actualBufferSize;

	m_model.get().kernelAudio = kernelAudio;
	m_model.swap(model::SwapType::NONE);
	return true;
}

/* -------------------------------------------------------------------------- */

bool KernelAudio::startStream()
{
	if (m_rtAudio->startStream() == RtAudioErrorType::RTAUDIO_NO_ERROR)
	{
		u::log::print("[KA] Start stream - latency = %lu\n", m_rtAudio->getStreamLatency());
		return true;
	}
	return false;
}

/* -------------------------------------------------------------------------- */

bool KernelAudio::stopStream()
{
	if (m_rtAudio->stopStream() == RtAudioErrorType::RTAUDIO_NO_ERROR)
	{
		u::log::print("[KA] Stop stream\n");
		return true;
	}
	return false;
}

/* -------------------------------------------------------------------------- */

void KernelAudio::shutdown()
{
	if (m_rtAudio->isStreamRunning())
		m_rtAudio->stopStream();
	if (m_rtAudio->isStreamOpen())
		m_rtAudio->closeStream();
	m_rtAudio.reset(nullptr);
}

/* -------------------------------------------------------------------------- */

bool               KernelAudio::isReady() const { return m_model.get().kernelAudio.ready; }
unsigned int       KernelAudio::getBufferSize() const { return m_model.get().kernelAudio.buffersize; }
int                KernelAudio::getSampleRate() const { return m_model.get().kernelAudio.samplerate; }
int                KernelAudio::getChannelsOutCount() const { return m_model.get().kernelAudio.channelsOutCount; }
int                KernelAudio::getChannelsInCount() const { return m_model.get().kernelAudio.channelsInCount; }
bool               KernelAudio::isInputEnabled() const { return m_model.get().kernelAudio.soundDeviceIn != -1; }
bool               KernelAudio::isLimitOutput() const { return m_model.get().kernelAudio.limitOutput; }
float              KernelAudio::getRecTriggerLevel() const { return m_model.get().kernelAudio.recTriggerLevel; }
Resampler::Quality KernelAudio::getResamplerQuality() const { return m_model.get().kernelAudio.rsmpQuality; }

/* -------------------------------------------------------------------------- */

std::vector<m::KernelAudio::Device> KernelAudio::getAvailableDevices() const
{
	std::vector<Device> out;
	for (unsigned i = 0; i < m_rtAudio->getDeviceCount(); i++)
		out.push_back(fetchDevice(i));
	return out;
}

/* -------------------------------------------------------------------------- */

KernelAudio::Device KernelAudio::getCurrentOutDevice() const
{
	assert(m_rtAudio != nullptr);

	const model::KernelAudio& kernelAudio = m_model.get().kernelAudio;

	Device d        = fetchDevice(m_rtAudio->isStreamOpen() ? kernelAudio.deviceOut.index : m_rtAudio->getDefaultOutputDevice());
	d.channelsCount = kernelAudio.channelsOutCount;
	d.channelsStart = kernelAudio.channelsOutStart;

	return d;
}

KernelAudio::Device KernelAudio::getCurrentInDevice() const
{
	assert(m_rtAudio != nullptr);

	const model::KernelAudio& kernelAudio = m_model.get().kernelAudio;

	Device d        = fetchDevice(m_rtAudio->isStreamOpen() ? kernelAudio.deviceIn.index : m_rtAudio->getDefaultInputDevice());
	d.channelsCount = kernelAudio.channelsInCount;
	d.channelsStart = kernelAudio.channelsInStart;

	return d;
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
		u::log::print("  %s\n", u::string::toString(api).c_str());
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
	    0,
	    0,
	    info.sampleRates};
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
	const CallbackInfo& info = *static_cast<CallbackInfo*>(data);

	mcl::AudioBuffer out(static_cast<float*>(outBuf), bufferSize, info.channelsOutCount);
	mcl::AudioBuffer in;
	if (info.channelsInCount > 0)
		in = mcl::AudioBuffer(static_cast<float*>(inBuf), bufferSize, info.channelsInCount);

	return info.kernelAudio->onAudioCallback(out, in);
}
} // namespace giada::m
