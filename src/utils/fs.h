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


#ifndef __UTILS_FS_H__
#define __UTILS_FS_H__


#include <string>


bool gu_fileExists(const std::string &path);
bool gu_dirExists(const std::string &path);
bool gu_isDir(const std::string &path);
bool gu_isProject(const std::string &path);
bool gu_mkdir(const std::string &path);
std::string gu_getCurrentPath();
std::string gu_getHomePath();
std::string gu_basename(const std::string &s);
std::string gu_dirname(const std::string &s);
std::string gu_getExt(const std::string &s);
std::string gu_stripExt(const std::string &s);
std::string gu_stripFileUrl(const std::string &s);


#endif
