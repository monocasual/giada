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


#if defined(__APPLE__)
#include <pwd.h>
#endif

#define	MAXINITCOMMANDS	32

using namespace std;

class MidiMapConf : public DataStorage
{
	private:
		int openFileForReading(std::string MapFile);
		void close();

		void parse(string Config, int* Channel, uint32_t* Message, int* NotePos);

	public:
		unsigned numBundles;
		string** bundles;


		std::string brand;
		std::string device;

		int init_channels[MAXINITCOMMANDS];
		uint32_t init_messages[MAXINITCOMMANDS];

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

		void setDefault();
		int readFromBundle(std::string BundleName);
		int readFromFile(std::string MapFile);
		void initBundles();
};

#endif
