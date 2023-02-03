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

#include <filesystem>
#if defined(_WIN32) // getcwd (unix) or __getcwd (win)
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <string>
#include <sys/stat.h> // stat (fs::dirExists)
#ifdef __APPLE__
#include <libgen.h> // basename unix
#include <pwd.h>    // getpwuid
#endif
#include "core/const.h"
#include "utils/fs.h"
#include "utils/log.h"
#include "utils/string.h"

namespace stdfs = std::filesystem;

namespace giada::u::fs
{
bool fileExists(const std::string& s)
{
	return stdfs::exists(s);
}

/* -------------------------------------------------------------------------- */

bool isDir(const std::string& s)
{
	return stdfs::is_directory(s) && !isProject(s);
}

/* -------------------------------------------------------------------------- */

bool dirExists(const std::string& s)
{
	return stdfs::exists(s);
}

/* -------------------------------------------------------------------------- */

bool mkdir(const std::string& s)
{
	return dirExists(s) ? true : stdfs::create_directory(s);
}

/* -------------------------------------------------------------------------- */

std::string getRealPath(const std::string& s)
{
	return s.empty() || !stdfs::exists(s) ? "" : stdfs::canonical(s).string();
}

/* -------------------------------------------------------------------------- */

std::string basename(const std::string& s)
{
	return stdfs::path(s).filename().string();
}

/* -------------------------------------------------------------------------- */

std::string dirname(const std::string& s)
{
	return stdfs::path(s).parent_path().string();
}

/* -------------------------------------------------------------------------- */

std::string getCurrentPath()
{
	return stdfs::current_path().string();
}

/* -------------------------------------------------------------------------- */

std::string getExt(const std::string& s)
{
	return stdfs::path(s).extension().string();
}

/* -------------------------------------------------------------------------- */

std::string stripExt(const std::string& s)
{
	return stdfs::path(s).replace_extension("").string();
}

/* -------------------------------------------------------------------------- */

bool isProject(const std::string& s)
{
	/** TODO - checks too weak. */
	return getExt(s) == G_PROJECT_EXT;
}

/* -------------------------------------------------------------------------- */

std::string stripFileUrl(const std::string& s)
{
	std::string out = s;
	out             = u::string::replace(out, "file://", "");
	out             = u::string::replace(out, "%20", " ");
	return out;
}

/* -------------------------------------------------------------------------- */

std::string getHomePath()
{
#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)

	char buf[PATH_MAX];
	snprintf(buf, PATH_MAX, "%s/.giada", getenv("HOME"));
	return stdfs::path(buf).string();

#elif defined(G_OS_WINDOWS)

	return stdfs::current_path().string();

#elif defined(G_OS_MAC)

	char           buf[PATH_MAX];
	struct passwd* pwd = getpwuid(getuid());
	if (pwd == nullptr)
	{
		log::print("[getHomePath] unable to fetch user infos\n");
		return "";
	}
	const char* home = pwd->pw_dir;
	snprintf(buf, PATH_MAX, "%s/Library/Application Support/Giada", home);

	return stdfs::path(buf).string();

#endif
}

/* -------------------------------------------------------------------------- */

std::string getMidiMapsPath()
{
	auto out = stdfs::path(getHomePath()) / "midimaps";
	return out.string();
}

std::string getLangMapsPath()
{
	auto out = stdfs::path(getHomePath()) / "langmaps";
	return out.string();
}

/* -------------------------------------------------------------------------- */

bool createConfigFolder()
{
#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)

	const std::string confDirPath = getHomePath();

	if (u::fs::dirExists(confDirPath))
		return true;

	u::log::print("[fs::createConfigFolder] .giada folder not present. Updating...\n");

	if (u::fs::mkdir(confDirPath))
	{
		u::log::print("[fs::createConfigFolder] status: ok\n");
		return true;
	}
	else
	{
		u::log::print("[fs::createConfigFolder] status: error!\n");
		return false;
	}

#else // Windows: nothing to do

	return true;

#endif
}

/* -------------------------------------------------------------------------- */

std::string getConfigFilePath()
{
	return join(getHomePath(), G_CONF_FILENAME);
}

/* -------------------------------------------------------------------------- */

bool isRootDir(const std::string& s)
{
	return stdfs::current_path().root_directory() == s;
}

/* -------------------------------------------------------------------------- */

std::string getUpDir(const std::string& s)
{
#ifdef G_OS_WINDOWS

	// If root, let the user browse the drives list by returning "".
	if (isRootDir(s))
		return "";

#endif

	return stdfs::path(s).parent_path().string();
}

/* -------------------------------------------------------------------------- */

std::string join(const std::string& a, const std::string& b)
{
	auto out = stdfs::path(a) / stdfs::path(b);
	return out.string();
}
} // namespace giada::u::fs