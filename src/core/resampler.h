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

#ifndef G_RESAMPLER_H
#define G_RESAMPLER_H

#include <cstddef>
#include <samplerate.h>

namespace giada::m
{
class Resampler final
{
public:
	enum class Quality
	{
		SINC_BEST       = 0,
		SINC_MEDIUM     = 1,
		SINC_FASTEST    = 2,
		ZERO_ORDER_HOLD = 3,
		LINEAR          = 4
	};

	/* Result
	A Result object is returned by the process() function below, containing the 
	number of frames used from input and generated to output. */

	struct Result
	{
		long used, generated;
	};

	Resampler(); // Invalid
	Resampler(Quality quality, int channels);
	Resampler(const Resampler& o);
	Resampler(Resampler&&);
	Resampler& operator=(const Resampler&);
	Resampler& operator=(Resampler&&);
	~Resampler();

	/* process
	Resamples a certain amount of frames from 'input' starting at 'inputPos' and
	puts the result into 'output'. */

	Result process(float* input, long inputPos, long inputLength, float* output,
	    long outputLength, float ratio);

	/* last
	Call this when you are about to process the last chunk of data. */

	void last();

private:
	static long callback(void* self, float** audio);
	long        callback(float** audio);

	void alloc(Quality quality, int channels);

	/* CHUNK_LEN
	How many chunks of data to read from input in the callback. */

	static constexpr int CHUNK_LEN = 256;

	SRC_STATE* m_state;
	Quality    m_quality;
	float*     m_input;       // Pointer to input data
	long       m_inputPos;    // Where to read from input
	long       m_inputLength; // Total number of frames in input data
	int        m_channels;    // Number of channels
	long       m_usedFrames;  // How many frames have been read from input with a process() call
};
} // namespace giada::m

#endif