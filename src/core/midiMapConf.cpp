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


#include <vector>
#include <string>
#include <cstring>
#include <dirent.h>
#include "utils/string.h"
#include "utils/log.h"
#include "utils/fs.h"
#include "utils/json.h"
#include "const.h"
#include "midiMapConf.h"


namespace giada {
namespace m {
namespace midimap
{
namespace
{
bool readInitCommands_(json_t* j)
{
	namespace uj = u::json;

	json_t* jcs = json_object_get(j, MIDIMAP_KEY_INIT_COMMANDS);
	if (jcs == nullptr)
    	return false;

	size_t  i;
	json_t* jc;
	json_array_foreach(jcs, i, jc) {

		if (!uj::isObject(jc))
			return false;

		Message m;
		m.channel  = uj::readInt   (jc, MIDIMAP_KEY_CHANNEL);
		m.valueStr = uj::readString(jc, MIDIMAP_KEY_MESSAGE);
		m.value    = strtoul(m.valueStr.c_str(), nullptr, 16);

		initCommands.push_back(m);
	}

	return true;
}


/* -------------------------------------------------------------------------- */


bool readCommand_(json_t* j, Message& m, const std::string& key)
{
	namespace uj = u::json;

	json_t* jc = json_object_get(j, key.c_str());
	if (jc == nullptr)
		return false;

	m.channel  = uj::readInt   (jc, MIDIMAP_KEY_CHANNEL);
	m.valueStr = uj::readString(jc, MIDIMAP_KEY_MESSAGE);

	return true;
}


/* -------------------------------------------------------------------------- */


void parse_(Message& message)
{
	/* Remove '0x' part from the original string. */

	std::string input = message.valueStr;

	size_t f = input.find("0x"); // check if "0x" is there
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



std::string brand;
std::string device;
std::vector<Message> initCommands;
Message muteOn;
Message muteOff;
Message soloOn;
Message soloOff;
Message waiting;
Message playing;
Message stopping;
Message stopped;
Message playingInaudible;

std::string midimapsPath;
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
	brand  = "";
	device = "";
	muteOn.channel    = 0;
	muteOn.valueStr   = "";
	muteOn.offset     = -1;
	muteOn.value      = 0;
	muteOff.channel   = 0;
	muteOff.valueStr  = "";
	muteOff.offset    = -1;
	muteOff.value     = 0;
	soloOn.channel    = 0;
	soloOn.valueStr   = "";
	soloOn.offset     = -1;
	soloOn.value      = 0;
	soloOff.channel   = 0;
	soloOff.valueStr  = "";
	soloOff.offset    = -1;
	soloOff.value     = 0;
	waiting.channel   = 0;
	waiting.valueStr  = "";
	waiting.offset    = -1;
	waiting.value     = 0;
	playing.channel   = 0;
	playing.valueStr  = "";
	playing.offset    = -1;
	playing.value     = 0;
	stopping.channel  = 0;
	stopping.valueStr = "";
	stopping.offset   = -1;
	stopping.value    = 0;
	stopped.channel   = 0;
	stopped.valueStr  = "";
	stopped.offset    = -1;
	stopped.value     = 0;
	playingInaudible.channel   = 0;
	playingInaudible.valueStr  = "";
	playingInaudible.offset    = -1;
	playingInaudible.value     = 0;
}


/* -------------------------------------------------------------------------- */


bool isDefined(const Message& m)
{
	return m.offset != -1;
}


/* -------------------------------------------------------------------------- */


int read(const std::string& file)
{
	namespace uj = u::json;

	if (file.empty()) {
		u::log::print("[midiMapConf::read] midimap not specified, nothing to do\n");
		return MIDIMAP_NOT_SPECIFIED;
	}

	u::log::print("[midiMapConf::read] reading midimap file '%s'\n", file.c_str());

	json_t* j = uj::load(std::string(midimapsPath + file).c_str());
	if (j == nullptr)
		return MIDIMAP_UNREADABLE;

	brand  = uj::readString(j, MIDIMAP_KEY_BRAND);
	device = uj::readString(j, MIDIMAP_KEY_DEVICE);
	
	if (!readInitCommands_(j)) return MIDIMAP_UNREADABLE;
	if (readCommand_(j, muteOn,           MIDIMAP_KEY_MUTE_ON))  parse_(muteOn);
	if (readCommand_(j, muteOff,          MIDIMAP_KEY_MUTE_OFF)) parse_(muteOff);
	if (readCommand_(j, soloOn,           MIDIMAP_KEY_SOLO_ON))  parse_(soloOn);
	if (readCommand_(j, soloOff,          MIDIMAP_KEY_SOLO_OFF)) parse_(soloOff);
	if (readCommand_(j, waiting,          MIDIMAP_KEY_WAITING))  parse_(waiting);
	if (readCommand_(j, playing,          MIDIMAP_KEY_PLAYING))  parse_(playing);
	if (readCommand_(j, stopping,         MIDIMAP_KEY_STOPPING)) parse_(stopping);
	if (readCommand_(j, stopped,          MIDIMAP_KEY_STOPPED))  parse_(stopped);
	if (readCommand_(j, playingInaudible, MIDIMAP_KEY_PLAYING_INAUDIBLE))  parse_(playingInaudible);

	return MIDIMAP_READ_OK;
}
}}}; // giada::m::midimap::
