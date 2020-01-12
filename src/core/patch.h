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


#ifndef G_PATCH_H
#define G_PATCH_H


#include <string>
#include <vector>
#include <cstdint>
#include "core/types.h"
#include "core/const.h"


namespace giada {
namespace m {
namespace patch
{
struct Version
{
	int major = G_VERSION_MAJOR;
	int minor = G_VERSION_MINOR;
	int patch = G_VERSION_PATCH;

	bool operator ==(const Version& o) const;
	bool operator <(const Version& o) const;
};


struct Action
{
	ID       id;
	ID       channelId;
	Frame    frame;
	uint32_t event;
	ID       prevId;
	ID       nextId;
};


#ifdef WITH_VST
struct Plugin
{
	ID                    id;
	std::string           path;
	bool                  bypass;
	std::vector<float>    params;
	std::vector<uint32_t> midiInParams;
};
#endif


struct Wave
{
	ID          id;
	std::string path;
};


struct Channel
{
	ID          id;
	ChannelType type;
	int         size;
	std::string name;
	ID          columnId;
	int         key;
	bool        mute;
	bool        solo;
	float       volume = G_DEFAULT_VOL;
	float       pan;
	bool        hasActions;
	bool        midiIn;
	uint32_t    midiInKeyPress;
	uint32_t    midiInKeyRel;
	uint32_t    midiInKill;
	uint32_t    midiInArm;
	uint32_t    midiInVolume;
	uint32_t    midiInMute;
	uint32_t    midiInSolo;
	int         midiInFilter;
	bool        midiOutL;
	uint32_t    midiOutLplaying;
	uint32_t    midiOutLmute;
	uint32_t    midiOutLsolo;
	bool        armed;
	// sample channel
	ID          waveId;
	ChannelMode mode;
	Frame       begin;
	Frame       end;
	// TODO - shift
	bool        readActions;
	float       pitch = G_DEFAULT_PITCH;
	bool        inputMonitor;
	bool        midiInVeloAsVol;
	uint32_t    midiInReadActions;
	uint32_t    midiInPitch;
	// midi channel
	bool        midiOut;
	int         midiOutChan;
#ifdef WITH_VST
	std::vector<ID> pluginIds;
#endif
};

extern std::string name;
extern int         samplerate;   // Original samplerate when the patch was saved
extern int         lastTakeId;
extern bool        metronome;

/* init
Initializes the patch with default values. */

void init();

/* verify
Checks if the patch is valid. */

int verify(const std::string& file);

/* read
Reads patch from file. Always call verify() first in order to see if the patch
format is valid. It takes 'basePath' as parameter for Wave reading.*/

int read(const std::string& file, const std::string& basePath);

/* write
Writes patch to file. */

bool write(const std::string& name, const std::string& file, bool isProject);
}}};  // giada::m::patch::


#endif
