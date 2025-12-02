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

#include "src/deps/mcl-utils/src/fs.hpp"
#include "src/const.h"
#include <climits>
#include <cstdlib>
#include <filesystem>
#include <string>
#if G_OS_MAC
#include <libgen.h> // basename unix
#include <pwd.h>    // getpwuid
#include <unistd.h> // getuid
#endif
#if G_OS_WINDOWS
#include <shlobj.h> // SHGetKnownFolderPath
#endif
#include "src/deps/mcl-utils/src/string.hpp"
#include "src/utils/fs.h"
#include "src/utils/log.h"

namespace stdfs = std::filesystem;
namespace utils = mcl::utils;

namespace giada::u::fs
{
bool isProject(const std::string& s)
{
	/** TODO - checks too weak. */
	return utils::fs::getExt(s) == G_PROJECT_EXT;
}

/* -------------------------------------------------------------------------- */

std::string getConfigDirPath()
{
	return utils::fs::join(utils::fs::getConfigDirPath(), "Giada");
}

/* -------------------------------------------------------------------------- */

std::string getMidiMapsPath()
{
	return utils::fs::join(getConfigDirPath(), "midimaps");
}

std::string getLangMapsPath()
{
	return utils::fs::join(getConfigDirPath(), "langmaps");
}

/* -------------------------------------------------------------------------- */

bool createConfigFolder()
{
	const std::string confDirPath = getConfigDirPath();

	if (utils::fs::dirExists(confDirPath))
		return true;

	u::log::print("[fs::createConfigFolder] .giada folder not present. Updating...\n");

	if (utils::fs::mkdir(confDirPath))
	{
		u::log::print("[fs::createConfigFolder] status: ok\n");
		return true;
	}
	else
	{
		u::log::print("[fs::createConfigFolder] status: error!\n");
		return false;
	}
}

/* -------------------------------------------------------------------------- */

std::string getConfigFilePath()
{
	return utils::fs::join(getConfigDirPath(), G_CONF_FILENAME);
}
} // namespace giada::u::fs
