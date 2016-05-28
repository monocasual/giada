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
#include <cstdio>
#include <vector>
#include "log.h"


using std::string;
using std::vector;


bool gFileExists(const char *path);

bool gDirExists(const char *path);
bool gDirExists(const string &path);

bool gIsDir(const char *path);
bool gIsDir(const string &path);

bool gIsProject(const char *path);

bool gIsPatch(const char *path);

bool gMkdir(const char *path);
bool gMkdir(const string &path);

string gBasename(const char *path);
string gBasename(const string &s);

string gReplace(string in, const string& search, const string& replace);

string gDirname(const char *path);

string gTrim(const char *path);
string gTrim(const string &s);

string gGetCurrentPath();

string gGetHomePath();

string gStripFileUrl(const char *path);

string gGetExt(const char *path);

string gStripExt(const char *path);
string gStripExt(const string &s);

string gGetProjectName(const char *path); // TODO - useless!

string gGetSlash();

string gItoa(int i);

void gSplit(string in, string sep, vector<string> *v);

#endif
