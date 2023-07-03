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
namespace
{
/* RTAUDIO_MAX_PRIORITY
The maximum priority level when instructing RtAudio to work in real-time mode
(with RTAUDIO_SCHEDULE_REALTIME flag). This is actually the maximum value allowed
on Unix, however RtAudio has some magic hardcoded values when it comes to Windows
implementation, so we can safely pass this value below in the options struct.*/

constexpr int RTAUDIO_MAX_PRIORITY = 99;
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

KernelAudio::KernelAudio(model::Model& model)
: onAudioCallback(nullptr)
, onStreamAboutToOpen(nullptr)
, onStreamOpened(nullptr)
, m_model(model)
{
}

/* -------------------------------------------------------------------------- */

bool KernelAudio::init()
{
	const model::KernelAudio& kernelAudio = m_model.get().kernelAudio;

	setAPI_(kernelAudio.api);

	OpenStreamResult result = openStream_(
	    kernelAudio.deviceOut,
	    kernelAudio.deviceIn,
	    kernelAudio.samplerate,
	    kernelAudio.buffersize);

	return result.success;
}

/* -------------------------------------------------------------------------- */

void KernelAudio::setAPI(RtAudio::Api api)
{
	setAPI_(api);

	m_model.get().kernelAudio     = {};
	m_model.get().kernelAudio.api = api;
	m_model.swap(model::SwapType::NONE);

	printDevices(getAvailableDevices());
}

/* -------------------------------------------------------------------------- */

bool KernelAudio::openStream(
    const model::KernelAudio::Device& out,
    const model::KernelAudio::Device& in,
    unsigned int                      sampleRate,
    unsigned int                      bufferSize)
{
	assert(onStreamOpened != nullptr);
	assert(onStreamAboutToOpen != nullptr);

	onStreamAboutToOpen();

	OpenStreamResult result = openStream_(out, in, sampleRate, bufferSize);
	if (!result.success)
		return false;

	model::KernelAudio& kernelAudio = m_model.get().kernelAudio;

	kernelAudio.deviceOut  = out;
	kernelAudio.deviceIn   = in;
	kernelAudio.samplerate = result.actualSampleRate;
	kernelAudio.buffersize = result.actualBufferSize;
	m_model.swap(model::SwapType::NONE);

	onStreamOpened();

	return true;
}

/* -------------------------------------------------------------------------- */

bool KernelAudio::startStream()
{
	if (m_rtAudio->startStream() == RtAudioErrorType::RTAUDIO_NO_ERROR)
	{
		u::log::print("[KA] Start stream - latency = {}\n", m_rtAudio->getStreamLatency());
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

bool KernelAudio::isReady() const
{
	return m_rtAudio != nullptr && m_rtAudio->isStreamOpen() && m_rtAudio->isStreamRunning();
}

/* -------------------------------------------------------------------------- */

unsigned int       KernelAudio::getBufferSize() const { return m_model.get().kernelAudio.buffersize; }
int                KernelAudio::getSampleRate() const { return m_model.get().kernelAudio.samplerate; }
int                KernelAudio::getChannelsOutCount() const { return m_model.get().kernelAudio.deviceOut.channelsCount; }
int                KernelAudio::getChannelsInCount() const { return m_model.get().kernelAudio.deviceIn.channelsCount; }
bool               KernelAudio::isInputEnabled() const { return m_model.get().kernelAudio.deviceIn.index != -1; }
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
	d.channelsCount = kernelAudio.deviceOut.channelsCount;
	d.channelsStart = kernelAudio.deviceOut.channelsStart;

	return d;
}

KernelAudio::Device KernelAudio::getCurrentInDevice() const
{
	assert(m_rtAudio != nullptr);

	const model::KernelAudio& kernelAudio = m_model.get().kernelAudio;

	Device d        = fetchDevice(m_rtAudio->isStreamOpen() ? kernelAudio.deviceIn.index : m_rtAudio->getDefaultInputDevice());
	d.channelsCount = kernelAudio.deviceIn.channelsCount;
	d.channelsStart = kernelAudio.deviceIn.channelsStart;

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

RtAudio::Api KernelAudio::getAPI() const { return m_model.get().kernelAudio.api; }

/* -------------------------------------------------------------------------- */

void KernelAudio::logCompiledAPIs()
{
	std::vector<RtAudio::Api> APIs;
	RtAudio::getCompiledApi(APIs);

	u::log::print("[KA] Compiled RtAudio APIs: {}\n", APIs.size());

	for (const RtAudio::Api& api : APIs)
		u::log::print("  {}\n", u::string::toString(api));
}

/* -------------------------------------------------------------------------- */

m::KernelAudio::Device KernelAudio::fetchDevice(size_t deviceIndex) const
{
	RtAudio::DeviceInfo info = m_rtAudio->getDeviceInfo(deviceIndex);

	if (!info.probed)
	{
		u::log::print("[KA] Can't probe device {}\n", deviceIndex);
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
	u::log::print("[KA] {} device(s) found with API {}\n", devices.size(), u::string::toString(m_rtAudio->getCurrentApi()));
	for (const m::KernelAudio::Device& d : devices)
	{
		u::log::print("  {}) {}\n", d.index, d.name);
		u::log::print("      ins={} outs={} duplex={}\n", d.maxInputChannels, d.maxOutputChannels, d.maxDuplexChannels);
		u::log::print("      isDefaultOut={} isDefaultIn={}\n", d.isDefaultOut, d.isDefaultIn);
		u::log::print("      sampleRates:\n\t");
		for (int s : d.sampleRates)
			u::log::print("{} ", s);
		u::log::print("\n");
	}
}

/* -------------------------------------------------------------------------- */

void KernelAudio::setAPI_(RtAudio::Api api)
{
	u::log::print("[KA] using API {}\n", u::string::toString(api));

	if (m_rtAudio != nullptr)
		shutdown();

	m_rtAudio = std::make_unique<RtAudio>(api);

	m_rtAudio->setErrorCallback([](RtAudioErrorType type, const std::string& msg) {
		u::log::print("[KA] RtAudio error {}: {}\n", static_cast<int>(type), msg);
	});
}

/* -------------------------------------------------------------------------- */

KernelAudio::OpenStreamResult KernelAudio::openStream_(
    const model::KernelAudio::Device& out,
    const model::KernelAudio::Device& in,
    unsigned int                      sampleRate,
    unsigned int                      bufferSize)

{
	assert(onAudioCallback != nullptr);
	assert(m_rtAudio != nullptr);

	u::log::print("[KA] Opening stream\n");
	u::log::print("     Out device: index={} channelsCount={} channelsStart={}\n", out.index, out.channelsCount, out.channelsStart);
	u::log::print("     In device: index={} channelsCount={} channelsStart={}\n", in.index, in.channelsCount, in.channelsStart);
	u::log::print("     SampleRate={}\n", sampleRate);
	u::log::print("     BufferSize={}\n", bufferSize);

	const RtAudio::Api api = m_model.get().kernelAudio.api;

	/* Abort here if devices found are zero, both devices are disabled or 
	current API is dummy. */

	if (m_rtAudio->getDeviceCount() == 0 || (in.index == -1 && out.index == -1) || api == RtAudio::Api::RTAUDIO_DUMMY)
		return {};

	/* Close stream before opening another one. Closing a stream frees any
	associated stream memory. */

	if (m_rtAudio->isStreamOpen())
		m_rtAudio->closeStream();

	RtAudio::StreamParameters outParams;
	RtAudio::StreamParameters inParams;

	outParams.deviceId     = out.index == G_DEFAULT_SOUNDDEV_OUT ? m_rtAudio->getDefaultOutputDevice() : out.index;
	outParams.nChannels    = out.channelsCount;
	outParams.firstChannel = out.channelsStart;

	/* Input device can be disabled. Unlike the output, here we are using all
	channels and let the user choose which one to record from in the configuration
	panel. */

	if (in.index != -1)
	{
		inParams.deviceId     = in.index;
		inParams.nChannels    = in.channelsCount;
		inParams.firstChannel = in.channelsStart;
	}

	RtAudio::StreamOptions options;
	options.flags           = RTAUDIO_SCHEDULE_REALTIME;
	options.priority        = RTAUDIO_MAX_PRIORITY;
	options.streamName      = G_APP_NAME;
	options.numberOfBuffers = 4; // TODO - wtf?

	unsigned int actualSampleRate = sampleRate;
	unsigned int actualBufferSize = bufferSize;

#ifdef WITH_AUDIO_JACK

	/* If JACK, use its own sample rate. */

	if (api == RtAudio::Api::UNIX_JACK)
	{
		const Device jackDevice = fetchDevice(0); // JACK has only one device

		assert(jackDevice.probed);
		assert(jackDevice.sampleRates.size() > 0);

		const unsigned int jackSampleRate = jackDevice.sampleRates[0];

		u::log::print("[KA] JACK in use, samplerate={}\n", jackSampleRate);

		actualSampleRate = jackSampleRate;
	}

#endif

	m_callbackInfo = {
	    /* rtAudio */ this,
	    /* channelsOutCount */ out.channelsCount,
	    /* channelsInCount */ in.channelsCount};

	RtAudioErrorType res = m_rtAudio->openStream(
	    &outParams,                           // output params
	    in.index != -1 ? &inParams : nullptr, // input params if inDevice is selected
	    RTAUDIO_FLOAT32,                      // audio format
	    actualSampleRate,                     // sample rate
	    &actualBufferSize,                    // buffer size in bytes. Might be changed to the actual value used by the soundcard
	    &audioCallback,                       // audio callback
	    &m_callbackInfo,                      // user data passed to callback
	    &options);

	if (res != RtAudioErrorType::RTAUDIO_NO_ERROR)
		return {};

	u::log::print("[KA] Device opened successfully\n");

	return {true, actualSampleRate, actualBufferSize};
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
