/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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

#ifndef G_TYPES_H
#define G_TYPES_H

#include <string>

namespace giada
{
using ID    = int;
using Frame = int;

enum class PluginSortMethod : int
{
	NAME = 0,
	CATEGORY,
	MANUFACTURER,
	FORMAT
};

enum class PluginSortDir : int
{
	ASC = 0,
	DESC
};

struct PluginSortMode
{
	PluginSortMethod method;
	PluginSortDir    dir;
};

struct PluginInfo
{
	std::string juceId;
	std::string name;
	std::string category;
	std::string manufacturerName;
	std::string format;
	bool        isInstrument;
	bool        exists;
	bool        isKnown;
};
} // namespace giada

#endif
