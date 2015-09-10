/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * patch
 *
 * -----------------------------------------------------------------------------
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
 * -------------------------------------------------------------------------- */


#ifndef __PATCH_H__
#define __PATCH_H__

#include <stdio.h>
#include <string>
#include <stdint.h>
#include "dataStorageJson.h"
#include "const.h"


using std::string;


class Patch : public DataStorageJson
{
public:

	float version;
	int   lastTakeId;

  const char *header;
  const char *version;
  float       versionF;
  const char *name;
  
  int bpm;
  int bars;
  int beats;
  int quantize;
  int masterVolIn;
  int masterVolOut;
  int metronome;
  int lastTakeId;
  int samplerate;
  int numChannels; // not needed anymore
  int numColumns;  // not needed anymore
  
  struct channel 
  {
    int   type;
    int   index;
    int   column;
    int   mute;
    int   mute_s;
    int   solo;
    float vol;
    float panLeft;
    float panRight;
    
    bool     midiIn;
    uint32_t midiInKeyPress;
    uint32_t midiInKeyRel;
    uint32_t midiInKill;
    uint32_t midiInVolume;
    uint32_t midiInMute;
    uint32_t midiInSolo;
    
    bool     midiOutL;
    uint32_t midiOutLplaying;
    uint32_t midiOutLmute;
    uint32_t midiOutLsolo;
  };
  
  struct action
  {
    int      channel;
    int      type;
    int      frame;
    float    fValue;
    uint32_t iValue;
  };

#ifdef WITH_VST

  struct plugin
  {
    int         type;      // master in, master out, channel
    const char *path;
    int         bypass;
    int         numParams; // not needed anymore
  };
  
#endif
  
  int write(const char *file, const char *name, bool isProject);
};

#endif
