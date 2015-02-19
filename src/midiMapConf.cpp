/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * midiMapConf
 *
 * ---------------------------------------------------------------------
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
 * ------------------------------------------------------------------ */


#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include "const.h"
#include "midiMapConf.h"
#include "utils.h"
#include "log.h"


void MidiMapConf::initBundles()
{
	gLog("[MidiConf] Init Bundles\n");
	numBundles = 1;

	bundles = new string*[numBundles];

	bundles[0] = new string[2];
	bundles[0][0] = "Novation Launchpad S";
	bundles[0][1] = "novation_launchpads.giadamap";
}


/* ------------------------------------------------------------------ */


int MidiMapConf::openFileForReading(std::string MapFile)
{
	fp = fopen(MapFile.c_str(), "r");
	if (fp == NULL) {
		gLog("[MidiConf::openFile] unable to open conf file for reading\n");
		return 0;
	}
	return 1;
}


/* ------------------------------------------------------------------ */


void MidiMapConf::setDefault()
{
	brand = "";
	device = "";

	for (int i = 0; i < 32; i++) {
		init_channels[i] = -1;
		init_messages[i] = 0x00;
	}

	for (int i = 0; i < 4; i++) {
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


/* ------------------------------------------------------------------ */


int MidiMapConf::readFromBundle(std::string BundleName)
{
	gLog("[MidiConf] Midi map bundle %s\n", BundleName.c_str());

	bool p_found = false;
	char path[PATH_MAX];

	for ( unsigned i = 0 ; i < numBundles ; i++) {
		if (!strcmp(BundleName.c_str(), bundles[0][0].c_str())) {
			snprintf(path, PATH_MAX, "%s/%s", "/home/blablack/src/giada/midimaps", bundles[0][1].c_str());
			p_found = true;
		}
	}

	if (p_found) {
		return readFromFile(path);
	}
	else {
		gLog("[MidiConf] Cannot find bundle %s\n", BundleName.c_str());
		return 0;
	}
}


/* ------------------------------------------------------------------ */


int MidiMapConf::readFromFile(std::string MapFile)
{
	gLog("[MidiConf] Reading midi map file %s\n", MapFile.c_str());

	if (!openFileForReading(MapFile)) {
		gLog("[MidiConf] unreadable .giadamap file\n");
		return 0;
	}

	brand = getValue("brand");
	device = getValue("device");

	gLog("[MidiConf] Reading map for %s - %s\n", brand.c_str(), device.c_str());

	istringstream StrStream(getValue("init_commands"));
	int i=0;
	while (StrStream)
	{
		string Token;
		getline(StrStream, Token, ';');

		if (Token.length() > 0)
		{
			init_channels[i] = atoi(Token.substr(0, Token.find(':')).c_str());
			init_messages[i] = strtoul(Token.substr(Token.find(':')+3, 10).c_str(), NULL, 16);

			gLog("[MidiConf] Init Command %x - Channel %x - Message 0x%X\n", i+1, init_channels[i], init_messages[i]);
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


/* ------------------------------------------------------------------ */


void MidiMapConf::close()
{
	if (fp != NULL)
		fclose(fp);
	fp = NULL;
}


/* ------------------------------------------------------------------ */


void MidiMapConf::parse(string Config, int* Channel, uint32_t* Message, int* NotePos)
{
	//gLog("[MidiConf] Init Command %x - Channel %x - Message 0x%X\n", i+1, init_channels[i], init_messages[i]);
	gLog("[MidiConf] Command %s - ", Config.c_str());
	string p_config = getValue(Config.c_str());

	*Channel = atoi(p_config.substr(0, p_config.find(':')).c_str());
	gLog("Channel %x - ", *Channel);

	string p_midiParts = p_config.substr(p_config.find(':')+3, 10);
	if (p_midiParts.find("nn") != std::string::npos)
		*NotePos = p_midiParts.find("nn")/2;
	else
		*NotePos = -1;

	for (int i=0 ; i<4 ; i++) {
		if (i!=*NotePos) {
			string p_bit = p_midiParts.substr(i*2, 2);
			Message[i] = strtoul(p_bit.c_str(), NULL, 16);
			gLog("0x%X ", Message[i]);
		}
		else {
			gLog("Note ");
		}
	}
	gLog("\n");
}

