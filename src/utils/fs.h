/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
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


#ifndef G_UTILS_FS_H
#define G_UTILS_FS_H


#include <string>


namespace giada {
namespace u {
namespace fs 
{
bool fileExists(const std::string& s);
bool dirExists(const std::string& s);
bool isDir(const std::string& s);

/* isRootDir
Tells whether 's' is '/' on Unix or '[X]:\' on Windows. */

bool isRootDir(const std::string& s);

bool isProject(const std::string& s);
bool mkdir(const std::string& s);
std::string getCurrentPath();
std::string getHomePath();

/* getRealPath
Expands all symbolic links and resolves references to /./, /../ and extra / 
characters in the input path and returns the canonicalized absolute pathname. */

std::string getRealPath(const std::string& s);

/* basename
/path/to/file.txt -> file.txt */

std::string basename(const std::string& s);

/* dirname
/path/to/file.txt -> /path/to */

std::string dirname(const std::string& s);

/* getExt
/path/to/file.txt -> txt */

std::string getExt(const std::string& s);

/* stripExt
/path/to/file.txt -> /path/to/file */

std::string stripExt(const std::string& s);

std::string stripFileUrl(const std::string& s);

/* getUpDir
Returns the upper directory:
/path/to/my/directory -> /path/to/my/ */

std::string getUpDir(const std::string& s);
}}}  // giada::u::fs::


#endif
