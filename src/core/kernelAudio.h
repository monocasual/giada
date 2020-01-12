/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#ifndef G_KERNELAUDIO_H
#define G_KERNELAUDIO_H


#include <string>
#if defined(__linux__) || defined(__FreeBSD__)
	#include <jack/jack.h>
	#include <jack/intclient.h>
	#include <jack/transport.h>
#endif


namespace giada {
namespace m {
namespace kernelAudio
{
#if defined(__linux__) || defined(__FreeBSD__)

struct JackState
{
  bool running;
  double bpm;
  uint32_t frame;
};

#endif

int openDevice();
int closeDevice();
int startStream();
int stopStream();

bool isReady();
bool isProbed(unsigned dev);
bool isDefaultIn(unsigned dev);
bool isDefaultOut(unsigned dev);
bool isInputEnabled();
std::string getDeviceName(unsigned dev);
unsigned getMaxInChans(int dev);
unsigned getMaxOutChans(unsigned dev);
unsigned getDuplexChans(unsigned dev);
unsigned getRealBufSize();
unsigned countDevices();
int getTotalFreqs(unsigned dev);
int getFreq(unsigned dev, int i);
int getDeviceByName(const char* name);
int getDefaultOut();
int getDefaultIn();
bool hasAPI(int API);
int getAPI();

#if defined(__linux__) || defined(__FreeBSD__)

void jackStart();
void jackStop();
void jackSetPosition(uint32_t frame);
void jackSetBpm(double bpm);
const JackState &jackTransportQuery();

#endif
}}}; // giada::m::kernelAudio::

#endif
