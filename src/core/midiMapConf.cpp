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


#include <vector>
#include <string>
#include <cstring>
#include <dirent.h>
#include "../utils/string.h"
#include "../utils/log.h"
#include "../utils/fs.h"
#include "const.h"
#include "storager.h"
#include "midiMapConf.h"


using std::string;
using std::vector;


namespace giada {
namespace m {
namespace midimap
{
namespace
{
bool readInitCommands(json_t *jContainer)
{
	json_t *jInitCommands = json_object_get(jContainer, MIDIMAP_KEY_INIT_COMMANDS);
  if (!storager::checkArray(jInitCommands, MIDIMAP_KEY_INIT_COMMANDS))
    return 0;

	size_t commandIndex;
	json_t *jInitCommand;
	json_array_foreach(jInitCommands, commandIndex, jInitCommand) {

		string indexStr = "init command " + gu_iToString(commandIndex);
		if (!storager::checkObject(jInitCommand, indexStr.c_str()))
			return 0;

		message_t message;
    if (!storager::setInt(jInitCommand, MIDIMAP_KEY_CHANNEL, message.channel)) return 0;
    if (!storager::setString(jInitCommand, MIDIMAP_KEY_MESSAGE, message.valueStr)) return 0;
		message.value = strtoul(message.valueStr.c_str(), nullptr, 16);

    initCommands.push_back(message);
	}

	return 1;
}


/* -------------------------------------------------------------------------- */


bool readCommand(json_t *jContainer, message_t *msg, const string &key)
{
	json_t *jCommand = json_object_get(jContainer, key.c_str());
  if (!storager::checkObject(jCommand, key.c_str()))
    return 0;

  if (!storager::setInt(jCommand, MIDIMAP_KEY_CHANNEL, msg->channel)) return 0;
  if (!storager::setString(jCommand, MIDIMAP_KEY_MESSAGE, msg->valueStr)) return 0;

	return 1;
}


/* -------------------------------------------------------------------------- */


void parse(message_t *message)
{
	/* Remove '0x' part from the original string. */

	string input = message->valueStr;

	size_t f = input.find("0x");					// check if "0x" is there
	if (f!=std::string::npos)
		input = message->valueStr.replace(f, 2, "");

	/* Then transform string value into the actual uint32_t value, by parsing
	 * each char (i.e. nibble) in the original string. Substitute 'n' with
	 * zeros. */

	string output;
	for (unsigned i=0, p=24; i<input.length(); i++, p-=4) {
		if (input[i] == 'n') {
			output += '0';
			if (message->offset == -1) // do it once
				message->offset = p;
		}
		else
			output += input[i];
	}

	/* from string to uint32_t */

	message->value = strtoul(output.c_str(), nullptr, 16);

	gu_log("[parse] parsed chan=%d valueStr=%s value=%#x, offset=%d\n",
			message->channel, message->valueStr.c_str(), message->value, message->offset);
}

}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */



string brand;
string device;
vector<message_t> initCommands;
message_t muteOn;
message_t muteOff;
message_t soloOn;
message_t soloOff;
message_t waiting;
message_t playing;
message_t stopping;
message_t stopped;
message_t playing_inaudible;

string midimapsPath;
vector<string> maps;


/* -------------------------------------------------------------------------- */


void init()
{
	midimapsPath = gu_getHomePath() + G_SLASH + "midimaps" + G_SLASH;

	/* scan dir of midi maps and load the filenames into <>maps. */

	gu_log("[init] scanning midimaps directory...\n");

  DIR    *dp;
  dirent *ep;
  dp = opendir(midimapsPath.c_str());

	if (!dp) {
		gu_log("[init] unable to scan midimaps directory!\n");
		return;
	}

	while ((ep = readdir(dp))) {
		if (!strcmp(ep->d_name, ".") || !strcmp(ep->d_name, ".."))
			continue;

		// TODO - check if is a valid midimap file (verify headers)

		gu_log("[init] found midimap '%s'\n", ep->d_name);

		maps.push_back(ep->d_name);
	}

	gu_log("[init] total midimaps found: %d\n", maps.size());
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
	playing_inaudible.channel   = 0;
	playing_inaudible.valueStr  = "";
	playing_inaudible.offset    = -1;
	playing_inaudible.value     = 0;
}


/* -------------------------------------------------------------------------- */


bool isDefined(message_t msg)
{
	return (msg.offset!=-1);
}


/* -------------------------------------------------------------------------- */


int read(const string &file)
{
	if (file.empty()) {
		gu_log("[read] midimap not specified, nothing to do\n");
		return MIDIMAP_NOT_SPECIFIED;
	}

	gu_log("[read] reading midimap file '%s'\n", file.c_str());

  json_error_t jError;
	string path = midimapsPath + file;
  json_t *jRoot = json_load_file(path.c_str(), 0, &jError);
	if (!jRoot) {
    gu_log("[read] unreadable midimap file. Error on line %d: %s\n", jError.line, jError.text);
    return MIDIMAP_UNREADABLE;
  }

	if (!storager::setString(jRoot, MIDIMAP_KEY_BRAND, brand))   return MIDIMAP_UNREADABLE;
    if (!storager::setString(jRoot, MIDIMAP_KEY_DEVICE, device)) return MIDIMAP_UNREADABLE;
	if (!readInitCommands(jRoot)) return MIDIMAP_UNREADABLE;
	if (readCommand(jRoot, &muteOn,   MIDIMAP_KEY_MUTE_ON))  parse(&muteOn);
	if (readCommand(jRoot, &muteOff,  MIDIMAP_KEY_MUTE_OFF)) parse(&muteOff);
	if (readCommand(jRoot, &soloOn,   MIDIMAP_KEY_SOLO_ON))  parse(&soloOn);
	if (readCommand(jRoot, &soloOff,  MIDIMAP_KEY_SOLO_OFF)) parse(&soloOff);
	if (readCommand(jRoot, &waiting,  MIDIMAP_KEY_WAITING))  parse(&waiting);
	if (readCommand(jRoot, &playing,  MIDIMAP_KEY_PLAYING))  parse(&playing);
	if (readCommand(jRoot, &stopping, MIDIMAP_KEY_STOPPING)) parse(&stopping);
	if (readCommand(jRoot, &stopped,  MIDIMAP_KEY_STOPPED))  parse(&stopped);
	if (readCommand(jRoot, &playing_inaudible,  MIDIMAP_KEY_PLAYING_INAUDIBLE))  parse(&playing_inaudible);

	return MIDIMAP_READ_OK;
}

}}}; // giada::m::midimap::
