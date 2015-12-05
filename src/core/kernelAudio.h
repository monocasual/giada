/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * KernelAudio
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
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


#ifndef KERNELAUDIO_H
#define KERNELAUDIO_H


#include "../deps/rtaudio-mod/RtAudio.h"
#if defined(__linux__)
	#include <jack/jack.h>
	#include <jack/intclient.h>
	#include <jack/transport.h>
#endif


using std::string;


namespace kernelAudio {

	int openDevice(
			int api,
			int outDev,
			int inDev,
			int outChan,
			int inChan,
			int samplerate,
			int buffersize);
	int closeDevice();

	int startStream();
	int stopStream();

	bool			  isProbed         (unsigned dev);
	bool		    isDefaultIn      (unsigned dev);
	bool			  isDefaultOut     (unsigned dev);
	string      getDeviceName    (unsigned dev);
	unsigned    getMaxInChans    (int dev);
	unsigned    getMaxOutChans   (unsigned dev);
	unsigned    getDuplexChans   (unsigned dev);
	int         getTotalFreqs    (unsigned dev);
	int					getFreq          (unsigned dev, int i);
	int					getDeviceByName  (const char *name);
	int         getDefaultOut    ();
	int         getDefaultIn     ();
	bool        hasAPI           (int API);
	string      getRtAudioVersion();

#ifdef __linux__
	jack_client_t *jackGetHandle();
	void jackStart();
	void jackStop();
	void jackSetSyncCb();
	int  jackSyncCb(jack_transport_state_t state, jack_position_t *pos, void *arg);
#endif

	/* *** how to avoid multiple definition of ***
	 * When you declare a variable in a header file, every source file that
	 * includes that header, either directly or indirectly, gets its own
	 * separate copy of the variable. Then when you go to link all the .o
	 * files together, the linker sees that the variable is instantiated
	 * in a bunch of .o files. Make it extern in the header file and
	 * instantiate it in memory.cpp. */

	extern RtAudio  *system;
	extern unsigned  numDevs;
	extern bool 		 inputEnabled;
	extern unsigned  realBufsize; 		// reale bufsize from the soundcard
	extern int       api;
}

#endif
