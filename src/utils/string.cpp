/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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


#include <iomanip>
#include <cstdarg>
#include <climits>
#include "core/const.h"
#include "string.h"


namespace giada {
namespace u     {
namespace string 
{
/* TODO - use std::to_string() */

std::string fToString(float f, int precision)
{
	std::stringstream out;
	out << std::fixed << std::setprecision(precision) << f;
	return out.str();
}


/* -------------------------------------------------------------------------- */


std::string trim(const std::string& s)
{
	std::size_t first = s.find_first_not_of(" \n\t");
	std::size_t last  = s.find_last_not_of(" \n\t");
	return s.substr(first, last-first+1);
}


/* -------------------------------------------------------------------------- */


std::string replace(std::string in, const std::string& search, const std::string& replace)
{
	std::size_t pos = 0;
	while ((pos = in.find(search, pos)) != std::string::npos) {
		in.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return in;
}


/* -------------------------------------------------------------------------- */


std::string format(const char* format, ...)
{
	va_list args;

	/* Compute the size of the new expanded std::string (i.e. with replacement taken
	into account). */

	va_start(args, format);
	std::size_t size = vsnprintf(nullptr, 0, format, args) + 1;
	va_end(args);
	
	/* Create a new temporary char array to hold the new expanded std::string. */

	std::unique_ptr<char[]> tmp(new char[size]);

	/* Fill the temporary std::string with the formatted data. */

	va_start(args, format);
	vsprintf(tmp.get(), format, args);
	va_end(args);

	return std::string(tmp.get(), tmp.get() + size - 1);
}


/* -------------------------------------------------------------------------- */


std::vector<std::string> split(std::string in, std::string sep)
{
	std::vector<std::string> out;
	std::string full  = in;
	std::string token = "";
	std::size_t curr = 0;
	std::size_t next = -1;
	do {
		curr  = next + 1;
		next  = full.find_first_of(sep, curr);
		token = full.substr(curr, next - curr);
		if (token != "")
			out.push_back(token);
	}
	while (next != std::string::npos);
	return out;
}

}}} // giada::u::string
