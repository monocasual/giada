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


#ifndef G_MIDIMAPCONF_H
#define G_MIDIMAPCONF_H


#include <vector>
#include <string>


namespace giada {
namespace m {
namespace midimap
{
struct Message
{
    int         channel  = 0;
    std::string valueStr = "";
	int         offset   = -1;
	uint32_t    value    = 0;
};


struct MidiMap
{
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
};


/* -------------------------------------------------------------------------- */

/* midimap
The actual MidiMap struct with data. */

extern MidiMap midimap;

/* midimapsPath
Path of midimap files, different between OSes. */

extern std::string midimapsPath;

/* maps
Maps are the available .giadamap files. Each element of the std::vector 
represents a .giadamap filename. */

extern std::vector<std::string> maps;


/* -------------------------------------------------------------------------- */

/* init
Parses the midi maps folders and find the available maps. */

void init();

/* setDefault
Sets default values in case no maps are available/chosen. */

void setDefault();

/* isDefined
Checks whether a specific message has been defined within midi map file. */

bool isDefined(const Message& msg);

/* read
Reads a midi map from file 'file'. */

int read(const std::string& file);
}}}; // giada::m::midimap::

#endif
