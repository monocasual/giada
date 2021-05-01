/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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

#ifndef G_KERNELAUDIO_H
#define G_KERNELAUDIO_H

#include <optional>
#include <string>
#include <vector>
#ifdef WITH_AUDIO_JACK
#include <jack/intclient.h>
#include <jack/jack.h>
#include <jack/transport.h>
#endif

namespace giada::m::kernelAudio
{
#ifdef WITH_AUDIO_JACK

struct JackState
{
	bool     running;
	double   bpm;
	uint32_t frame;

	bool operator!=(const JackState& o) const;
};

#endif

struct Device
{
	size_t           index             = 0;
	bool             probed            = false;
	std::string      name              = "";
	int              maxOutputChannels = 0;
	int              maxInputChannels  = 0;
	int              maxDuplexChannels = 0;
	bool             isDefaultOut      = false;
	bool             isDefaultIn       = false;
	std::vector<int> sampleRates       = {};
};

int openDevice();
int closeDevice();
int startStream();
int stopStream();

bool                       isReady();
bool                       isInputEnabled();
unsigned                   getRealBufSize();
bool                       hasAPI(int API);
int                        getAPI();
void                       logCompiledAPIs();
Device                     getDevice(const char* name);
const std::vector<Device>& getDevices();

#ifdef WITH_AUDIO_JACK

void      jackStart();
void      jackStop();
void      jackSetPosition(uint32_t frame);
void      jackSetBpm(double bpm);
JackState jackTransportQuery();

#endif
} // namespace giada::m::kernelAudio

#endif
