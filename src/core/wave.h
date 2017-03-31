/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * wave
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


#ifndef G_WAVE_H
#define G_WAVE_H


#include <sndfile.h>
#include <string>


class Wave
{
private:

	SNDFILE *fileIn;
	SNDFILE *fileOut;
	SF_INFO  inHeader;
	SF_INFO  outHeader;

public:

	Wave();
	~Wave();
	Wave(const Wave &other);

	std::string pathfile; // full path + sample name
	std::string name;			// sample name (changeable)

	float *data;
	int    size;			  // wave size (size in stereo: size / 2)
	bool   isLogical;   // memory only (a take)
	bool   isEdited;    // edited via editor

	int  rate    ();
	int  channels();
	int  frames  ();
	void rate    (int v);
	void channels(int v);
	void frames  (int v);

	std::string basename(bool ext=false) const;
	std::string extension() const;

	void updateName(const char *n);
	int  open      (const char *f);
	int  readData  ();
	int	 writeData (const char *f);
	void clear     ();

	/* allocEmpty
	 * alloc an empty waveform. */

	int allocEmpty(unsigned size, unsigned samplerate);

	/* resample
	 * simple algorithm for one-shot resampling. */

	int resample(int quality, int newRate);
};

#endif
