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


#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <dirent.h>
#include "deps/json/single_include/nlohmann/json.hpp"
#include "utils/string.h"
#include "utils/log.h"
#include "utils/fs.h"
#include "const.h"
#include "midiMapConf.h"


namespace nl = nlohmann;


namespace giada {
namespace m {
namespace midimap
{
namespace
{
bool readInitCommands_(const nl::json& j)
{
	if (j.find(MIDIMAP_KEY_INIT_COMMANDS) == j.end())
		return false;

	for (const auto& jc : j[MIDIMAP_KEY_INIT_COMMANDS])
	{
		Message m;
		m.channel  = jc[MIDIMAP_KEY_CHANNEL];
		m.valueStr = jc[MIDIMAP_KEY_MESSAGE];
		m.value    = strtoul(m.valueStr.c_str(), nullptr, 16);

		midimap.initCommands.push_back(m);
	}

	return true;
}


/* -------------------------------------------------------------------------- */


bool readCommand_(const nl::json& j, Message& m, const std::string& key)
{
	if (j.find(key) == j.end())
		return false;

	const nl::json& jc = j[key];

	m.channel  = jc[MIDIMAP_KEY_CHANNEL];
	m.valueStr = jc[MIDIMAP_KEY_MESSAGE];

	return true;
}


/* -------------------------------------------------------------------------- */


void parse_(Message& message)
{
	/* Remove '0x' part from the original string. */

	std::string input = message.valueStr;

	std::size_t f = input.find("0x"); // check if "0x" is there
	if (f != std::string::npos)
		input = message.valueStr.replace(f, 2, "");

	/* Then transform string value into the actual uint32_t value, by parsing
	each char (i.e. nibble) in the original string. Substitute 'n' with
	zeros. */

	std::string output;
	for (unsigned i=0, p=24; i<input.length(); i++, p-=4) {
		if (input[i] == 'n') {
			output += '0';
			if (message.offset == -1) // do it once
				message.offset = p;
		}
		else
			output += input[i];
	}

	/* From string to uint32_t */

	message.value = strtoul(output.c_str(), nullptr, 16);

	u::log::print("[parse] parsed chan=%d valueStr=%s value=%#x, offset=%d\n",
			message.channel, message.valueStr.c_str(), message.value, message.offset);
}

}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


MidiMap                  midimap;
std::string              midimapsPath;
std::vector<std::string> maps;


/* -------------------------------------------------------------------------- */


void init()
{
	midimapsPath = u::fs::getHomePath() + G_SLASH + "midimaps" + G_SLASH;

	/* scan dir of midi maps and load the filenames into <>maps. */

	u::log::print("[midiMapConf::init] scanning midimaps directory '%s'...\n",
		midimapsPath.c_str());

	DIR*    dp;
	dirent* ep;
	dp = opendir(midimapsPath.c_str());

	if (dp == nullptr) {
		u::log::print("[midiMapConf::init] unable to scan midimaps directory!\n");
		return;
	}

	while ((ep = readdir(dp))) {
		if (!strcmp(ep->d_name, ".") || !strcmp(ep->d_name, ".."))
			continue;

		// TODO - check if is a valid midimap file (verify headers)

		u::log::print("[midiMapConf::init] found midimap '%s'\n", ep->d_name);

		maps.push_back(ep->d_name);
	}

	u::log::print("[midiMapConf::init] total midimaps found: %d\n", maps.size());
	closedir(dp);
}


/* -------------------------------------------------------------------------- */


void setDefault()
{
	midimap = MidiMap();
}


/* -------------------------------------------------------------------------- */


bool isDefined(const Message& m)
{
	return m.offset != -1;
}


/* -------------------------------------------------------------------------- */


int read(const std::string& file)
{
	if (file.empty()) {
		u::log::print("[midiMapConf::read] midimap not specified, nothing to do\n");
		return MIDIMAP_NOT_SPECIFIED;
	}

	u::log::print("[midiMapConf::read] reading midimap file '%s'\n", file.c_str());

	std::ifstream ifs(midimapsPath + file);
	if (!ifs.good())
		return MIDIMAP_UNREADABLE;

	nl::json j = nl::json::parse(ifs);

	midimap.brand  = j[MIDIMAP_KEY_BRAND];
	midimap.device = j[MIDIMAP_KEY_DEVICE];
	
	if (!readInitCommands_(j)) return MIDIMAP_UNREADABLE;
	if (readCommand_(j, midimap.muteOn,           MIDIMAP_KEY_MUTE_ON))  parse_(midimap.muteOn);
	if (readCommand_(j, midimap.muteOff,          MIDIMAP_KEY_MUTE_OFF)) parse_(midimap.muteOff);
	if (readCommand_(j, midimap.soloOn,           MIDIMAP_KEY_SOLO_ON))  parse_(midimap.soloOn);
	if (readCommand_(j, midimap.soloOff,          MIDIMAP_KEY_SOLO_OFF)) parse_(midimap.soloOff);
	if (readCommand_(j, midimap.waiting,          MIDIMAP_KEY_WAITING))  parse_(midimap.waiting);
	if (readCommand_(j, midimap.playing,          MIDIMAP_KEY_PLAYING))  parse_(midimap.playing);
	if (readCommand_(j, midimap.stopping,         MIDIMAP_KEY_STOPPING)) parse_(midimap.stopping);
	if (readCommand_(j, midimap.stopped,          MIDIMAP_KEY_STOPPED))  parse_(midimap.stopped);
	if (readCommand_(j, midimap.playingInaudible, MIDIMAP_KEY_PLAYING_INAUDIBLE))  parse_(midimap.playingInaudible);

	return MIDIMAP_READ_OK;
}
}}}; // giada::m::midimap::
