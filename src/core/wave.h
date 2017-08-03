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

	float* m_data;
	int m_size;		    // Wave size in bytes (size in stereo: size / 2)
	int m_channels;
	int m_rate;
	int m_bits;
	bool m_logical;   // memory only (a take)
	bool m_edited;    // edited via editor
	
	std::string m_path; // E.g. /path/to/my/sample.wav
	std::string m_name; // Sample name (can be changed)

public:

	Wave();
	Wave(float* data, int size, int channels, int rate, int bits, const std::string& path);
	~Wave();
	Wave(const Wave& other);

	void setRate(int v);
	void setChannels(int v);
	void setPath(const std::string& p);
	void setName(const std::string& p);
	void setData(float* data);
	void setSize(int s);
	void setLogical(bool l);
	void setEdited(bool e);

	std::string getBasename(bool ext=false) const;
	int getRate() const;
	int getChannels() const;
	std::string getPath() const;	
	std::string getName() const;
	int getBits() const;
	float* getData() const;
	int getSize_DEPR_() const;  // with no channels count (deprecated)
	int getSize() const;        // with channels count
	int getDuration() const;
	bool isLogical() const;
	bool isEdited() const;
	void clear();

};

#endif
