/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
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


#ifndef G_UTILS_STRING_H
#define G_UTILS_STRING_H


#include <string>
#include <vector>


std::string gu_getRealPath(const std::string &path);

std::string gu_replace(std::string in, const std::string &search,
  const std::string &replace);

std::string gu_trim(const std::string &s);

// TODO - use std::to_string -> http://stackoverflow.com/questions/191757/how-to-concatenate-a-stdstring-and-an-int?rq=1
std::string gu_itoa(int i);

void gu_split(std::string in, std::string sep, std::vector<std::string> *v);


#endif
