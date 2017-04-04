/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include <limits.h>
#include <stdint.h>
#include <vector>
#include <string>
#include "dataStorageIni.h"
#include "dataStorageJson.h"
#include "../utils/fs.h"
#if defined(__APPLE__)
#include <pwd.h>
#endif


namespace giada {
namespace midimap
{
struct message_t
{
  int         channel;
  std::string valueStr;
	int         offset;
	uint32_t    value;
};

extern std::string brand;
extern std::string device;
extern std::vector<message_t> initCommands;
extern message_t muteOn;
extern message_t muteOff;
extern message_t soloOn;
extern message_t soloOff;
extern message_t waiting;
extern message_t playing;
extern message_t stopping;
extern message_t stopped;

/* midimapsPath
 * path of midimap files, different between OSes. */

extern std::string midimapsPath;

/* maps
 * Maps are the available .giadamap files. Each element of the std::vector
 * represents a .giadamap filename. */

extern std::vector<std::string> maps;

/* init
Parse the midi maps folders and find the available maps. */

void init();

/* setDefault
Set default values in case no maps are available/choosen. */

void setDefault();

/* read
Read a midi map from file 'file'. */

int read(const std::string &file);

}}; // giada::midimap::

#endif
