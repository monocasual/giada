/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/const.h"
#include "src/core/types.h"
#include "src/version.h"
#include <cstdint>
#include <string>
#include <vector>

namespace giada::m
{
struct Patch
{
	struct Track
	{
		int             width;
		bool            internal;
		std::vector<ID> channels;
	};

	struct Channel
	{
		ID          id;
		ChannelType type;
		int         height;
		std::string name;
		int         key;
		bool        mute;
		bool        solo;
		float       volume = G_DEFAULT_VOL;
		float       pan    = G_DEFAULT_PAN;
		bool        hasActions;
		bool        armed;
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
		// sample channel
		ID               waveId = 0;
		SamplePlayerMode mode;
		Frame            begin;
		Frame            end;
		Frame            shift;
		bool             readActions;
		float            pitch = G_DEFAULT_PITCH;
		bool             inputMonitor;
		bool             overdubProtection;
		bool             midiInVeloAsVol;
		uint32_t         midiInReadActions;
		uint32_t         midiInPitch;
		// midi channel
		bool            midiOut;
		int             midiOutChan;
		std::vector<ID> pluginIds;
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

	struct Wave
	{
		ID          id;
		std::string path;
	};

	struct Plugin
	{
		ID                    id;
		std::string           path;
		bool                  bypass;
		std::vector<float>    params; // TODO - to be removed in 0.18.0
		std::string           state;
		std::vector<uint32_t> midiInParams;
	};

	Version     version    = G_VERSION;
	int         status     = G_FILE_INVALID;
	std::string name       = G_DEFAULT_PATCH_NAME;
	int         bars       = G_DEFAULT_BARS;
	int         beats      = G_DEFAULT_BEATS;
	float       bpm        = G_DEFAULT_BPM;
	bool        quantize   = G_DEFAULT_QUANTIZE;
	int         samplerate = G_DEFAULT_SAMPLERATE;
	bool        metronome  = false;

	std::vector<Track>   tracks;
	std::vector<Channel> channels;
	std::vector<Action>  actions;
	std::vector<Wave>    waves;
	std::vector<Plugin>  plugins;
};
} // namespace giada::m

#endif
