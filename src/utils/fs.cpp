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

#include "core/const.h"
#include <climits>
#include <cstdlib>
#include <filesystem>
#include <string>
#ifdef G_OS_MAC
#include <libgen.h> // basename unix
#include <pwd.h>    // getpwuid
#endif
#ifdef G_OS_WINDOWS
#include <shlobj.h> // SHGetKnownFolderPath
#endif
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

std::string getConfigDirPath()
{
#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)

	char buf[PATH_MAX];
	snprintf(buf, PATH_MAX, "%s/.giada", getenv("HOME"));
	return stdfs::path(buf).string();

#elif defined(G_OS_WINDOWS)

	wchar_t* appdataPathPtr = nullptr;
	auto     result         = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, NULL, &appdataPathPtr);
	if (result != S_OK)
	{
		log::print("[getConfigDirPath] unable to fetch AppData path\n");
		return "";
	}

	auto appDataPath = stdfs::path(std::wstring(appdataPathPtr)) / "Giada";

	// It's up to the SHGetKnownFolderPath caller to free appdataPathPtr.
	CoTaskMemFree(static_cast<void*>(appdataPathPtr));

	return appDataPath.string();

#elif defined(G_OS_MAC)

	char           buf[PATH_MAX];
	struct passwd* pwd = getpwuid(getuid());
	if (pwd == nullptr)
	{
		log::print("[getConfigDirPath] unable to fetch user infos\n");
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
	auto out = stdfs::path(getConfigDirPath()) / "midimaps";
	return out.string();
}

std::string getLangMapsPath()
{
	auto out = stdfs::path(getConfigDirPath()) / "langmaps";
	return out.string();
}

/* -------------------------------------------------------------------------- */

bool createConfigFolder()
{
	const std::string confDirPath = getConfigDirPath();

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
}

/* -------------------------------------------------------------------------- */

std::string getConfigFilePath()
{
	return join(getConfigDirPath(), G_CONF_FILENAME);
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

/* -------------------------------------------------------------------------- */

bool isValidFileName(const std::string& f)
{
#ifdef G_OS_WINDOWS
	const std::vector<char> forbidden = {'<', '>', ':', '"', '/', '\\', '|', '?', '*'};
#else
	const std::vector<char> forbidden = {'/', ':'}; // ':' not supported in macOS
#endif
	for (const char c : forbidden)
		if (u::string::contains(f, c))
			return false;
	return true;
}
} // namespace giada::u::fs