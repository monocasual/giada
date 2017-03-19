/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * patch
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


#ifndef __PATCH_H__
#define __PATCH_H__


#include <string>
#include <vector>
#include <stdint.h>
#include "dataStorageJson.h"


class Patch : public DataStorageJson
{
public:

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

  std::string header;
  std::string version;
  int    versionMajor;
  int    versionMinor;
  int    versionPatch;
  std::string name;
  float  bpm;
  int    bars;
  int    beats;
  int    quantize;
  float  masterVolIn;
  float  masterVolOut;
  int    metronome;
  int    lastTakeId;
  int    samplerate;   // original samplerate when the patch was saved

  std::vector<column_t>  columns;
  std::vector<channel_t> channels;

#ifdef WITH_VST
  std::vector<plugin_t> masterInPlugins;
  std::vector<plugin_t> masterOutPlugins;
#endif

  /* init
   * Init Patch with default values. */

  void init();

  /* read/write
   * Read/write patch to/from file. */

  int  write(const std::string &file);
  int  read (const std::string &file);

private:

  /* sanitize
   * Internal sanity check. */

  void sanitize();

  /* setInvalid
   * Helper function used to return invalid status while reading. */

  int setInvalid();

  /* readers */

  bool readCommons (json_t *jContainer);
  bool readChannels(json_t *jContainer);
#ifdef WITH_VST
  bool readPlugins (json_t *jContainer, std::vector<plugin_t> *container,
    const char* key);
#endif
  bool readActions (json_t *jContainer, channel_t *channel);
  bool readColumns (json_t *jContainer);

  /* writers */

  void writeCommons (json_t *jContainer);
  void writeChannels(json_t *jContainer, std::vector<channel_t> *channels);
#ifdef WITH_VST
  void writePlugins (json_t *jContainer, std::vector<plugin_t> *plugins,
    const char* key);
#endif
  void writeActions (json_t *jContainer, std::vector<action_t> *actions);
  void writeColumns (json_t *jContainer, std::vector<column_t> *columns);
};

#endif
