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
#include "const.h"
#include "midiMapConf.h"
#include "utils.h"
#include "log.h"



void MidiMapConf::init()
{
	// TODO - scan dir of midi maps and load the filenames into <>maps. 
}


/* -------------------------------------------------------------------------- */


void MidiMapConf::setDefault()
{
	/* TODO - this is just a test, remove it asap */
	maps.add("novation_launchpads.giadamap");

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


int MidiMapConf::readMap(std::string file)
{
	gLog("[MidiMapConf::readFromFile] Reading midi map file '%s'\n", file.c_str());

	std::string path = "/home/mcl/.giada/midimaps/" + file;

	fp = fopen(path.c_str(), "r");
	if (fp == NULL) {
		gLog("[MidiMapConf::readFromFile] unreadable .giadamap file\n");
		return 0;
	}

	brand = getValue("brand");
	device = getValue("device");

	gLog("[MidiMapConf::readFromFile] Reading map for %s - %s\n", brand.c_str(), device.c_str());

	std::istringstream StrStream(getValue("init_commands"));
	int i=0;
	while (StrStream)
	{
		std::string Token;
		getline(StrStream, Token, ';');

		if (Token.length() > 0)
		{
			init_channels[i] = atoi(Token.substr(0, Token.find(':')).c_str());
			init_messages[i] = strtoul(Token.substr(Token.find(':')+3, 10).c_str(), NULL, 16);

			gLog("[MidiMapConf::readFromFile] Init Command %x - Channel %x - Message 0x%X\n", i+1, init_channels[i], init_messages[i]);
		}

		i++;
	}

	parse("mute_on",         &mute_on_channel,         mute_on,         &mute_on_notePos);
	parse("mute_off",        &mute_off_channel,        mute_off,        &mute_off_notePos);
	parse("solo_on",         &solo_on_channel,         solo_on,         &solo_on_notePos);
	parse("solo_off",        &solo_off_channel,        solo_off,        &solo_off_notePos);
	parse("waiting",         &waiting_channel,         waiting,         &waiting_notePos);
	parse("playing",         &playing_channel,         playing,         &playing_notePos);
	parse("stopping",        &stopping_channel,        stopping,        &stopping_notePos);
	parse("stopped",         &stopped_channel,         stopped,         &stopped_notePos);

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


void MidiMapConf::parse(std::string config, int *channel, uint32_t *message, int *notePos)
{
	//gLog("[MidiMapConf] Init Command %x - Channel %x - Message 0x%X\n", i+1, init_channels[i], init_messages[i]);
	gLog("[MidiMapConf::parse] Command %s - ", config.c_str());
	std::string p_config = getValue(config.c_str());

	*channel = atoi(p_config.substr(0, p_config.find(':')).c_str());
	gLog("channel %x - ", *channel);

	std::string p_midiParts = p_config.substr(p_config.find(':')+3, 10);
	if (p_midiParts.find("nn") != std::string::npos)
		*notePos = p_midiParts.find("nn")/2;
	else
		*notePos = -1;

	for (int i=0; i<4; i++) {
		if (i != *notePos) {
			std::string p_bit = p_midiParts.substr(i*2, 2);
			message[i] = strtoul(p_bit.c_str(), NULL, 16);
			gLog("0x%X ", message[i]);
		}
		else {
			gLog("Note ");
		}
	}
	gLog("\n");
}
