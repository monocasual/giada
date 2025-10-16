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

#include "src/const.h"
#include "src/core/const.h"
#include "src/core/types.h"
#include "src/gui/const.h"
#include "src/version.h"
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace geompp
{
void to_json(nlohmann::json&, const giada::SampleRange&);
void from_json(const nlohmann::json&, giada::SampleRange&);
} // namespace geompp

namespace giada::m
{
struct Patch
{
	struct Track
	{
		int             width    = G_DEFAULT_TRACK_WIDTH;
		bool            internal = false;
		std::vector<ID> channels;
	};

	struct Sample
	{
		ID          waveId = 0;
		SampleRange range;
	};
	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Sample, waveId, range);

	struct Channel
	{
		ID                       id              = 0;
		ChannelType              type            = ChannelType::SAMPLE;
		int                      height          = 0;
		std::vector<std::string> names           = {};
		int                      key             = 0;
		bool                     mute            = false;
		bool                     solo            = false;
		float                    volume          = G_DEFAULT_VOL;
		float                    pan             = G_DEFAULT_PAN;
		bool                     hasActions      = false;
		bool                     armed           = false;
		bool                     sendToMaster    = true;
		std::vector<int>         extraOutputs    = {};
		bool                     midiIn          = false;
		uint32_t                 midiInKeyPress  = 0x0;
		uint32_t                 midiInKeyRel    = 0x0;
		uint32_t                 midiInKill      = 0x0;
		uint32_t                 midiInArm       = 0x0;
		uint32_t                 midiInVolume    = 0x0;
		uint32_t                 midiInMute      = 0x0;
		uint32_t                 midiInSolo      = 0x0;
		int                      midiInFilter    = 0;
		bool                     midiOutL        = false;
		uint32_t                 midiOutLplaying = 0x0;
		uint32_t                 midiOutLmute    = 0x0;
		uint32_t                 midiOutLsolo    = 0x0;
		// sample channel
		std::vector<Sample> samples           = {};
		SamplePlayerMode    mode              = SamplePlayerMode::SINGLE_BASIC;
		Frame               shift             = 0;
		bool                readActions       = false;
		float               pitch             = G_DEFAULT_PITCH;
		bool                inputMonitor      = false;
		bool                overdubProtection = false;
		bool                midiInVeloAsVol   = false;
		uint32_t            midiInReadActions = 0x0;
		uint32_t            midiInPitch       = 0x0;
		// midi channel
		bool            midiOut     = false;
		int             midiOutChan = 0;
		std::vector<ID> pluginIds   = {};
	};

	struct Action
	{
		ID       id        = 0;
		ID       channelId = 0;
		Frame    frame     = 0;
		uint32_t event     = 0;
		ID       prevId    = 0;
		ID       nextId    = 0;
	};

	struct Wave
	{
		ID          id = 0;
		std::string path;
	};

	struct Plugin
	{
		ID                    id = 0;
		std::string           juceId;
		bool                  bypass = false;
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
