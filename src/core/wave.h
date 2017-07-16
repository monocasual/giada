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

	SNDFILE* fileIn;
	SNDFILE* fileOut;
	SF_INFO  inHeader;
	SF_INFO  outHeader;

	float* data;
	int size;		    // Wave size in bytes (size in stereo: size / 2)
	bool logical;   // memory only (a take)
	bool edited;    // edited via editor
	
	std::string path; // E.g. /path/to/my/sample.wav
	std::string name; // Sample name (can be changed)

public:

	Wave();
	~Wave();
	Wave(const Wave& other);

	void setRate(int v);
	void setChannels(int v);
	void setFrames(int v);
	void setPath(const std::string& p);
	void setName(const std::string& p);
	void setData(float* data);
	void setSize(int s);
	void setEdited(bool e);

	std::string getBasename(bool ext=false) const;
	int getRate() const;
	int getChannels() const;
	int getFrames() const;	
	std::string getPath() const;	
	std::string getName() const;
	float* getData() const;
	int getSize() const;
	bool isLogical() const;
	bool isEdited() const;

	int  open(const char* f);
	int  readData();
	int	 writeData(const char* f);
	void clear();

	/* allocEmpty
	 * alloc an empty waveform. */

	int allocEmpty(unsigned size, unsigned samplerate);

	/* resample
	 * simple algorithm for one-shot resampling. */

	int resample(int quality, int newRate);
};

#endif
