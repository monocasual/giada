/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * midiMapConf
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


#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <dirent.h>
#include "midiMapConf.h"
#include "const.h"
#include "utils.h"
#include "log.h"


using std::string;


void MidiMapConf::init()
{
	midimapsPath = gGetHomePath() + "/midimaps/";

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

		maps.add(ep->d_name);
	}

	closedir(dp);
}


/* -------------------------------------------------------------------------- */


void MidiMapConf::setDefault()
{
	brand  = "";
	device = "";

	for (int i=0; i<MAX_INIT_COMMANDS; i++) {
		init_channels[i] = -1;
		init_messages[i] = 0x00;
	}

	for (int i=0; i<4; i++) {
		mute_on_channel = -1;
		mute_on[i] = 0x00;
		mute_off_channel = -1;
		mute_off[i] = 0x00;

		solo_on_channel = -1;
		solo_on[i] = 0x00;
		solo_off_channel = -1;
		solo_off[i] = 0x00;

		waiting_channel = -1;
		waiting[i] = 0x00;
		playing_channel = -1;
		playing[i] = 0x00;
		stopping_channel = -1;
		stopping[i] = 0x00;
		stopped_channel = -1;
		stopped[i] = 0x00;
	}
}


/* -------------------------------------------------------------------------- */


int MidiMapConf::readMap(string file)
{
	if (file.empty()) {
		gLog("[MidiMapConf::readFromFile] midimap not specified, nothing to do\n");
		return 0;
	}

	gLog("[MidiMapConf::readFromFile] reading midimap file '%s'\n", file.c_str());

	string path = midimapsPath + file;
	fp = fopen(path.c_str(), "r");
	if (!fp) {
		gLog("[MidiMapConf::readFromFile] unreadable midimap file\n");
		return 0;
	}

	brand  = getValue("brand");
	device = getValue("device");

	gLog("[MidiMapConf::readFromFile] reading midimap for %s %s\n",
			brand.c_str(), device.c_str());

	/* parse init commands */

	gVector<string> ic;
	gSplit(getValue("init_commands"), ";", &ic);
	for (unsigned i=0; i<MAX_INIT_COMMANDS && i<ic.size; i++) {
		sscanf(ic.at(i).c_str(), "%d:%x", &init_channels[i], &init_messages[i]);
		gLog("[MidiMapConf::readFromFile] init command %d - channel %d - message 0x%X\n",
				i, init_channels[i], init_messages[i]);
	}

	/* parse messages */

	parse("mute_on",  &mute_on_channel,  mute_on,  &mute_on_notePos);
	parse("mute_off", &mute_off_channel, mute_off, &mute_off_notePos);
	parse("solo_on",  &solo_on_channel,  solo_on,  &solo_on_notePos);
	parse("solo_off", &solo_off_channel, solo_off, &solo_off_notePos);
	parse("waiting",  &waiting_channel,  waiting,  &waiting_notePos);
	parse("playing",  &playing_channel,  playing,  &playing_notePos);
	parse("stopping", &stopping_channel, stopping, &stopping_notePos);
	parse("stopped",  &stopped_channel,  stopped,  &stopped_notePos);

	parse2("mute_on",  &muteOnChan,   &muteOnMsg,   &muteOnOffset);
	parse2("mute_off", &muteOffChan,  &muteOffMsg,  &muteOffOffset);
	parse2("solo_on",  &soloOnChan,   &soloOnMsg,   &soloOnOffset);
	parse2("solo_off", &soloOffChan,  &soloOffMsg,  &soloOffOffset);
	parse2("waiting",  &waitingChan,  &waitingMsg,  &waitingOffset);
	parse2("playing",  &playingChan,  &playingMsg,  &playingOffset);
	parse2("stopping", &stoppingChan, &stoppingMsg, &stoppingOffset);
	parse2("stopped",  &stoppedChan,  &stoppedMsg,  &stoppedOffset);

	close();
	return 1;
}


/* -------------------------------------------------------------------------- */


void MidiMapConf::close()
{
	if (fp != NULL)
		fclose(fp);
	fp = NULL;
}


/* -------------------------------------------------------------------------- */


void MidiMapConf::parse(string key, int *channel, uint32_t *message, int *notePos)
{
	gLog("[MidiMapConf::parse] command %s - ", key.c_str());
	string value = getValue(key.c_str());

	/* grab channel part, i.e. [channel]:*/

	*channel = atoi(value.substr(0, value.find(':')).c_str());
	gLog("channel %x - ", *channel);

	/* grab MIDI part :[midi-message] and search for 'nn' note placeholder within.
	 * Note: when using 'string::npos' as the value for a len (or sublen)
	 * parameter in string's member functions, means "until the end of the
	 * string". */

	string midiParts = value.substr(value.find(':')+3, string::npos);

	if (midiParts.find("nn") != string::npos)
		*notePos = midiParts.find("nn")/2;
	else
		*notePos = -1;

	/* fill message[MAX_MIDI_BYTES] with each byte in the message. Skip 'nn' if
	 * found. */

	for (int i=0; i<MAX_MIDI_BYTES; i++) {
		if (i != *notePos) {
			string bit = midiParts.substr(i*2, 2);
			message[i] = strtoul(bit.c_str(), NULL, 16);
			gLog("0x%X ", message[i]);
		}
		else {
			gLog("[note] ");
		}
	}
	gLog("\n");
}


void MidiMapConf::parse2(string key, int *chan, uint32_t *msg, int *offset)
{
	gLog("[MidiMapConf::parse2] command %s - ", key.c_str());
	string value = getValue(key.c_str());

	/* grab channel part, i.e. [channel]:*/

	*chan = atoi(value.substr(0, value.find(':')).c_str());

	/* grab MIDI part :[midi-message] and search for 'nn' note placeholder within.
	 * Note: when using 'string::npos' as the value for a len (or sublen)
	 * parameter in string's member functions, means "until the end of the
	 * string". */

	string midiParts = value.substr(value.find(':')+3, string::npos);

	char strmsg[MAX_MIDI_NIBBLES];
	*offset = -1;

	for (unsigned i=0, p=24; i<MAX_MIDI_NIBBLES; i++, p-=4) {
		if (midiParts[i] == 'n') {
			strmsg[i] = '0';
			if (*offset == -1)
				*offset = p;
		}
		else
			strmsg[i] = midiParts[i];
	}

	*msg = strtoul(strmsg, NULL, 16);

	gLog("chan=%d value=%s msg=%#x, offset=%d\n", *chan, midiParts.c_str(), *msg, *offset);
}
