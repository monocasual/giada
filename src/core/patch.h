/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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


#ifndef G_PATCH_H
#define G_PATCH_H


#include <string>
#include <vector>
#ifdef __APPLE__  // our Clang still doesn't know about cstdint (c++11 stuff)
	#include <stdint.h>
#else
	#include <cstdint>
#endif


namespace giada {
namespace patch
{
struct action_t
{
  int      type;
  int      frame;
  float    fValue;
  uint32_t iValue;
};

#ifdef WITH_VST
struct plugin_t
{
  std::string           path;
  bool                  bypass;
  std::vector<float>    params;
  std::vector<uint32_t> midiInParams;
};
#endif

struct channel_t
{
  int         type;
  int         index;
  int         column;
  int         mute;
  int         mute_s;
  int         solo;
  float       volume;
  float       panLeft;
  float       panRight;
  bool        midiIn;
  uint32_t    midiInKeyPress;
  uint32_t    midiInKeyRel;
  uint32_t    midiInKill;
  uint32_t    midiInArm;
  uint32_t    midiInVolume;
  uint32_t    midiInMute;
  uint32_t    midiInSolo;
  bool        midiOutL;
  uint32_t    midiOutLplaying;
  uint32_t    midiOutLmute;
  uint32_t    midiOutLsolo;
  // sample channel
  std::string samplePath;
  int         key;
  int         mode;
  int         begin;
  int         end;
  float       boost;
  int         recActive;
  float       pitch;
  bool        inputMonitor;
  uint32_t    midiInReadActions;
  uint32_t    midiInPitch;
  // midi channel
  uint32_t    midiOut;
  uint32_t    midiOutChan;

  std::vector<action_t> actions;

#ifdef WITH_VST
  std::vector<plugin_t> plugins;
#endif
};

struct column_t
{
  int index;
  int width;
  std::vector<int> channels;
};

extern std::string header;
extern std::string version;
extern int    versionMajor;
extern int    versionMinor;
extern int    versionPatch;
extern std::string name;
extern float  bpm;
extern int    bars;
extern int    beats;
extern int    quantize;
extern float  masterVolIn;
extern float  masterVolOut;
extern int    metronome;
extern int    lastTakeId;
extern int    samplerate;   // original samplerate when the patch was saved

extern std::vector<column_t>  columns;
extern std::vector<channel_t> channels;

#ifdef WITH_VST
extern std::vector<plugin_t> masterInPlugins;
extern std::vector<plugin_t> masterOutPlugins;
#endif

/* init
 * Init Patch with default values. */

void init();

/* read/write
 * Read/write patch to/from file. */

int write(const std::string &file);
int read (const std::string &file);
}};  // giada::patch::

#endif
