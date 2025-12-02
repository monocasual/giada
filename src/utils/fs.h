/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

namespace giada::u::fs
{
bool        isProject(const std::string& s);
std::string getConfigDirPath();
std::string getMidiMapsPath();
std::string getLangMapsPath();

/* createConfigFolder
Creates the configuration folder that holds the .conf file. */

bool createConfigFolder();

/* getConfigFilePath
Returns the path to the .conf file. */

std::string getConfigFilePath();

/* join
Joins two string paths using the correct separator. */

std::string join(const std::string& a, const std::string& b);

/* isValidFileName
Returns false if the file name contains forbidden characters. */

bool isValidFileName(const std::string&);
} // namespace giada::u::fs

#endif
