/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * log
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "log.h"
#include <cstdio>
#include <string>

namespace giada::u::log
{
int init(int m)
{
	mode = m;
	stat = true;
	if (mode == LOG_MODE_FILE)
	{
		std::string fpath = fs::getHomePath() + G_SLASH + "giada.log";
		f                 = std::fopen(fpath.c_str(), "a");
		if (!f)
		{
			stat = false;
			return 0;
		}
	}
	return 1;
}

/* -------------------------------------------------------------------------- */

void close()
{
	if (mode == LOG_MODE_FILE)
		std::fclose(f);
}
} // namespace giada::u::log
