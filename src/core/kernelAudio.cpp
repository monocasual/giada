/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * KernelAudio
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

#include "src/core/kernelAudio.h"
#include "src/core/const.h"
#include "src/core/model/kernelAudio.h"
#include "src/core/model/model.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "src/utils/log.h"
#include "src/utils/string.h"
#include "src/utils/vector.h"
#include <cassert>
#include <cstddef>

namespace giada::m
{
namespace
{
using Now      = std::chrono::time_point<std::chrono::high_resolution_clock>;
using Duration = std::chrono::duration<double, std::micro>;

/* RTAUDIO_MAX_PRIORITY
The maximum priority level when instructing RtAudio to work in real-time mode
(with RTAUDIO_SCHEDULE_REALTIME flag). This is actually the maximum value allowed
on Unix, however RtAudio has some magic hardcoded values when it comes to Windows
implementation, so we can safely pass this value below in the options struct.*/

constexpr int RTAUDIO_MAX_PRIORITY = 99;

/* -------------------------------------------------------------------------- */

double computeCpuLoad(const Now& startTime, unsigned int sampleRate, int bufferSize)
{
	const Now      endTime        = std::chrono::high_resolution_clock::now();
	const Duration processingTime = endTime - startTime;

	const double callbackDuration = (static_cast<double>(bufferSize) / sampleRate) * 1e6; // in microseconds
	const double load             = (processingTime.count() / callbackDuration) * 100.0;

	return load;
}

} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

KernelAudio::KernelAudio(model::Model& model)
: onAudioCallback(nullptr)
, onStreamAboutToOpen(nullptr)
, onStreamOpened(nullptr)
, m_model(model)
, m_jackMaxOutputChannels(0)
{
}

/* -------------------------------------------------------------------------- */

bool KernelAudio::init()
{
	model::KernelAudio& kernelAudio = m_model.get().kernelAudio;

	kernelAudio.api = setAPI_(kernelAudio.api);

	OpenStreamResult result = openStream_(
	    kernelAudio.deviceOut,
	    kernelAudio.deviceIn,
	    kernelAudio.samplerate,
	    kernelAudio.buffersize);

	if (result.success)
	{
		kernelAudio.deviceOut  = result.deviceOut;
		kernelAudio.deviceIn   = result.deviceIn;
		kernelAudio.samplerate = result.actualSampleRate;
		kernelAudio.buffersize = result.actualBufferSize;
	}

	m_model.swap(model::SwapType::NONE);

	return result.success;
}

/* -------------------------------------------------------------------------- */

void KernelAudio::setAPI(RtAudio::Api desiredApi)
{
	m_model.get().kernelAudio     = {}; // Set API = reset everything
	m_model.get().kernelAudio.api = setAPI_(desiredApi);
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

	kernelAudio.deviceOut  = result.deviceOut;
	kernelAudio.deviceIn   = result.deviceIn;
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
bool               KernelAudio::isInputEnabled() const { return m_model.get().kernelAudio.deviceIn.id != 0; }
bool               KernelAudio::isLimitOutput() const { return m_model.get().kernelAudio.limitOutput; }
float              KernelAudio::getRecTriggerLevel() const { return m_model.get().kernelAudio.recTriggerLevel; }
Resampler::Quality KernelAudio::getResamplerQuality() const { return m_model.get().kernelAudio.rsmpQuality; }

/* -------------------------------------------------------------------------- */

std::vector<m::KernelAudio::Device> KernelAudio::getAvailableDevices() const
{
	std::vector<Device> out;
	for (unsigned int i : m_rtAudio->getDeviceIds())
		out.push_back(fetchDevice(i));
	return out;
}

/* -------------------------------------------------------------------------- */

KernelAudio::Device KernelAudio::getCurrentOutDevice() const
{
	assert(m_rtAudio != nullptr);

	const model::KernelAudio& kernelAudio = m_model.get().kernelAudio;

	Device d        = fetchDevice(m_rtAudio->isStreamOpen() ? kernelAudio.deviceOut.id : m_rtAudio->getDefaultOutputDevice());
	d.channelsStart = kernelAudio.deviceOut.channelsStart;
	if (getAPI() == RtAudio::Api::UNIX_JACK)
		d.maxOutputChannels = m_jackMaxOutputChannels;

	return d;
}

KernelAudio::Device KernelAudio::getCurrentInDevice() const
{
	assert(m_rtAudio != nullptr);

	const model::KernelAudio& kernelAudio = m_model.get().kernelAudio;

	Device d        = fetchDevice(m_rtAudio->isStreamOpen() ? kernelAudio.deviceIn.id : m_rtAudio->getDefaultInputDevice());
	d.channelsStart = kernelAudio.deviceIn.channelsStart;

	return d;
}

/* -------------------------------------------------------------------------- */

double KernelAudio::getCpuLoad() const
{
	return m_model.get().kernelAudio.a_getCpuLoad();
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

m::KernelAudio::Device KernelAudio::fetchDevice(unsigned int deviceId) const
{
	RtAudio::DeviceInfo info = m_rtAudio->getDeviceInfo(deviceId);

	return {
	    deviceId,
	    info.name,
	    static_cast<int>(info.outputChannels),
	    static_cast<int>(info.inputChannels),
	    static_cast<int>(info.duplexChannels),
	    info.isDefaultOutput,
	    info.isDefaultInput,
	    0,
	    info.sampleRates};
}

/* -------------------------------------------------------------------------- */

void KernelAudio::printDevices(const std::vector<m::KernelAudio::Device>& devices) const
{
	u::log::print("[KA] {} device(s) found with API {}\n", devices.size(), u::string::toString(m_rtAudio->getCurrentApi()));
	for (const m::KernelAudio::Device& d : devices)
	{
		u::log::print("  id={}) {}\n", d.id, d.name);
		u::log::print("      ins={} outs={} duplex={}\n", d.maxInputChannels, d.maxOutputChannels, d.maxDuplexChannels);
		u::log::print("      isDefaultOut={} isDefaultIn={}\n", d.isDefaultOut, d.isDefaultIn);
		u::log::print("      sampleRates:\n\t");
		for (int s : d.sampleRates)
			u::log::print("{} ", s);
		u::log::print("\n");
	}
}

/* -------------------------------------------------------------------------- */

RtAudio::Api KernelAudio::setAPI_(RtAudio::Api api)
{
	u::log::print("[KA] using API {}\n", u::string::toString(api));

	if (m_rtAudio != nullptr)
		shutdown();

	m_rtAudio = std::make_unique<RtAudio>(api);

	m_rtAudio->setErrorCallback([](RtAudioErrorType type, const std::string& msg)
	    { u::log::print("[KA] RtAudio error {}: {}\n", static_cast<int>(type), msg); });

	/* If api == UNSPECIFIED, rtAudio will pick one according to some internal
	logic. */

	return api == RtAudio::Api::UNSPECIFIED ? m_rtAudio->getCurrentApi() : api;
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
	assert(out.channelsCount >= 0);
	assert(out.channelsStart >= 0);
	assert(in.channelsCount >= 0);
	assert(in.channelsStart >= 0);

	u::log::print("[KA] Opening stream\n");
	u::log::print("     Out device: id={} channelsCount={} channelsStart={}\n", out.id, out.channelsCount, out.channelsStart);
	u::log::print("     In device: id={} channelsCount={} channelsStart={}\n", in.id, in.channelsCount, in.channelsStart);
	u::log::print("     SampleRate={}\n", sampleRate);
	u::log::print("     BufferSize={}\n", bufferSize);

	const RtAudio::Api api = m_model.get().kernelAudio.api;

	/* Abort here if devices found are zero, both devices are disabled or
	current API is dummy. */

	if (m_rtAudio->getDeviceCount() == 0 || (in.id == 0 && out.id == 0) || api == RtAudio::Api::RTAUDIO_DUMMY)
		return {};

	/* Close stream before opening another one. Closing a stream frees any
	associated stream memory. */

	if (m_rtAudio->isStreamOpen())
		m_rtAudio->closeStream();

	RtAudio::StreamParameters outParams;
	RtAudio::StreamParameters inParams;

	outParams.deviceId     = out.id == -1 ? m_rtAudio->getDefaultOutputDevice() : out.id;
	outParams.nChannels    = out.channelsCount;
	outParams.firstChannel = out.channelsStart;

	/* Store the current number of output channels for JACK. This is needed when calling getCurrentOutDevice(),
	where the maxOutputChannels parameter is not correctly updated for JACK. */

	m_jackMaxOutputChannels = out.channelsCount;

	if (in.id != 0)
	{
		inParams.deviceId     = in.id == -1 ? m_rtAudio->getDefaultInputDevice() : in.id;
		inParams.nChannels    = in.channelsCount;
		inParams.firstChannel = in.channelsStart;
	}

	RtAudio::StreamOptions options;
	options.flags           = RTAUDIO_SCHEDULE_REALTIME | RTAUDIO_JACK_DONT_CONNECT;
	options.priority        = RTAUDIO_MAX_PRIORITY;
	options.streamName      = G_APP_NAME;
	options.numberOfBuffers = 4; // TODO - wtf?

	unsigned int actualSampleRate = sampleRate;
	unsigned int actualBufferSize = bufferSize;

#ifdef WITH_AUDIO_JACK

	/* If JACK, use its own sample rate. */

	if (api == RtAudio::Api::UNIX_JACK)
	{
		const Device jackDevice = fetchDevice(out.id); // JACK has only one device

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
	    &outParams,                       // output params
	    in.id != 0 ? &inParams : nullptr, // input params if inDevice is selected
	    RTAUDIO_FLOAT32,                  // audio format
	    actualSampleRate,                 // sample rate
	    &actualBufferSize,                // buffer size in bytes. Might be changed to the actual value used by the soundcard
	    &audioCallback,                   // audio callback
	    &m_callbackInfo,                  // user data passed to callback
	    &options);

	if (res != RtAudioErrorType::RTAUDIO_NO_ERROR)
		return {};

	u::log::print("[KA] Device opened successfully\n");

	return {
	    true,
	    {
	        static_cast<int>(outParams.deviceId),
	        static_cast<int>(outParams.nChannels),
	        static_cast<int>(outParams.firstChannel),
	    },
	    {
	        static_cast<int>(inParams.deviceId),
	        static_cast<int>(inParams.nChannels),
	        static_cast<int>(inParams.firstChannel),
	    },
	    actualSampleRate, actualBufferSize};
}

/* -------------------------------------------------------------------------- */

int KernelAudio::audioCallback(void* outBuf, void* inBuf, unsigned bufferSize,
    double /*streamTime*/, RtAudioStreamStatus /*status*/, void*   data)
{
	const Now startTime = std::chrono::high_resolution_clock::now();

	const CallbackInfo& info = *static_cast<CallbackInfo*>(data);

	mcl::AudioBuffer out(static_cast<float*>(outBuf), bufferSize, info.channelsOutCount);
	mcl::AudioBuffer in;
	if (info.channelsInCount > 0)
		in = mcl::AudioBuffer(static_cast<float*>(inBuf), bufferSize, info.channelsInCount);

	const int ret = info.kernelAudio->onAudioCallback(out, in);

	/* CPU load computation. */

	const model::DocumentLock documentLock = info.kernelAudio->m_model.get_RT();
	const model::Document&    document_RT  = documentLock.get();
	const model::KernelAudio& kernelAudio  = document_RT.kernelAudio;

	kernelAudio.a_setCpuLoad(computeCpuLoad(startTime, kernelAudio.samplerate, bufferSize));

	return ret;
}
} // namespace giada::m
