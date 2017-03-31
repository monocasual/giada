/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * KernelAudio
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include "../deps/rtaudio-mod/RtAudio.h"
#include "../utils/log.h"
#include "../glue/main.h"
#include "conf.h"
#include "mixer.h"
#include "const.h"
#include "kernelAudio.h"


using std::string;
using std::vector;


namespace giada {
namespace kernelAudio
{
namespace
{
  RtAudio *rtSystem;
  unsigned numDevs;
	bool inputEnabled;
	unsigned realBufsize; 		// reale bufsize from the soundcard
	int api;

#ifdef __linux__

  JackState jackState;

  jack_client_t *jackGetHandle()
  {
  	return static_cast<jack_client_t*>(rtSystem->rtapi_->__HACK__getJackClient());
  }

#endif
};  // namespace


/* -------------------------------------------------------------------------- */


void init()
{
  rtSystem     = nullptr;
  numDevs      = 0;
  inputEnabled = 0;
  realBufsize  = 0;
  api          = 0;
}


/* -------------------------------------------------------------------------- */


int openDevice(Mixer *mixer)
{
	api = conf::soundSystem;
	gu_log("[KA] using rtSystem 0x%x\n", api);

#if defined(__linux__)

	if (api == SYS_API_JACK && hasAPI(RtAudio::UNIX_JACK))
		rtSystem = new RtAudio(RtAudio::UNIX_JACK);
	else
	if (api == SYS_API_ALSA && hasAPI(RtAudio::LINUX_ALSA))
		rtSystem = new RtAudio(RtAudio::LINUX_ALSA);
	else
	if (api == SYS_API_PULSE && hasAPI(RtAudio::LINUX_PULSE))
		rtSystem = new RtAudio(RtAudio::LINUX_PULSE);

#elif defined(_WIN32)

	if (api == SYS_API_DS && hasAPI(RtAudio::WINDOWS_DS))
		rtSystem = new RtAudio(RtAudio::WINDOWS_DS);
	else
	if (api == SYS_API_ASIO && hasAPI(RtAudio::WINDOWS_ASIO))
		rtSystem = new RtAudio(RtAudio::WINDOWS_ASIO);
	else
	if (api == SYS_API_WASAPI && hasAPI(RtAudio::WINDOWS_WASAPI))
		rtSystem = new RtAudio(RtAudio::WINDOWS_WASAPI);

#elif defined(__APPLE__)

	if (api == SYS_API_CORE && hasAPI(RtAudio::MACOSX_CORE))
		rtSystem = new RtAudio(RtAudio::MACOSX_CORE);

#endif

	else
		return 0;

	gu_log("[KA] Opening devices %d (out), %d (in), f=%d...\n",
    conf::soundDeviceOut, conf::soundDeviceIn, conf::samplerate);

	numDevs = rtSystem->getDeviceCount();

	if (numDevs < 1) {
		gu_log("[KA] no devices found with this API\n");
		closeDevice();
		return 0;
	}
	else {
		gu_log("[KA] %d device(s) found\n", numDevs);
		for (unsigned i=0; i<numDevs; i++)
			gu_log("  %d) %s\n", i, getDeviceName(i).c_str());
	}

	RtAudio::StreamParameters outParams;
	RtAudio::StreamParameters inParams;

	if (conf::soundDeviceOut == G_DEFAULT_SOUNDDEV_OUT)
		outParams.deviceId = getDefaultOut();
	else
		outParams.deviceId = conf::soundDeviceOut;

	outParams.nChannels = 2;
	outParams.firstChannel = conf::channelsOut * 2; // chan 0=0, 1=2, 2=4, ...

	/* inDevice can be disabled */

	if (conf::soundDeviceIn != -1) {
		inParams.deviceId     = conf::soundDeviceIn;
		inParams.nChannels    = 2;
		inParams.firstChannel = conf::channelsIn * 2;   // chan 0=0, 1=2, 2=4, ...
		inputEnabled = true;
	}
	else
		inputEnabled = false;

  RtAudio::StreamOptions options;
  options.streamName = G_APP_NAME;
  options.numberOfBuffers = 4;

	realBufsize = conf::buffersize;

#if defined(__linux__) || defined(__APPLE__)

	if (api == SYS_API_JACK) {
		conf::samplerate = getFreq(conf::soundDeviceOut, 0);
		gu_log("[KA] JACK in use, freq = %d\n", conf::samplerate);
	}

#endif

	try {
		rtSystem->openStream(
			&outParams, 					              // output params
			conf::soundDeviceIn != -1 ? &inParams : nullptr,  // input params if inDevice is selected
			RTAUDIO_FLOAT32,			              // audio format
			conf::samplerate, 					        // sample rate
			&realBufsize, 				              // buffer size in byte
			&mixer->masterPlay,                 // audio callback
			nullptr,									          // user data (unused)
			&options);
		return 1;
	}
	catch (RtAudioError &e) {
		gu_log("[KA] rtSystem init error: %s\n", e.getMessage().c_str());
		closeDevice();
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


int startStream()
{
	try {
		rtSystem->startStream();
		gu_log("[KA] latency = %lu\n", rtSystem->getStreamLatency());
		return 1;
	}
	catch (RtAudioError &e) {
		gu_log("[KA] Start stream error: %s\n", e.getMessage().c_str());
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


int stopStream()
{
	try {
		rtSystem->stopStream();
		return 1;
	}
	catch (RtAudioError &e) {
		gu_log("[KA] Stop stream error\n");
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


string getDeviceName(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).name;
	}
	catch (RtAudioError &e) {
		gu_log("[KA] invalid device ID = %d\n", dev);
		return "";
	}
}


/* -------------------------------------------------------------------------- */


int closeDevice()
{
	if (rtSystem->isStreamOpen()) {
#if defined(__linux__) || defined(__APPLE__)
		rtSystem->abortStream(); // stopStream seems to lock the thread
#elif defined(_WIN32)
		rtSystem->stopStream();	 // on Windows it's the opposite
#endif
		rtSystem->closeStream();
		delete rtSystem;
		rtSystem = nullptr;
	}
	return 1;
}


/* -------------------------------------------------------------------------- */


unsigned getMaxInChans(int dev)
{
	if (dev == -1) return 0;

	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).inputChannels;
	}
	catch (RtAudioError &e) {
		gu_log("[KA] Unable to get input channels\n");
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


unsigned getMaxOutChans(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).outputChannels;
	}
	catch (RtAudioError &e) {
		gu_log("[KA] Unable to get output channels\n");
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


bool isProbed(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).probed;
	}
	catch (RtAudioError &e) {
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


unsigned getDuplexChans(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).duplexChannels;
	}
	catch (RtAudioError &e) {
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


bool isDefaultIn(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).isDefaultInput;
	}
	catch (RtAudioError &e) {
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


bool isDefaultOut(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).isDefaultOutput;
	}
	catch (RtAudioError &e) {
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


int getTotalFreqs(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).sampleRates.size();
	}
	catch (RtAudioError &e) {
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


int	getFreq(unsigned dev, int i)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).sampleRates.at(i);
	}
	catch (RtAudioError &e) {
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


unsigned getRealBufSize()
{
  return realBufsize;
}


/* -------------------------------------------------------------------------- */


bool isInputEnabled()
{
  return inputEnabled;
}


/* -------------------------------------------------------------------------- */


unsigned countDevices()
{
  return numDevs;
}


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


int	getDeviceByName(const char *name)
{
	for (unsigned i=0; i<numDevs; i++)
		if (name == getDeviceName(i))
			return i;
	return -1;
}


/* -------------------------------------------------------------------------- */


bool hasAPI(int API)
{
	vector<RtAudio::Api> APIs;
	RtAudio::getCompiledApi(APIs);
	for (unsigned i=0; i<APIs.size(); i++)
		if (APIs.at(i) == API)
			return true;
	return false;
}


/* -------------------------------------------------------------------------- */


string getRtAudioVersion()
{
	return RtAudio::getVersion();
}


/* -------------------------------------------------------------------------- */


#ifdef __linux__


const JackState &jackTransportQuery()
{
	if (api != SYS_API_JACK)
    return jackState;
  jack_position_t position;
	jack_transport_state_t ts = jack_transport_query(jackGetHandle(), &position);
  jackState.running = ts != JackTransportStopped;
  jackState.bpm     = position.beats_per_minute;
  jackState.frame   = position.frame;
  return jackState;
}


/* -------------------------------------------------------------------------- */


void jackStart()
{
	if (api == SYS_API_JACK)
		jack_transport_start(jackGetHandle());
}


/* -------------------------------------------------------------------------- */


void jackSetPosition(uint32_t frame)
{
	if (api != SYS_API_JACK)
    return;
  jack_position_t position;
  jack_transport_query(jackGetHandle(), &position);
  position.frame = frame;
  jack_transport_reposition(jackGetHandle(), &position);
}


/* -------------------------------------------------------------------------- */


void jackSetBpm(double bpm)
{
  if (api != SYS_API_JACK)
    return;
  jack_position_t position;
  jack_transport_query(jackGetHandle(), &position);
  position.valid = jack_position_bits_t::JackPositionBBT;
  position.bar  = 0;  // no such info from Giada
  position.beat = 0;  // no such info from Giada
  position.tick = 0;  // no such info from Giada
  position.beats_per_minute = bpm;
  jack_transport_reposition(jackGetHandle(), &position);
}


/* -------------------------------------------------------------------------- */


void jackStop()
{
	if (api == SYS_API_JACK)
		jack_transport_stop(jackGetHandle());
}

}}; // giada::kernelAudio


#endif
