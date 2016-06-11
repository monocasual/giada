/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * midiMapConf
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#include <stdlib.h>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <dirent.h>
#include "midiMapConf.h"
#include "const.h"
#include "../utils/utils.h"
#include "../utils/log.h"


using std::string;
using std::vector;



void MidiMapConf::init()
{
	midimapsPath = gGetHomePath() + G_SLASH + "midimaps" + G_SLASH;

	/* scan dir of midi maps and load the filenames into <>maps. */

	gLog("[MidiMapConf::init] scanning midimaps directory...\n");

  DIR    *dp;
  dirent *ep;
  dp = opendir(midimapsPath.c_str());

	if (!dp) {
		gLog("[MidiMapConf::init] unable to scan midimaps directory!\n");
		return;
	}

	while ((ep = readdir(dp))) {
		if (!strcmp(ep->d_name, ".") || !strcmp(ep->d_name, ".."))
			continue;

		// TODO - check if is a valid midimap file (verify headers)

		gLog("[MidiMapConf::init] found midimap '%s'\n", ep->d_name);

		maps.push_back(ep->d_name);
	}

	gLog("[MidiMapConf::init] total midimaps found: %d\n", maps.size());
	closedir(dp);
}


/* -------------------------------------------------------------------------- */


void MidiMapConf::setDefault()
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
}


/* -------------------------------------------------------------------------- */


int MidiMapConf::read(const string &file)
{
	if (file.empty()) {
		gLog("[MidiMapConf::read] midimap not specified, nothing to do\n");
		return MIDIMAP_NOT_SPECIFIED;
	}

	gLog("[MidiMapConf::read] reading midimap file '%s'\n", file.c_str());

	string path = midimapsPath + file;
	jRoot = json_load_file(path.c_str(), 0, &jError);
	if (!jRoot) {
    gLog("[MidiMapConf::read] unreadable midimap file. Error on line %d: %s\n", jError.line, jError.text);
    return MIDIMAP_UNREADABLE;
  }

	if (!setString(jRoot, MIDIMAP_KEY_BRAND, brand))   return MIDIMAP_UNREADABLE;
  if (!setString(jRoot, MIDIMAP_KEY_DEVICE, device)) return MIDIMAP_UNREADABLE;
	if (!readInitCommands(jRoot)) return MIDIMAP_UNREADABLE;
	if (!readCommand(jRoot, &muteOn,   MIDIMAP_KEY_MUTE_ON))  return MIDIMAP_UNREADABLE;
	if (!readCommand(jRoot, &muteOff,  MIDIMAP_KEY_MUTE_OFF)) return MIDIMAP_UNREADABLE;
	if (!readCommand(jRoot, &soloOn,   MIDIMAP_KEY_SOLO_ON))  return MIDIMAP_UNREADABLE;
	if (!readCommand(jRoot, &soloOff,  MIDIMAP_KEY_SOLO_OFF)) return MIDIMAP_UNREADABLE;
	if (!readCommand(jRoot, &waiting,  MIDIMAP_KEY_WAITING))  return MIDIMAP_UNREADABLE;
	if (!readCommand(jRoot, &playing,  MIDIMAP_KEY_PLAYING))  return MIDIMAP_UNREADABLE;
	if (!readCommand(jRoot, &stopping, MIDIMAP_KEY_STOPPING)) return MIDIMAP_UNREADABLE;
	if (!readCommand(jRoot, &stopped,  MIDIMAP_KEY_STOPPED))  return MIDIMAP_UNREADABLE;

	/* parse messages */

	parse(&muteOn);
	parse(&muteOff);
	parse(&soloOn);
	parse(&soloOff);
	parse(&waiting);
	parse(&playing);
	parse(&stopping);
	parse(&stopped);

	return MIDIMAP_READ_OK;
}


/* -------------------------------------------------------------------------- */


bool MidiMapConf::readInitCommands(json_t *jContainer)
{
	json_t *jInitCommands = json_object_get(jContainer, MIDIMAP_KEY_INIT_COMMANDS);
  if (!checkArray(jInitCommands, MIDIMAP_KEY_INIT_COMMANDS))
    return 0;

	size_t commandIndex;
	json_t *jInitCommand;
	json_array_foreach(jInitCommands, commandIndex, jInitCommand) {

		string indexStr = "init command " + gItoa(commandIndex);
		if (!checkObject(jInitCommand, indexStr.c_str()))
			return 0;

		message_t message;
    if (!setInt(jInitCommand, MIDIMAP_KEY_CHANNEL, message.channel)) return 0;
    if (!setString(jInitCommand, MIDIMAP_KEY_MESSAGE, message.valueStr)) return 0;
		message.value = strtoul(message.valueStr.c_str(), NULL, 16);

    initCommands.push_back(message);
	}

	return 1;
}


/* -------------------------------------------------------------------------- */


bool MidiMapConf::readCommand(json_t *jContainer, message_t *msg, const string &key)
{
	json_t *jCommand = json_object_get(jContainer, key.c_str());
  if (!checkObject(jCommand, key.c_str()))
    return 0;

  if (!setInt(jCommand, MIDIMAP_KEY_CHANNEL, msg->channel)) return 0;
  if (!setString(jCommand, MIDIMAP_KEY_MESSAGE, msg->valueStr)) return 0;

	return 1;
}


/* -------------------------------------------------------------------------- */


void MidiMapConf::parse(message_t *message)
{
	/* Remove '0x' part from the original string. */

	string input = message->valueStr.replace(0, 2, "");

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

	message->value = strtoul(output.c_str(), NULL, 16);

	gLog("[MidiMapConf::parse] parsed chan=%d valueStr=%s value=%#x, offset=%d\n",
			message->channel, message->valueStr.c_str(), message->value, message->offset);
}


/* -------------------------------------------------------------------------- */


void MidiMapConf::setDefault_DEPR_()
{
	brand  = "";
	device = "";

	for (int i=0; i<MAX_INIT_COMMANDS; i++) {
		init_channels[i] = -1;
		init_messages[i] = 0x00;
	}

	muteOnChan     = 0;
	muteOnOffset   = 0;
	muteOnMsg      = 0;

	muteOffChan    = 0;
	muteOffOffset  = 0;
	muteOffMsg     = 0;

	soloOnChan     = 0;
	soloOnOffset   = 0;
	soloOnMsg      = 0;

	soloOffChan    = 0;
	soloOffOffset  = 0;
	soloOffMsg     = 0;

	waitingChan    = 0;
	waitingOffset  = 0;
	waitingMsg     = 0;

	playingChan    = 0;
	playingOffset  = 0;
	playingMsg     = 0;

	stoppingChan   = 0;
	stoppingOffset = 0;
	stoppingMsg    = 0;

	stoppedChan    = 0;
	stoppedOffset  = 0;
	stoppedMsg     = 0;
}


/* -------------------------------------------------------------------------- */


int MidiMapConf::readMap_DEPR_(string file)
{
	if (file.empty()) {
		gLog("[MidiMapConf::readMap_DEPR_] midimap not specified, nothing to do\n");
		return MIDIMAP_NOT_SPECIFIED;
	}

	gLog("[MidiMapConf::readMap_DEPR_] reading midimap file '%s'\n", file.c_str());

	string path = midimapsPath + file;
	fp = fopen(path.c_str(), "r");
	if (!fp) {
		gLog("[MidiMapConf::readMap_DEPR_] unreadable midimap file\n");
		return MIDIMAP_UNREADABLE;
	}

	brand  = getValue("brand");
	device = getValue("device");

	if (brand.empty() || device.empty()) {
		gLog("[MidiMapConf::readMap_DEPR_] invalid midimap file\n");
		return MIDIMAP_INVALID;
	}

	gLog("[MidiMapConf::readMap_DEPR_] reading midimap for %s %s\n",
			brand.c_str(), device.c_str());

	/* parse init commands */

	vector<string> ic;
	gSplit(getValue("init_commands"), ";", &ic);
	for (unsigned i=0; i<(unsigned)MAX_INIT_COMMANDS && i<ic.size(); i++) {
		sscanf(ic.at(i).c_str(), "%d:%x", &init_channels[i], &init_messages[i]);
		gLog("[MidiMapConf::readMap_DEPR_] init command %d - channel %d - message 0x%X\n",
				i, init_channels[i], init_messages[i]);

		/* forward compatibility */
		message_t message;
		message.channel = init_channels[i];
		message.value   = init_messages[i];
		initCommands.push_back(message);
	}

	/* parse messages */

	parse_DEPR_("mute_on",  &muteOnChan,   &muteOnMsg,   &muteOnOffset);
	parse_DEPR_("mute_off", &muteOffChan,  &muteOffMsg,  &muteOffOffset);
	parse_DEPR_("solo_on",  &soloOnChan,   &soloOnMsg,   &soloOnOffset);
	parse_DEPR_("solo_off", &soloOffChan,  &soloOffMsg,  &soloOffOffset);
	parse_DEPR_("waiting",  &waitingChan,  &waitingMsg,  &waitingOffset);
	parse_DEPR_("playing",  &playingChan,  &playingMsg,  &playingOffset);
	parse_DEPR_("stopping", &stoppingChan, &stoppingMsg, &stoppingOffset);
	parse_DEPR_("stopped",  &stoppedChan,  &stoppedMsg,  &stoppedOffset);

	/* forward compatibility with new JSON-based midimaps. This stuff will be
	wiped out soon. */

	muteOn.channel   = muteOnChan;
	muteOn.offset    = muteOnOffset;
	muteOn.value     = muteOnMsg;
	muteOff.channel  = muteOffChan;
	muteOff.offset   = muteOffOffset;
	muteOff.value    = muteOffMsg;
	soloOn.channel   = soloOnChan;
	soloOn.offset    = soloOnOffset;
	soloOn.value     = soloOnMsg;
	soloOff.channel  = soloOffChan;
	soloOff.offset   = soloOffOffset;
	soloOff.value    = soloOffMsg;
	waiting.channel  = waitingChan;
	waiting.offset   = waitingOffset;
	waiting.value    = waitingMsg;
	playing.channel  = playingChan;
	playing.offset   = playingOffset;
	playing.value    = playingMsg;
	stopping.channel = stoppingChan;
	stopping.offset  = stoppingOffset;
	stopping.value   = stoppingMsg;
	stopped.channel  = stoppedChan;
	stopped.offset   = stoppedOffset;
	stopped.value    = stoppedMsg;

	close_DEPR_();
	return MIDIMAP_READ_OK;
}


/* -------------------------------------------------------------------------- */


void MidiMapConf::close_DEPR_()
{
	if (fp != NULL)
		fclose(fp);
	fp = NULL;
}


/* -------------------------------------------------------------------------- */


void MidiMapConf::parse_DEPR_(string key, int *chan, uint32_t *msg, int *offset)
{
	gLog("[MidiMapConf::parse_DEPR_] command %s - ", key.c_str());
	string value = getValue(key.c_str());

	/* grab channel part, i.e. [channel]:*/

	*chan = atoi(value.substr(0, value.find(':')).c_str());

	/* grab MIDI part :[midi-message] and search for 'nn' note placeholder within.
	 * Note: when using 'string::npos' as the value for a len (or sublen)
	 * parameter in string's member functions, means "until the end of the
	 * string". */

	string midiParts = value.substr(value.find(':')+3, string::npos);

	char strmsg[MAX_MIDI_NIBBLES];
	*offset = 0;

	/* build the message as a string, for each char (i.e. nibble) in the
	 * original string. Substitute 'n' with zeros. */

	for (unsigned i=0, p=24; i<(unsigned)MAX_MIDI_NIBBLES; i++, p-=4) {
		if (midiParts[i] == 'n') {
			strmsg[i] = '0';
			if (*offset == 0)
				*offset = p;
		}
		else
			strmsg[i] = midiParts[i];
	}

	*msg = strtoul(strmsg, NULL, 16);  // from string to uint32_t

	gLog("chan=%d value=%s msg=%#x, offset=%d\n", *chan, midiParts.c_str(), *msg, *offset);
}
