/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/types.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include <string>

namespace giada::m
{
class Wave
{
public:
	Wave(ID id);
	Wave(const Wave& o);
	Wave(Wave&& o) = default;

	Wave& operator=(Wave&& o) = default;

	std::string getBasename(bool ext = false) const;
	std::string getExtension() const;
	int         getRate() const;
	std::string getPath() const;
	int         getBits() const;
	float       getDuration() const;
	bool        isLogical() const;
	bool        isEdited() const;

	/* getBuffer
	Returns a (non-)const reference to the underlying audio buffer. */

	mcl::AudioBuffer&       getBuffer();
	const mcl::AudioBuffer& getBuffer() const;

	/* setPath
	Sets new path 'p'. If 'id' != -1 inserts a numeric id next to the file
	extension, e.g. : /path/to/sample-[id].wav */

	void setPath(const std::string& p, int id = -1);

	void setRate(int v);
	void setLogical(bool l);
	void setEdited(bool e);

	/* replaceData
	Replaces internal audio buffer with 'b' by moving it. */

	void replaceData(mcl::AudioBuffer&& b);

	void alloc(Frame size, int channels, int rate, int bits, const std::string& path);

	ID id;

private:
	mcl::AudioBuffer m_buffer;
	int              m_rate;
	int              m_bits;
	bool             m_logical; // memory only (a take)
	bool             m_edited;  // edited via editor
	std::string      m_path;    // E.g. /path/to/my/sample.wav
};
} // namespace giada::m

#endif
