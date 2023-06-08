/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_UTILS_STRING_H
#define G_UTILS_STRING_H

#include "core/types.h"
#include "deps/rtaudio/RtAudio.h"
#include <sstream>
#include <string>
#include <vector>

namespace giada::u::string
{
std::string replace(std::string in, const std::string& search,
    const std::string& replace);

std::string trim(const std::string& s);

std::vector<std::string> split(std::string in, std::string sep);

/* contains
Returns true if the string in input contains the specified character. */

bool contains(const std::string&, char);

std::string format(const char* format, ...);

std::string toString(Thread);
std::string toString(RtAudio::Api);

/* toFloat, toInt
Convert a string to numbers. Like std::stof, std::stoi, just safer. */

float toFloat(const std::string&);
int   toInt(const std::string&);
} // namespace giada::u::string

#endif
