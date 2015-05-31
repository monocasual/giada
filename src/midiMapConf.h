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


#ifndef __MIDIMAPCONF_H__
#define __MIDIMAPCONF_H__


#include <limits.h>
#include <stdint.h>
#include "dataStorage.h"
#include "utils.h"
#if defined(__APPLE__)
#include <pwd.h>
#endif


class MidiMapConf : public DataStorage
{
private:

	void close();
	void parse(std::string config, int *channel, uint32_t *message, int *notePos);

public:

	/* MAX_INIT_COMMANDS */

	static const int MAX_INIT_COMMANDS = 32;

	/* path of midimap files, different between OSes. */
	 
	std::string midimapsPath;

	/* Maps are the available .giadamap files. Each element of the vector
	represents a .giadamap filename. */

	gVector<std::string> maps;

	std::string brand;
	std::string device;

	/* init_commands. These messages are sent to the physical device as a
	wake up signal. */

	int      init_channels[MAX_INIT_COMMANDS];
	uint32_t init_messages[MAX_INIT_COMMANDS];

	/* events */

	int mute_on_channel;
	int mute_on_notePos;
	uint32_t mute_on[4];

	int mute_off_channel;
	int mute_off_notePos;
	uint32_t mute_off[4];

	int solo_on_channel;
	int solo_on_notePos;
	uint32_t solo_on[4];

	int solo_off_channel;
	int solo_off_notePos;
	uint32_t solo_off[4];

	int waiting_channel;
	int waiting_notePos;
	uint32_t waiting[4];

	int playing_channel;
	int playing_notePos;
	uint32_t playing[4];

	int stopping_channel;
	int stopping_notePos;
	uint32_t stopping[4];

	int stopped_channel;
	int stopped_notePos;
	uint32_t stopped[4];

	/* init
	Parse the midi maps folders and find the available maps. */

	void init();

	/* setDefault
	Set default values in case no maps are available/choosen. */

	void setDefault();

	/* readMap
	Read a midi map from file 'file'. */

	int readMap(std::string file);
};

#endif
