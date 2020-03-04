/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#ifndef G_AUDIO_BUFFER_H
#define G_AUDIO_BUFFER_H


#include <array>
#include "core/types.h"
#include "core/const.h"


namespace giada {
namespace m
{
class AudioBuffer
{
public:

	using Pan = std::array<float, G_MAX_IO_CHANS>;

	/* AudioBuffer (1)
	Creates an empty (and invalid) audio buffer. */

	AudioBuffer();

	/* AudioBuffer (2)
	Creates an audio buffer and allocates memory for size * channels frames. */

	AudioBuffer(Frame size, int channels);

	AudioBuffer(const AudioBuffer& o);
	~AudioBuffer();

	/* operator []
	Given a frame 'offset', returns a pointer to it. This is useful for digging 
	inside a frame, i.e. parsing each channel. How to use it:

		for (int k=0; k<buffer->countFrames(), k++)
			for (int i=0; i<buffer->countChannels(); i++)
				... buffer[k][i] ...

	Also note that buffer[0] will give you a pointer to the whole internal data
	array. */

	float* operator [](int offset) const;

	Frame countFrames() const;
	int countSamples() const;
	int countChannels() const;
	bool isAllocd() const;

	/* getPeak
	Returns the highest value from any channel. */
	
	float getPeak() const;

	void alloc(Frame size, int channels);
	void free();

	/* copyData
	Copies 'frames' frames from the new 'data' into m_data, and fills m_data 
	starting from frame 'offset'. The new data MUST contain	the same number of 
	channels than m_channels. */

	void copyData(const float* data, Frame frames, int offset=0);

	void copyData(const AudioBuffer& b, float gain=1.0f);
	void addData(const AudioBuffer& b, float gain=1.0f, Pan pan={1.0f, 1.0f});

	/* setData
	Borrow 'data' as new m_data. Makes sure not to delete the data 'data' points
	to while using it. Set it back to nullptr when done. */

	void setData(float* data, Frame size, int channels);

	/* moveData
	Moves data held by 'b' into this buffer. Then 'b' becomes an empty buffer. */
	 
	void moveData(AudioBuffer& b);

	/* clear
	Clears the internal data by setting all bytes to 0.0f. Optional parameters
	'a' and 'b' set the range. */
	
	void clear(Frame a=0, Frame b=-1);

	void applyGain(float g);

private:

	float* m_data;
	Frame  m_size;
	int    m_channels;
};

}} // giada::m::

#endif