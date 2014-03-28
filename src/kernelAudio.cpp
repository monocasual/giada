/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * KernelAudio
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#include <vector>
#include "rtaudio/RtAudio.h"
#include "kernelAudio.h"
#include "glue.h"
#include "conf.h"
#include "log.h"


extern Mixer G_Mixer;
extern Conf  G_Conf;
extern bool	 G_audio_status;


namespace kernelAudio {

RtAudio  *system       = NULL;
unsigned  numDevs      = 0;
bool 		  inputEnabled = 0;
unsigned  realBufsize  = 0;
int       api          = 0;

int openDevice(
	int _api,
	int outDev,
	int inDev,
	int outChan,
	int inChan,
	int samplerate,
	int buffersize)
{
	api = _api;
	gLog("[KA] using system 0x%x\n", api);
#if defined(__linux__)
	if (api == SYS_API_JACK && hasAPI(RtAudio::UNIX_JACK))
		system = new RtAudio(RtAudio::UNIX_JACK);
	else
	if (api == SYS_API_ALSA && hasAPI(RtAudio::LINUX_ALSA))
		system = new RtAudio(RtAudio::LINUX_ALSA);
	else
	if (api == SYS_API_PULSE && hasAPI(RtAudio::LINUX_PULSE))
		system = new RtAudio(RtAudio::LINUX_PULSE);
#elif defined(_WIN32)
	if (api == SYS_API_DS && hasAPI(RtAudio::WINDOWS_DS))
		system = new RtAudio(RtAudio::WINDOWS_DS);
	else
	if (api == SYS_API_ASIO && hasAPI(RtAudio::WINDOWS_ASIO))
		system = new RtAudio(RtAudio::WINDOWS_ASIO);
#elif defined(__APPLE__)
	if (api == SYS_API_CORE && hasAPI(RtAudio::MACOSX_CORE))
		system = new RtAudio(RtAudio::MACOSX_CORE);
#endif
	else {
		G_audio_status = false;
		return 0;
	}



	//gLog("[KA] %d\n", sizeof(system->rtapi_));

	gLog("[KA] Opening devices %d (out), %d (in), f=%d...\n", outDev, inDev, samplerate);

	numDevs = system->getDeviceCount();

	if (numDevs < 1) {
		gLog("[KA] no devices found with this API\n");
		closeDevice();
		G_audio_status = false;
		return 0;
	}
	else {
		gLog("[KA] %d device(s) found\n", numDevs);
		for (unsigned i=0; i<numDevs; i++)
			gLog("  %d) %s\n", i, getDeviceName(i));
	}


	RtAudio::StreamParameters outParams;
	RtAudio::StreamParameters inParams;

	if (outDev == DEFAULT_SOUNDDEV_OUT)
		outParams.deviceId = getDefaultOut();
	else
		outParams.deviceId = outDev;
	outParams.nChannels = 2;
	outParams.firstChannel = outChan*2; // chan 0=0, 1=2, 2=4, ...

	/* inDevice can be disabled */

	if (inDev != -1) {
		inParams.deviceId     = inDev;
		inParams.nChannels    = 2;
		inParams.firstChannel = inChan*2;   // chan 0=0, 1=2, 2=4, ...
		inputEnabled = true;
	}
	else
		inputEnabled = false;


  RtAudio::StreamOptions options;
  options.streamName = "Giada";
  options.numberOfBuffers = 4;

	realBufsize = buffersize;

#if defined(__linux__) || defined(__APPLE__)
	if (api == SYS_API_JACK) {
		samplerate = getFreq(outDev, 0);
		gLog("[KA] JACK in use, freq = %d\n", samplerate);
		G_Conf.samplerate = samplerate;
	}
#endif

	try {
		if (inDev != -1) {
			system->openStream(
				&outParams, 					// output params
				&inParams, 			  		// input params
				RTAUDIO_FLOAT32,			// audio format
				samplerate, 					// sample rate
				&realBufsize, 				// buffer size in byte
				&G_Mixer.masterPlay,  // audio callback
				NULL,									// user data (unused)
				&options);
		}
		else {
			system->openStream(
				&outParams, 					// output params
				NULL, 	     		  		// input params
				RTAUDIO_FLOAT32,			// audio format
				samplerate, 					// sample rate
				&realBufsize, 				// buffer size in byte
				&G_Mixer.masterPlay,  // audio callback
				NULL,									// user data (unused)
				&options);
		}
		G_audio_status = true;

#if defined(__linux__)
		if (api == SYS_API_JACK)
			jackSetSyncCb();
#endif

		return 1;
	}
	catch (RtError &e) {
		gLog("[KA] system init error: %s\n", e.getMessage().c_str());
		closeDevice();
		G_audio_status = false;
		return 0;
	}
}


/* ------------------------------------------------------------------ */


int startStream() {
	try {
		system->startStream();
		gLog("[KA] latency = %lu\n", system->getStreamLatency());
		return 1;
	}
	catch (RtError &e) {
		gLog("[KA] Start stream error\n");
		return 0;
	}
}


/* ------------------------------------------------------------------ */


int stopStream() {
	try {
		system->stopStream();
		return 1;
	}
	catch (RtError &e) {
		gLog("[KA] Stop stream error\n");
		return 0;
	}
}


/* ------------------------------------------------------------------ */


const char *getDeviceName(unsigned dev) {
	try {
		return ((RtAudio::DeviceInfo) system->getDeviceInfo(dev)).name.c_str();
	}
	catch (RtError &e) {
		gLog("[KA] invalid device ID = %d\n", dev);
		return NULL;
	}
}


/* ------------------------------------------------------------------ */


int closeDevice() {
	if (system->isStreamOpen()) {
#if defined(__linux__) || defined(__APPLE__)
		system->abortStream(); // stopStream seems to lock the thread
#elif defined(_WIN32)
		system->stopStream();	 // on Windows it's the opposite
#endif
		system->closeStream();
		delete system;
		system = NULL;
	}
	return 1;
}


/* ------------------------------------------------------------------ */


unsigned getMaxInChans(int dev) {

	if (dev == -1) return 0;

	try {
		return ((RtAudio::DeviceInfo) system->getDeviceInfo(dev)).inputChannels;
	}
	catch (RtError &e) {
		gLog("[KA] Unable to get input channels\n");
		return 0;
	}
}


/* ------------------------------------------------------------------ */


unsigned getMaxOutChans(unsigned dev) {
	try {
		return ((RtAudio::DeviceInfo) system->getDeviceInfo(dev)).outputChannels;
	}
	catch (RtError &e) {
		gLog("[KA] Unable to get output channels\n");
		return 0;
	}
}


/* ------------------------------------------------------------------ */


bool isProbed(unsigned dev) {
	try {
		return ((RtAudio::DeviceInfo) system->getDeviceInfo(dev)).probed;
	}
	catch (RtError &e) {
		return 0;
	}
}


/* ------------------------------------------------------------------ */


unsigned getDuplexChans(unsigned dev) {
	try {
		return ((RtAudio::DeviceInfo) system->getDeviceInfo(dev)).duplexChannels;
	}
	catch (RtError &e) {
		return 0;
	}
}


/* ------------------------------------------------------------------ */


bool isDefaultIn(unsigned dev) {
	try {
		return ((RtAudio::DeviceInfo) system->getDeviceInfo(dev)).isDefaultInput;
	}
	catch (RtError &e) {
		return 0;
	}
}


/* ------------------------------------------------------------------ */


bool isDefaultOut(unsigned dev) {
	try {
		return ((RtAudio::DeviceInfo) system->getDeviceInfo(dev)).isDefaultOutput;
	}
	catch (RtError &e) {
		return 0;
	}
}


/* ------------------------------------------------------------------ */


int getTotalFreqs(unsigned dev) {
	try {
		return ((RtAudio::DeviceInfo) system->getDeviceInfo(dev)).sampleRates.size();
	}
	catch (RtError &e) {
		return 0;
	}
}


/* ------------------------------------------------------------------ */


int	getFreq(unsigned dev, int i) {
	try {
		return ((RtAudio::DeviceInfo) system->getDeviceInfo(dev)).sampleRates.at(i);
	}
	catch (RtError &e) {
		return 0;
	}
}


/* ------------------------------------------------------------------ */


int getDefaultIn() {
	return system->getDefaultInputDevice();
}

int getDefaultOut() {
	return system->getDefaultOutputDevice();
}


/* ------------------------------------------------------------------ */


int	getDeviceByName(const char *name) {
	for (unsigned i=0; i<numDevs; i++)
		if (strcmp(name, getDeviceName(i))==0)
			return i;
	return -1;
}


/* ------------------------------------------------------------------ */


bool hasAPI(int API) {
	std::vector<RtAudio::Api> APIs;
	RtAudio::getCompiledApi(APIs);
	for (unsigned i=0; i<APIs.size(); i++)
		if (APIs.at(i) == API)
			return true;
	return false;
}


/* ------------------------------------------------------------------ */


std::string getRtAudioVersion() {
	return RtAudio::getVersion();
}


/* ------------------------------------------------------------------ */


#ifdef __linux__
#include <jack/jack.h>
#include <jack/intclient.h>
#include <jack/transport.h>

jack_client_t *jackGetHandle() {
	return (jack_client_t*) system->rtapi_->__HACK__getJackClient();
}

void jackStart() {
	if (api == SYS_API_JACK) {
		jack_client_t *client = jackGetHandle();
		jack_transport_start(client);
	}
}


void jackStop() {
	if (api == SYS_API_JACK) {
		jack_client_t *client = jackGetHandle();
		jack_transport_stop(client);
	}
}


void jackSetSyncCb() {
	jack_client_t *client = jackGetHandle();
	jack_set_sync_callback(client, jackSyncCb, NULL);
	//jack_set_sync_timeout(client, 8);
}


int jackSyncCb(jack_transport_state_t state, jack_position_t *pos,
		void *arg)
{
	switch (state) {
		case JackTransportStopped:
			gLog("[KA] Jack transport stopped, frame=%d\n", pos->frame);
			glue_stopSeq(false);  // false = not from GUI
			if (pos->frame == 0)
				glue_rewindSeq();
			break;

		case JackTransportRolling:
			gLog("[KA] Jack transport rolling\n");
			break;

		case JackTransportStarting:
			gLog("[KA] Jack transport starting, frame=%d\n", pos->frame);
			glue_startSeq(false);  // false = not from GUI
			if (pos->frame == 0)
				glue_rewindSeq();
			break;

		default:
			gLog("[KA] Jack transport [unknown]\n");
	}
	return 1;
}

#endif

}


