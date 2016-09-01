/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#ifndef UTILS_H
#define UTILS_H


#include <string>
#include <vector>


using std::string;
using std::vector;


bool gu_fileExists(const string &path);

bool gu_dirExists(const string &path);

bool gu_isDir(const string &path);

bool gu_isProject(const string &path);

bool gu_mkdir(const string &path);

string gu_getCurrentPath();

string gu_getHomePath();

string gu_basename(const string &s);

string gu_dirname(const string &s);

string gu_getExt(const string &s);

string gu_stripExt(const string &s);

string gu_stripFileUrl(const string &s);


#endif
