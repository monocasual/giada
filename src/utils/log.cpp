/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * log
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#include <cstdio>
#include <cstdarg>
#include <string>
#include "../utils/utils.h"
#include "../core/const.h"
#include "log.h"


static FILE *f;
static int   mode;
static bool  stat;


int gLog_init(int m) {
	mode = m;
	stat = true;
	if (mode == LOG_MODE_FILE) {
		std::string fpath = gGetHomePath() + "/giada.log";
		f = fopen(fpath.c_str(), "a");
		if (!f) {
			stat = false;
			return 0;
		}
	}
	return 1;
}


/* ------------------------------------------------------------------ */


void gLog_close() {
	if (mode == LOG_MODE_FILE)
		fclose(f);
}


/* ------------------------------------------------------------------ */


void gLog(const char *format, ...) {
	if (mode == LOG_MODE_MUTE)
		return;
  va_list args;
  va_start(args, format);
  if (mode == LOG_MODE_FILE && stat == true) {
		vfprintf(f, format, args);
#ifdef _WIN32
		fflush(f);
#endif
	}
  else
		vprintf(format, args);
  va_end(args);
}
