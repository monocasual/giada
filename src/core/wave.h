/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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


#include <string>
#include "core/audioBuffer.h"
#include "core/types.h"


namespace giada::m 
{
class Wave
{
public:

	Wave(ID id);
	Wave(const Wave& o);
	Wave(Wave&& o) = default;

	Wave& operator=(Wave&& o) = default;

	float* operator [](int offset) const;

	/* getFrame
	Works like operator []. See AudioBuffer for reference. */
	
	float* getFrame(int f) const;
	
	std::string getBasename(bool ext=false) const;
	std::string getExtension() const;
	int getRate() const;
	int getChannels() const;
	std::string getPath() const;	
	int getBits() const;
	int getSize() const;        // in frames
	int getDuration() const;
	bool isLogical() const;
	bool isEdited() const;

	/* setPath
	Sets new path 'p'. If 'id' != -1 inserts a numeric id next to the file 
	extension, e.g. : /path/to/sample-[id].wav */

	void setPath(const std::string& p, int id=-1);

	void setRate(int v);
	void setLogical(bool l);
	void setEdited(bool e);

	/* replaceData
	Replaces internal audio buffer with 'b' by moving it. */

	void replaceData(AudioBuffer&& b);
	
	/* copyData
	Copies 'frames' frames from the new 'data' into m_data, starting from frame 
	'offset'. */

	void copyData(const float* data, int frames, int channels, int offset=0);
	void copyData(const AudioBuffer& b);

	/* addData
	Merges audio data from buffer 'b' onto this one. */

	void addData(const AudioBuffer& b);

	void alloc(int size, int channels, int rate, int bits, const std::string& path);

	ID id;

private:

	AudioBuffer m_buffer;
	int         m_rate;
	int         m_bits;
	bool        m_logical;  // memory only (a take)
	bool        m_edited;   // edited via editor
	std::string m_path;     // E.g. /path/to/my/sample.wav
};
} // giada::m::


#endif
