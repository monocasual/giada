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


#include <limits.h>
#include <sstream>
#include "string.h"


using std::string;


string gu_getRealPath(const string &path)
{
	string out = "";

#if defined(__linux__) || defined(__APPLE__)

	char *buf = realpath(path.c_str(), NULL);

#else // Windows

	char *buf = _fullpath(NULL, path.c_str(), PATH_MAX);

#endif

	if (buf) {
		out = buf;
		free(buf);
	}
	return out;
}


/* -------------------------------------------------------------------------- */


string gu_itoa(int i)
{
    // TODO - use std::to_string -> http://stackoverflow.com/questions/191757/how-to-concatenate-a-stdstring-and-an-int?rq=1
	std::stringstream out;
	out << i;
	return out.str();
}


/* -------------------------------------------------------------------------- */


string gu_trim(const string &s)
{
	std::size_t first = s.find_first_not_of(" \n\t");
	std::size_t last  = s.find_last_not_of(" \n\t");
	return s.substr(first, last-first+1);
}


/* -------------------------------------------------------------------------- */


string gu_replace(string in, const string &search, const string &replace)
{
	size_t pos = 0;
	while ((pos = in.find(search, pos)) != string::npos) {
		in.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return in;
}


/* -------------------------------------------------------------------------- */


void gu_split(string in, string sep, vector<string> *v)
{
	string full  = in;
	string token = "";
	size_t curr = 0;
	size_t next = -1;
	do {
	  curr  = next + 1;
	  next  = full.find_first_of(sep, curr);
		token = full.substr(curr, next - curr);
		if (token != "")
			v->push_back(token);
	}
	while (next != string::npos);
}
