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


#ifndef KERNELAUDIO_H
#define KERNELAUDIO_H


#include <string>
#ifdef __linux__
	#include <jack/jack.h>
	#include <jack/intclient.h>
	#include <jack/transport.h>
#endif


class RtAudio;


class KernelAudio
{
public:

#ifdef __linux__

  struct JackState
  {
    bool     running;
    double   bpm;
    uint32_t frame;
  } jackState;

#endif

	KernelAudio();

	int openDevice(int api,	int outDev,	int inDev, int outChan,	int inChan,
		int samplerate,	int buffersize);

	int closeDevice();

	int startStream();
	int stopStream();

	bool			  isProbed         (unsigned dev);
	bool		    isDefaultIn      (unsigned dev);
	bool			  isDefaultOut     (unsigned dev);
	std::string getDeviceName    (unsigned dev);
	unsigned    getMaxInChans    (int dev);
	unsigned    getMaxOutChans   (unsigned dev);
	unsigned    getDuplexChans   (unsigned dev);
	int         getTotalFreqs    (unsigned dev);
	int					getFreq          (unsigned dev, int i);
	int					getDeviceByName  (const char *name);
	int         getDefaultOut    ();
	int         getDefaultIn     ();
	bool        hasAPI           (int API);
	std::string getRtAudioVersion();

#ifdef __linux__

  void   jackStart();
  void   jackStop();
  void   jackLocate(jack_nframes_t n);
  void   jackReposition(jack_nframes_t n, double bpm, int bar, int beat);
  const JackState &jackTransportQuery();

#endif

	unsigned numDevs;
	bool 		 inputEnabled;
	unsigned realBufsize; 		// reale bufsize from the soundcard
	int      api;

private:

	RtAudio *system;

#ifdef __linux__

  jack_client_t *jackGetHandle();

#endif
};

#endif
