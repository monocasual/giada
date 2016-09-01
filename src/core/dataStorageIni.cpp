/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * dataStorageIni
 *
 * -----------------------------------------------------------------------------
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
 * -------------------------------------------------------------------------- */


#include <stdlib.h>
#include <limits.h>
#include "../utils/log.h"
#include "dataStorageIni.h"
#include "const.h"


std::string DataStorageIni::getValue(const char *in)
{
	/* on each call reset the pointe to the beginning of the file. Not so
	 * good but necessary if you want to pick up random values from the
	 * file. */

	fseek(fp, 0L, SEEK_SET);
	std::string out = "";

	while (!feof(fp)) {

		char buffer[MAX_LINE_LEN];
		if (fgets(buffer, MAX_LINE_LEN, fp) == NULL) {
			gu_log("[DataStorageIni::getValue] key '%s' not found\n", in);
			return "";
		}

		if (buffer[0] == '#')
			continue;

		unsigned len = strlen(in);
		if (strncmp(buffer, in, len) == 0) {

			for (unsigned i=len+1; i<MAX_LINE_LEN; i++) {
				if (buffer[i] == '\0' || buffer[i] == '\n' || buffer[i] == '\r')
					break;
				out += buffer[i];
			}

			break; // string found
		}
	}
	return out;
}
