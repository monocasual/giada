/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#if defined(_WIN32)			// getcwd (unix) or __getcwd (win)
	#include <direct.h>
	#include <windows.h>
#else
	#include <unistd.h>
#endif

#include <cstdarg>
#include <sys/stat.h>   // stat (gu_dirExists)
#include <errno.h>
#include <cstdlib>
#ifdef __APPLE__  // our Clang still doesn't know about cstdint (c++11 stuff)
	#include <stdint.h>
#else
	#include <cstdint>
#endif
#include <string>
#include <cstring>
#include <climits>
#ifdef __APPLE__
	#include <libgen.h>     // basename unix
	#include <pwd.h>        // getpwuid
#endif
#include "../core/const.h"
#include "string.h"
#include "log.h"
#include "fs.h"


using std::string;
using std::vector;
using namespace giada;


bool gu_fileExists(const string &filename)
{
	FILE *fh = fopen(filename.c_str(), "rb");
	if (!fh) {
		return 0;
	}
	else {
		fclose(fh);
		return 1;
	}
}


/* -------------------------------------------------------------------------- */


bool gu_isDir(const string &path)
{
	bool ret;

#if defined(__linux__) || defined(__FreeBSD__)

	struct stat s1;
	stat(path.c_str(), &s1);
	ret = S_ISDIR(s1.st_mode);

#elif defined(__APPLE__)

	if (strcmp(path.c_str(), "") == 0)
		ret = false;
	else {
		struct stat s1;
		stat(path.c_str(), &s1);
		ret = S_ISDIR(s1.st_mode);

		/* check if ret is a bundle, a special OS X folder which must be
		 * shown as a regular file (VST).
		 * FIXME - consider native functions CFBundle... */

		if (ret) {
			if (gu_fileExists(path + "/Contents/Info.plist"))
				ret = false;
		}
	}

#elif defined(__WIN32)

  unsigned dwAttrib = GetFileAttributes(path.c_str());
  ret = (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif

	return ret & !gu_isProject(path);
}


/* -------------------------------------------------------------------------- */


bool gu_dirExists(const string &path)
{
	struct stat st;
	if (stat(path.c_str(), &st) != 0 && errno == ENOENT)
		return false;
	return true;
}


/* -------------------------------------------------------------------------- */


bool gu_mkdir(const string &path)
{
#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
	if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
#else
	if (_mkdir(path.c_str()) == 0)
#endif
		return true;
	return false;
}


/* -------------------------------------------------------------------------- */


string gu_basename(const string& s)
{
	string out = s;
	out.erase(0, out.find_last_of(G_SLASH_STR) + 1);
	return out;
}


/* -------------------------------------------------------------------------- */


string gu_dirname(const string& path)
{
	if (path.empty())
		return "";
	string out = path;
	out.erase(out.find_last_of(G_SLASH_STR));
	return out;
}


/* -------------------------------------------------------------------------- */


string gu_getCurrentPath()
{
 char buf[PATH_MAX];
#if defined(__WIN32)
	if (_getcwd(buf, PATH_MAX) != nullptr)
#else
	if (getcwd(buf, PATH_MAX) != nullptr)
#endif
		return buf;
	else
		return "";
}


/* -------------------------------------------------------------------------- */


string gu_getExt(const string& file)
{
	// TODO - use std functions
	int len = strlen(file.c_str());
	int pos = len;
	while (pos>0) {
		if (file[pos] == '.')
			break;
		pos--;
	}
	if (pos==0)
		return "";
	string out = file;
	return out.substr(pos+1, len);
}


/* -------------------------------------------------------------------------- */


string gu_stripExt(const string& s)
{
	return s.substr(0, s.find_last_of("."));
}


/* -------------------------------------------------------------------------- */


bool gu_isProject(const string& path)
{
	/** FIXME - checks too weak */

	if (gu_getExt(path.c_str()) == "gprj" && gu_dirExists(path))
		return 1;
	return 0;
}


/* -------------------------------------------------------------------------- */


string gu_stripFileUrl(const string& f)
{
	string out = f;
	out = u::string::replace(out, "file://", "");
	out = u::string::replace(out, "%20", " ");
	return out;
}


/* -------------------------------------------------------------------------- */


string gu_getHomePath()
{
	char path[PATH_MAX];

#if   defined(__linux__) || defined(__FreeBSD__)

	snprintf(path, PATH_MAX, "%s/.giada", getenv("HOME"));

#elif defined(_WIN32)

	snprintf(path, PATH_MAX, ".");

#elif defined(__APPLE__)

	struct passwd* p = getpwuid(getuid());
	if (p == nullptr) {
		gu_log("[gu_getHomePath] unable to fetch user infos\n");
		return "";
	}
	else {
		const char* home = p->pw_dir;
		snprintf(path, PATH_MAX, "%s/Library/Application Support/Giada", home);
	}

#endif

	return string(path);
}


/* -------------------------------------------------------------------------- */


bool gu_isRootDir(const std::string& s)
{
	if (s == "")
		return false;

#ifdef G_OS_WINDOWS

	return s.length() <= 3 && s[1] == ':';  /* X: or X:\ */

#else

	return s == G_SLASH_STR;

#endif
}


/* -------------------------------------------------------------------------- */


std::string gu_getUpDir(const std::string& s)
{
#ifdef G_OS_WINDOWS

	/* If root, let the user browse the drives list by returning "". */
	if (gu_isRootDir(s))
		return "";

#endif

	return s.substr(0, s.find_last_of(G_SLASH_STR)) + G_SLASH_STR;
}
