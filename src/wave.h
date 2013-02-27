/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * wave
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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


#ifndef WAVE_H
#define WAVE_H


#include <samplerate.h>
#include <sndfile.h>
#include <string>


class Wave {
public:

	Wave();
	~Wave();

	std::string pathfile; // full path + sample name
	std::string name;			// sample name (changeable)

	SNDFILE *fileIn;
	SNDFILE *fileOut;
	SF_INFO  inHeader;
	SF_INFO  outHeader;
	float   *data;
	int      size;			// wave size (size in stereo: size / 2)
	bool     isLogical; // memory only (a take)
	bool     isEdited;  // edited via editor

	inline int getRate() { return inHeader.samplerate; }

	int open(const char *f);
	int readData();
	int	writeData(const char *f);
	void clear();

	/* allocEmpty
	 * alloc an empty waveform. */

	int allocEmpty(unsigned size);

	int resample(int quality, int newRate);

};

#endif
