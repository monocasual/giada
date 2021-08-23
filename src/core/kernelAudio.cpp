/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * KernelAudio
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

#include "kernelAudio.h"
#include "conf.h"
#include "const.h"
#include "core/clock.h"
#include "core/mixerHandler.h"
#include "core/model/model.h"
#include "core/recManager.h"
#include "core/sync.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "deps/rtaudio/RtAudio.h"
#include "glue/main.h"
#include "mixer.h"
#include "utils/log.h"
#include "utils/vector.h"

namespace giada::m::kernelAudio
{
namespace
{
#ifdef WITH_AUDIO_JACK
std::optional<JackTransport> jackTransport_;
#endif
std::vector<Device> devices_;
RtAudio*            rtSystem_       = nullptr;
bool                inputEnabled_   = false;
unsigned            realBufsize_    = 0; // Real buffer size from the soundcard
int                 realSampleRate_ = 0; // Sample rate might differ if JACK in use
int                 api_            = 0;

/* -------------------------------------------------------------------------- */

Device fetchDevice_(size_t deviceIndex)
{
	try
	{
		RtAudio::DeviceInfo info = rtSystem_->getDeviceInfo(deviceIndex);

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
	catch (RtAudioError& e)
	{
		u::log::print("[KA] Error fetching device %d: %s\n", deviceIndex, e.getMessage());
		return {0};
	}
}

/* -------------------------------------------------------------------------- */

std::vector<Device> fetchDevices_()
{
	std::vector<Device> out;
	for (unsigned i = 0; i < rtSystem_->getDeviceCount(); i++)
		out.push_back(fetchDevice_(i));
	return out;
}

/* -------------------------------------------------------------------------- */

void printDevices_(const std::vector<Device>& devices)
{
	u::log::print("[KA] %d device(s) found\n", devices.size());
	for (const Device& d : devices)
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

bool canRender_()
{
	return model::get().kernel.audioReady && model::get().mixer.state->active.load() == true;
}

/* -------------------------------------------------------------------------- */

int callback_(void* outBuf, void* inBuf, unsigned bufferSize, double /*streamTime*/,
    RtAudioStreamStatus /*status*/, void* /*userData*/)
{
	mcl::AudioBuffer out(static_cast<float*>(outBuf), bufferSize, G_MAX_IO_CHANS);
	mcl::AudioBuffer in;
	if (isInputEnabled())
		in = mcl::AudioBuffer(static_cast<float*>(inBuf), bufferSize, conf::conf.channelsInCount);

	/* Clean up output buffer before any rendering. Do this even if mixer is
	disabled to avoid audio leftovers during a temporary suspension (e.g. when
	loading a new patch). */

	out.clear();

	if (!canRender_())
		return 0;

#ifdef WITH_AUDIO_JACK
	if (getAPI() == G_SYS_API_JACK)
		sync::recvJackSync(jackTransportQuery());
#endif

	mixer::RenderInfo info;
	info.isAudioReady    = model::get().kernel.audioReady;
	info.hasInput        = isInputEnabled();
	info.isClockActive   = clock::isActive();
	info.isClockRunning  = clock::isRunning();
	info.canLineInRec    = recManager::isRecordingInput() && isInputEnabled();
	info.limitOutput     = conf::conf.limitOutput;
	info.inToOut         = mh::getInToOut();
	info.maxFramesToRec  = conf::conf.inputRecMode == InputRecMode::FREE ? clock::getMaxFramesInLoop() : clock::getFramesInLoop();
	info.outVol          = mh::getOutVol();
	info.inVol           = mh::getInVol();
	info.recTriggerLevel = conf::conf.recTriggerLevel;

	return mixer::render(out, in, info);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

bool isReady()
{
	return model::get().kernel.audioReady;
}

/* -------------------------------------------------------------------------- */

int openDevice(const conf::Conf& conf)
{
	api_ = conf.soundSystem;
	u::log::print("[KA] using system 0x%x\n", api_);

#if defined(__linux__) || defined(__FreeBSD__)

	if (api_ == G_SYS_API_JACK && hasAPI(RtAudio::UNIX_JACK))
		rtSystem_ = new RtAudio(RtAudio::UNIX_JACK);
	else if (api_ == G_SYS_API_ALSA && hasAPI(RtAudio::LINUX_ALSA))
		rtSystem_ = new RtAudio(RtAudio::LINUX_ALSA);
	else if (api_ == G_SYS_API_PULSE && hasAPI(RtAudio::LINUX_PULSE))
		rtSystem_ = new RtAudio(RtAudio::LINUX_PULSE);

#elif defined(__FreeBSD__)

	if (api_ == G_SYS_API_JACK && hasAPI(RtAudio::UNIX_JACK))
		rtSystem_ = new RtAudio(RtAudio::UNIX_JACK);
	else if (api_ == G_SYS_API_PULSE && hasAPI(RtAudio::LINUX_PULSE))
		rtSystem_ = new RtAudio(RtAudio::LINUX_PULSE);

#elif defined(_WIN32)

	if (api_ == G_SYS_API_DS && hasAPI(RtAudio::WINDOWS_DS))
		rtSystem_ = new RtAudio(RtAudio::WINDOWS_DS);
	else if (api_ == G_SYS_API_ASIO && hasAPI(RtAudio::WINDOWS_ASIO))
		rtSystem_ = new RtAudio(RtAudio::WINDOWS_ASIO);
	else if (api_ == G_SYS_API_WASAPI && hasAPI(RtAudio::WINDOWS_WASAPI))
		rtSystem_ = new RtAudio(RtAudio::WINDOWS_WASAPI);

#elif defined(__APPLE__)

	if (api_ == G_SYS_API_CORE && hasAPI(RtAudio::MACOSX_CORE))
		rtSystem_ = new RtAudio(RtAudio::MACOSX_CORE);

#endif

	else
	{
		u::log::print("[KA] No API available, nothing to do!\n");
		return 0;
	}

	u::log::print("[KA] Opening device out=%d, in=%d, samplerate=%d\n",
	    conf.soundDeviceOut, conf.soundDeviceIn, conf.samplerate);

	devices_ = fetchDevices_();
	printDevices_(devices_);

	/* Abort here if devices found are zero. */

	if (devices_.size() == 0)
	{
		closeDevice();
		return 0;
	}

	RtAudio::StreamParameters outParams;
	RtAudio::StreamParameters inParams;

	outParams.deviceId     = conf.soundDeviceOut == G_DEFAULT_SOUNDDEV_OUT ? rtSystem_->getDefaultOutputDevice() : conf.soundDeviceOut;
	outParams.nChannels    = conf.channelsOutCount;
	outParams.firstChannel = conf.channelsOutStart;

	/* Input device can be disabled. Unlike the output, here we are using all
	channels and let the user choose which one to record from in the configuration
	panel. */

	if (conf.soundDeviceIn != -1)
	{
		inParams.deviceId     = conf.soundDeviceIn;
		inParams.nChannels    = conf.channelsInCount;
		inParams.firstChannel = conf.channelsInStart;
		inputEnabled_         = true;
	}
	else
		inputEnabled_ = false;

	RtAudio::StreamOptions options;
	options.streamName      = G_APP_NAME;
	options.numberOfBuffers = 4; // TODO - wtf?

	realBufsize_    = conf.buffersize;
	realSampleRate_ = conf.samplerate;

#ifdef WITH_AUDIO_JACK

	/* If JACK, use its own sample rate, not the one coming from the conf
	object. */

	if (api_ == G_SYS_API_JACK)
	{
		assert(devices_.size() > 0);
		assert(devices_[0].sampleRates.size() > 0);

		realSampleRate_ = devices_[0].sampleRates[0];
		u::log::print("[KA] JACK in use, samplerate=%d\n", realSampleRate_);
	}

#endif

	try
	{
		rtSystem_->openStream(
		    &outParams,                                     // output params
		    conf.soundDeviceIn != -1 ? &inParams : nullptr, // input params if inDevice is selected
		    RTAUDIO_FLOAT32,                                // audio format
		    realSampleRate_,                                // sample rate
		    &realBufsize_,                                  // buffer size in byte
		    &callback_,                                     // audio callback
		    nullptr,                                        // user data (unused)
		    &options);

#ifdef WITH_AUDIO_JACK
		// Initialize JACK transport - TODO waiting for KernelAudio class + constructor
		jackTransport_.emplace(*static_cast<jack_client_t*>(rtSystem_->HACK__getJackClient()));
#endif

		model::get().kernel.audioReady = true;
		model::swap(model::SwapType::NONE);
		return 1;
	}
	catch (RtAudioError& e)
	{
		u::log::print("[KA] rtSystem_ init error: %s\n", e.getMessage());
		closeDevice();
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

int startStream()
{
	try
	{
		rtSystem_->startStream();
		u::log::print("[KA] latency = %lu\n", rtSystem_->getStreamLatency());
		return 1;
	}
	catch (RtAudioError& e)
	{
		u::log::print("[KA] Start stream error: %s\n", e.getMessage());
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

int stopStream()
{
	try
	{
		rtSystem_->stopStream();
		return 1;
	}
	catch (RtAudioError& /*e*/)
	{
		u::log::print("[KA] Stop stream error\n");
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

int closeDevice()
{
	if (rtSystem_->isStreamOpen())
	{
		rtSystem_->stopStream();
		rtSystem_->closeStream();
		delete rtSystem_;
		rtSystem_ = nullptr;
	}
	return 1;
}

/* -------------------------------------------------------------------------- */

unsigned getRealBufSize() { return realBufsize_; }
bool     isInputEnabled() { return inputEnabled_; }

/* -------------------------------------------------------------------------- */

Device getDevice(const char* name)
{
	for (Device device : devices_)
		if (name == device.name)
			return device;
	return {0, false};
}

/* -------------------------------------------------------------------------- */

const std::vector<Device>& getDevices()
{
	return devices_;
}

/* -------------------------------------------------------------------------- */

bool hasAPI(int API)
{
	std::vector<RtAudio::Api> APIs;
	RtAudio::getCompiledApi(APIs);
	for (unsigned i = 0; i < APIs.size(); i++)
		if (APIs.at(i) == API)
			return true;
	return false;
}

int getAPI() { return api_; }

/* -------------------------------------------------------------------------- */

void logCompiledAPIs()
{
	std::vector<RtAudio::Api> APIs;
	RtAudio::getCompiledApi(APIs);

	u::log::print("[KA] Compiled RtAudio APIs: %d\n", APIs.size());

	for (const RtAudio::Api& api_ : APIs)
	{
		switch (api_)
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

#ifdef WITH_AUDIO_JACK

JackTransport::State jackTransportQuery()
{
	if (api_ == G_SYS_API_JACK)
		return jackTransport_->getState();
	return {};
}

/* -------------------------------------------------------------------------- */

void jackStart()
{
	if (api_ == G_SYS_API_JACK)
		jackTransport_->start();
}

/* -------------------------------------------------------------------------- */

void jackSetPosition(uint32_t frame)
{
	if (api_ == G_SYS_API_JACK)
		jackTransport_->setPosition(frame);
}

/* -------------------------------------------------------------------------- */

void jackSetBpm(double bpm)
{
	if (api_ == G_SYS_API_JACK)
		jackTransport_->setBpm(bpm);
}

/* -------------------------------------------------------------------------- */

void jackStop()
{
	if (api_ == G_SYS_API_JACK)
		jackTransport_->stop();
}

#endif // WITH_AUDIO_JACK
} // namespace giada::m::kernelAudio
