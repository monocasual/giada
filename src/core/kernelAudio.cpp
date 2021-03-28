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
#include "deps/rtaudio/RtAudio.h"
#include "glue/main.h"
#include "mixer.h"
#include "utils/log.h"

namespace giada::m::kernelAudio
{
namespace
{
RtAudio* rtSystem     = nullptr;
unsigned numDevs      = 0;
bool     inputEnabled = false;
unsigned realBufsize  = 0; // Real buffer size from the soundcard
int      api          = 0;

#ifdef WITH_AUDIO_JACK

JackState jackState;

jack_client_t* jackGetHandle_()
{
	return static_cast<jack_client_t*>(rtSystem->HACK__getJackClient());
}

#endif

/* -------------------------------------------------------------------------- */

bool canRender_()
{
	return model::get().kernel.audioReady && model::get().mixer.state->active.load() == true;
}

/* -------------------------------------------------------------------------- */

int callback_(void* outBuf, void* inBuf, unsigned bufferSize, double /*streamTime*/,
    RtAudioStreamStatus /*status*/, void* /*userData*/)
{
	AudioBuffer out(static_cast<float*>(outBuf), bufferSize, G_MAX_IO_CHANS);
	AudioBuffer in;
	if (isInputEnabled())
		in = AudioBuffer(static_cast<float*>(inBuf), bufferSize, conf::conf.channelsInCount);

	/* Clean up output buffer before any rendering. Do this even if mixer is
	disabled to avoid audio leftovers during a temporary suspension (e.g. when
	loading a new patch). */

	out.clear();

	if (!canRender_())
		return 0;

#ifdef WITH_AUDIO_JACK
	if (getAPI() == G_SYS_API_JACK)
		clock::recvJackSync();
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

#ifdef WITH_AUDIO_JACK

bool JackState::operator!=(const JackState& o) const
{
	return !(running == o.running && bpm == o.bpm && frame == o.frame);
}

#endif

/* -------------------------------------------------------------------------- */

bool isReady()
{
	return model::get().kernel.audioReady;
}

/* -------------------------------------------------------------------------- */

int openDevice()
{
	api = conf::conf.soundSystem;
	u::log::print("[KA] using system 0x%x\n", api);

#if defined(__linux__) || defined(__FreeBSD__)

	if (api == G_SYS_API_JACK && hasAPI(RtAudio::UNIX_JACK))
		rtSystem = new RtAudio(RtAudio::UNIX_JACK);
	else if (api == G_SYS_API_ALSA && hasAPI(RtAudio::LINUX_ALSA))
		rtSystem = new RtAudio(RtAudio::LINUX_ALSA);
	else if (api == G_SYS_API_PULSE && hasAPI(RtAudio::LINUX_PULSE))
		rtSystem = new RtAudio(RtAudio::LINUX_PULSE);

#elif defined(__FreeBSD__)

	if (api == G_SYS_API_JACK && hasAPI(RtAudio::UNIX_JACK))
		rtSystem = new RtAudio(RtAudio::UNIX_JACK);
	else if (api == G_SYS_API_PULSE && hasAPI(RtAudio::LINUX_PULSE))
		rtSystem = new RtAudio(RtAudio::LINUX_PULSE);

#elif defined(_WIN32)

	if (api == G_SYS_API_DS && hasAPI(RtAudio::WINDOWS_DS))
		rtSystem = new RtAudio(RtAudio::WINDOWS_DS);
	else if (api == G_SYS_API_ASIO && hasAPI(RtAudio::WINDOWS_ASIO))
		rtSystem = new RtAudio(RtAudio::WINDOWS_ASIO);
	else if (api == G_SYS_API_WASAPI && hasAPI(RtAudio::WINDOWS_WASAPI))
		rtSystem = new RtAudio(RtAudio::WINDOWS_WASAPI);

#elif defined(__APPLE__)

	if (api == G_SYS_API_CORE && hasAPI(RtAudio::MACOSX_CORE))
		rtSystem = new RtAudio(RtAudio::MACOSX_CORE);

#endif

	else
	{
		u::log::print("[KA] No API available, nothing to do!\n");
		return 0;
	}

	u::log::print("[KA] Opening device out=%d, in=%d, samplerate=%d\n",
	    conf::conf.soundDeviceOut, conf::conf.soundDeviceIn, conf::conf.samplerate);

	numDevs = rtSystem->getDeviceCount();

	if (numDevs < 1)
	{
		u::log::print("[KA] no devices found with this API\n");
		closeDevice();
		return 0;
	}
	else
	{
		u::log::print("[KA] %d device(s) found\n", numDevs);
		for (unsigned i = 0; i < numDevs; i++)
			u::log::print("  %d) %s\n", i, getDeviceName(i));
	}

	RtAudio::StreamParameters outParams;
	RtAudio::StreamParameters inParams;

	outParams.deviceId     = conf::conf.soundDeviceOut == G_DEFAULT_SOUNDDEV_OUT ? getDefaultOut() : conf::conf.soundDeviceOut;
	outParams.nChannels    = G_MAX_IO_CHANS;
	outParams.firstChannel = conf::conf.channelsOut * G_MAX_IO_CHANS; // chan 0=0, 1=2, 2=4, ...

	/* Input device can be disabled. Unlike the output, here we are using all
	channels and let the user choose which one to record from in the configuration
	panel. */

	if (conf::conf.soundDeviceIn != -1)
	{
		inParams.deviceId     = conf::conf.soundDeviceIn;
		inParams.nChannels    = conf::conf.channelsInCount;
		inParams.firstChannel = conf::conf.channelsInStart;
		inputEnabled          = true;
	}
	else
		inputEnabled = false;

	RtAudio::StreamOptions options;
	options.streamName      = G_APP_NAME;
	options.numberOfBuffers = 4; // TODO - wtf?

	realBufsize = conf::conf.buffersize;

#ifdef WITH_AUDIO_JACK

	if (api == G_SYS_API_JACK)
	{
		conf::conf.samplerate = getFreq(conf::conf.soundDeviceOut, 0);
		u::log::print("[KA] JACK in use, samplerate=%d\n", conf::conf.samplerate);
	}

#endif

	try
	{
		rtSystem->openStream(
		    &outParams,                                           // output params
		    conf::conf.soundDeviceIn != -1 ? &inParams : nullptr, // input params if inDevice is selected
		    RTAUDIO_FLOAT32,                                      // audio format
		    conf::conf.samplerate,                                // sample rate
		    &realBufsize,                                         // buffer size in byte
		    &callback_,                                           // audio callback
		    nullptr,                                              // user data (unused)
		    &options);

		model::get().kernel.audioReady = true;
		model::swap(model::SwapType::NONE);
		return 1;
	}
	catch (RtAudioError& e)
	{
		u::log::print("[KA] rtSystem init error: %s\n", e.getMessage());
		closeDevice();
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

int startStream()
{
	try
	{
		rtSystem->startStream();
		u::log::print("[KA] latency = %lu\n", rtSystem->getStreamLatency());
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
		rtSystem->stopStream();
		return 1;
	}
	catch (RtAudioError& /*e*/)
	{
		u::log::print("[KA] Stop stream error\n");
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

std::string getDeviceName(unsigned dev)
{
	try
	{
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).name;
	}
	catch (RtAudioError& /*e*/)
	{
		u::log::print("[KA] invalid device ID = %d\n", dev);
		return "";
	}
}

/* -------------------------------------------------------------------------- */

int closeDevice()
{
	if (rtSystem->isStreamOpen())
	{
		rtSystem->stopStream();
		rtSystem->closeStream();
		delete rtSystem;
		rtSystem = nullptr;
	}
	return 1;
}

/* -------------------------------------------------------------------------- */

unsigned getMaxInChans(int dev)
{
	if (dev == -1)
		return 0;

	try
	{
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).inputChannels;
	}
	catch (RtAudioError& /*e*/)
	{
		u::log::print("[KA] Unable to get input channels\n");
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

unsigned getMaxOutChans(unsigned dev)
{
	try
	{
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).outputChannels;
	}
	catch (RtAudioError& /*e*/)
	{
		u::log::print("[KA] Unable to get output channels\n");
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

bool isProbed(unsigned dev)
{
	try
	{
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).probed;
	}
	catch (RtAudioError& /*e*/)
	{
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

unsigned getDuplexChans(unsigned dev)
{
	try
	{
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).duplexChannels;
	}
	catch (RtAudioError& /*e*/)
	{
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

bool isDefaultIn(unsigned dev)
{
	try
	{
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).isDefaultInput;
	}
	catch (RtAudioError& /*e*/)
	{
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

bool isDefaultOut(unsigned dev)
{
	try
	{
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).isDefaultOutput;
	}
	catch (RtAudioError& /*e*/)
	{
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

int getTotalFreqs(unsigned dev)
{
	try
	{
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).sampleRates.size();
	}
	catch (RtAudioError& /*e*/)
	{
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

int getFreq(unsigned dev, int i)
{
	try
	{
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).sampleRates.at(i);
	}
	catch (RtAudioError& /*e*/)
	{
		return 0;
	}
}

/* -------------------------------------------------------------------------- */

unsigned getRealBufSize() { return realBufsize; }
bool     isInputEnabled() { return inputEnabled; }
unsigned countDevices() { return numDevs; }

/* -------------------------------------------------------------------------- */

int getDefaultIn()
{
	return rtSystem->getDefaultInputDevice();
}

int getDefaultOut()
{
	return rtSystem->getDefaultOutputDevice();
}

/* -------------------------------------------------------------------------- */

int getDeviceByName(const char* name)
{
	for (unsigned i = 0; i < numDevs; i++)
		if (name == getDeviceName(i))
			return i;
	return -1;
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

int getAPI() { return api; }

/* -------------------------------------------------------------------------- */

void logCompiledAPIs()
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

#ifdef WITH_AUDIO_JACK

JackState jackTransportQuery()
{
	if (api != G_SYS_API_JACK)
		return jackState;

	jack_position_t        position;
	jack_transport_state_t ts = jack_transport_query(jackGetHandle_(), &position);

	return {
	    ts != JackTransportStopped,
	    position.beats_per_minute,
	    position.frame};
}

/* -------------------------------------------------------------------------- */

void jackStart()
{
	if (api == G_SYS_API_JACK)
		jack_transport_start(jackGetHandle_());
}

/* -------------------------------------------------------------------------- */

void jackSetPosition(uint32_t frame)
{
	if (api != G_SYS_API_JACK)
		return;
	jack_position_t position;
	jack_transport_query(jackGetHandle_(), &position);
	position.frame = frame;
	jack_transport_reposition(jackGetHandle_(), &position);
}

/* -------------------------------------------------------------------------- */

void jackSetBpm(double bpm)
{
	if (api != G_SYS_API_JACK)
		return;
	jack_position_t position;
	jack_transport_query(jackGetHandle_(), &position);
	position.valid            = jack_position_bits_t::JackPositionBBT;
	position.bar              = 0; // no such info from Giada
	position.beat             = 0; // no such info from Giada
	position.tick             = 0; // no such info from Giada
	position.beats_per_minute = bpm;
	jack_transport_reposition(jackGetHandle_(), &position);
}

/* -------------------------------------------------------------------------- */

void jackStop()
{
	if (api == G_SYS_API_JACK)
		jack_transport_stop(jackGetHandle_());
}

#endif // WITH_AUDIO_JACK
} // namespace giada::m::kernelAudio
