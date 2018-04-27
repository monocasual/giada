/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#include "../core/const.h"
#ifdef G_OS_MAC
	#include <RtMidi.h>
#else
	#include <rtmidi/RtMidi.h>
#endif
#include <sndfile.h>
#include "../deps/rtaudio-mod/RtAudio.h"
#include "ver.h"


using std::string;


namespace giada {
namespace u     {
namespace ver  
{
string getLibsndfileVersion()
{
  char buffer[128];
  sf_command(NULL, SFC_GET_LIB_VERSION, buffer, sizeof(buffer));
  return string(buffer);
}


/* -------------------------------------------------------------------------- */


string getRtAudioVersion()
{
#ifdef TESTS
		return "";
#else
  return RtAudio::getVersion();
#endif
}


/* -------------------------------------------------------------------------- */


string getRtMidiVersion()
{
#ifdef TESTS
		return "";
#else
  return RtMidi::getVersion();
#endif
}


/* -------------------------------------------------------------------------- */


bool isLess(int a1, int b1, int c1, int a2, int b2, int c2)
{
	const int s = 3;
	int v1[s] = {a1, b1, c1};
	int v2[s] = {a2, b2, c2};

	for (int i=0; i<s; i++) {
		if (v1[i] == v2[i])
			continue;
		return v1[i] < v2[i];
	}
	return false;
}

}}};  // giada::u::ver::