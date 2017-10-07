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
#include "const.h"


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

public:

	Wave();
	Wave(float* data, int size, int channels, int rate, int bits, const std::string& path);
	~Wave();
	Wave(const Wave& other);

	void setRate(int v);
	void setChannels(int v);
	void setPath(const std::string& p);
	void setData(float* data, int size);
	void setLogical(bool l);
	void setEdited(bool e);

	std::string getBasename(bool ext=false) const;
	std::string getExtension() const;
	int getRate() const;
	int getChannels() const;
	std::string getPath() const;	
	int getBits() const;
	float* getData() const;
	int getSize() const;        // with channels count
	int getDuration() const;
	bool isLogical() const;
	bool isEdited() const;

	/* clear
	Resets Wave to init state. */

	void clear();

	/* free
	Frees memory, leaving everything else untouched. */

	void free();

	/* getFrame
	Given a frame 'f', returns a pointer to it. This is useful for digging inside
	a frame, i.e. parsing each channel. How to use it:

		float* frame = w->getFrame(40);
		for (int i=0; i<w->getChannels(); i++)
			... frame[i] ...
	*/

	float* getFrame(int f) const;

};

#endif
