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


using std::string;


class MidiMapConf : public DataStorage
{
private:

	void close();
	void parse(string key, int *channel, uint32_t *message, int *notePos);
	void parse2(string key, int *chan, uint32_t *msg, int *offset);

public:

	static const int MAX_INIT_COMMANDS = 32;
	static const int MAX_MIDI_BYTES = 4;
	static const int MAX_MIDI_NIBBLES = 8;

	/* midimapsPath
	 * path of midimap files, different between OSes. */

	string midimapsPath;

	/* maps
	 * Maps are the available .giadamap files. Each element of the vector
	 * represents a .giadamap filename. */

	gVector<string> maps;

	string brand;
	string device;

	/* init_*
	 * init_commands. These messages are sent to the physical device as a wake up
	 * signal. */

	int      init_channels[MAX_INIT_COMMANDS];
	uint32_t init_messages[MAX_INIT_COMMANDS];

	/* events
	 * [event]Channel: the MIDI output channel to send the event to
	 * [event]notePos: the byte where the note is stored ('nn' placeholder)
	 * [event][MIDI_BYTES]: the actual MIDI message split in 4 single bytes */

	int mute_on_channel;
	int mute_on_notePos;
	uint32_t mute_on[MAX_MIDI_BYTES];
	int      muteOnChan;
	int      muteOnOffset;
	uint32_t muteOnMsg;

	int mute_off_channel;
	int mute_off_notePos;
	uint32_t mute_off[MAX_MIDI_BYTES];
	int      muteOffChan;
	int      muteOffOffset;
	uint32_t muteOffMsg;

	int solo_on_channel;
	int solo_on_notePos;
	uint32_t solo_on[MAX_MIDI_BYTES];
	int      soloOnChan;
	int      soloOnOffset;
	uint32_t soloOnMsg;

	int solo_off_channel;
	int solo_off_notePos;
	uint32_t solo_off[MAX_MIDI_BYTES];
	int      soloOffChan;
	int      soloOffOffset;
	uint32_t soloOffMsg;

	int waiting_channel;
	int waiting_notePos;
	uint32_t waiting[MAX_MIDI_BYTES];
	int      waitingChan;
	int      waitingOffset;
	uint32_t waitingMsg;

	int playing_channel;
	int playing_notePos;
	uint32_t playing[MAX_MIDI_BYTES];
	int      playingChan;
	int      playingOffset;
	uint32_t playingMsg;

	int stopping_channel;
	int stopping_notePos;
	uint32_t stopping[MAX_MIDI_BYTES];
	int      stoppingChan;
	int      stoppingOffset;
	uint32_t stoppingMsg;

	int stopped_channel;
	int stopped_notePos;
	uint32_t stopped[MAX_MIDI_BYTES];
	int      stoppedChan;
	int      stoppedOffset;
	uint32_t stoppedMsg;

	/* init
	Parse the midi maps folders and find the available maps. */

	void init();

	/* setDefault
	Set default values in case no maps are available/choosen. */

	void setDefault();

	/* readMap
	Read a midi map from file 'file'. */

	int readMap(string file);
};

#endif
