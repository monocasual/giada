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


#include <filesystem>
#if defined(_WIN32)			// getcwd (unix) or __getcwd (win)
	#include <direct.h>
	#include <windows.h>
#else
	#include <unistd.h>
#endif
#include <cstdarg>
#include <sys/stat.h>       // stat (fs::dirExists)
#include <errno.h>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <cstring>
#include <climits>
#ifdef __APPLE__
	#include <libgen.h>     // basename unix
	#include <pwd.h>        // getpwuid
#endif
#include "core/const.h"
#include "utils/string.h"
#include "utils/log.h"
#include "utils/fs.h"


namespace giada {
namespace u     {
namespace fs 
{
bool fileExists(const std::string& s)
{
	return std::filesystem::exists(s);
}


/* -------------------------------------------------------------------------- */


bool isDir(const std::string& s)
{
	return std::filesystem::is_directory(s) && !isProject(s);
}


/* -------------------------------------------------------------------------- */


bool dirExists(const std::string& s)
{ 
	return std::filesystem::exists(s);
}


/* -------------------------------------------------------------------------- */


bool mkdir(const std::string& s)
{
	return dirExists(s) ? true : std::filesystem::create_directory(s);
}


/* -------------------------------------------------------------------------- */


std::string getRealPath(const std::string& s)
{
	return std::filesystem::canonical(s).string();
}


/* -------------------------------------------------------------------------- */


std::string basename(const std::string& s)
{
	return std::filesystem::path(s).filename().string();
}


/* -------------------------------------------------------------------------- */


std::string dirname(const std::string& s)
{
	return std::filesystem::path(s).parent_path().string();
}


/* -------------------------------------------------------------------------- */


std::string getCurrentPath()
{
	return std::filesystem::current_path().string();
}


/* -------------------------------------------------------------------------- */


std::string getExt(const std::string& s)
{
	return std::filesystem::path(s).extension().string();
}


/* -------------------------------------------------------------------------- */


std::string stripExt(const std::string& s)
{
	return std::filesystem::path(s).replace_extension("").string();
}


/* -------------------------------------------------------------------------- */


bool isProject(const std::string& s)
{
	/** TODO - checks too weak. */
	return getExt(s) == ".gprj";
}


/* -------------------------------------------------------------------------- */


std::string stripFileUrl(const std::string& s)
{
	std::string out = s;
	out = u::string::replace(out, "file://", "");
	out = u::string::replace(out, "%20", " ");
	return out;
}


/* -------------------------------------------------------------------------- */


std::string getHomePath()
{
#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)

	char buf[PATH_MAX];
	snprintf(buf, PATH_MAX, "%s/.giada", getenv("HOME"));

#elif defined(G_OS_WINDOWS)

	char buf[MAX_PATH];
	snprintf(buf, MAX_PATH, ".");

#elif defined(G_OS_MAC)

	char buf[PATH_MAX];
	struct passwd* pwd = getpwuid(getuid());
	if (pwd == nullptr) {
		log::print("[getHomePath] unable to fetch user infos\n");
		return "";
	}
	const char* home = pwd->pw_dir;
	snprintf(buf, PATH_MAX, "%s/Library/Application Support/Giada", home);

#endif

	return std::filesystem::path(buf).string();
}


/* -------------------------------------------------------------------------- */


bool isRootDir(const std::string& s)
{
	return std::filesystem::current_path().root_directory() == s;
}


/* -------------------------------------------------------------------------- */


std::string getUpDir(const std::string& s)
{
#ifdef G_OS_WINDOWS

	// If root, let the user browse the drives list by returning "". 
	if (isRootDir(s))
		return "";

#endif

	return std::filesystem::path(s).parent_path().string();
}
}}}  // giada::u::fs::