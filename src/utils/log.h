/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * log
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_UTILS_LOG_H
#define G_UTILS_LOG_H

#include "core/const.h"
#include "utils/fs.h"
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fstream>
#include <string>
#include <type_traits>
#include <utility>

#ifdef G_DEBUG_MODE
#define G_DEBUG(f, ...) \
	std::cerr << __FILE__ << "::" << __func__ << "() - " << fmt::format(f __VA_OPT__(, ) __VA_ARGS__) << "\n";
#else
#define G_DEBUG(f, ...) \
	do                  \
	{                   \
	} while (0)
#endif

namespace giada::u::log
{
inline std::ofstream file;
inline int           mode;

/* init
Initializes logger. Mode defines where to write the output: LOG_MODE_STDOUT,
LOG_MODE_FILE and LOG_MODE_MUTE. */

bool init(int mode);

void close();

template <typename... Args>
static void print(const char* format, Args&&... args)
{
	if (mode == LOG_MODE_MUTE)
		return;
	if (mode == LOG_MODE_FILE && file.is_open())
		fmt::print(file, fmt::runtime(format), args...);
	else
		fmt::print(fmt::runtime(format), args...);
}
} // namespace giada::u::log

#endif
