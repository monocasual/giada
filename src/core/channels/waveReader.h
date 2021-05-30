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

#ifndef G_CHANNEL_WAVE_READER_H
#define G_CHANNEL_WAVE_READER_H

#include "core/types.h"
#include <samplerate.h>

namespace giada::m
{
class Wave;
class AudioBuffer;
class WaveReader final
{
public:
	/* Result
	A Result object is returned by the fill() function below, containing the 
	number of frames used and generated from a buffer filling operation. The
	two values are different only when pitch is != 1.0, where a chunk of audio
	in input (used) might result in a longer or shorter portion of audio in 
	output (generated). */

	struct Result
	{
		Frame used, generated;
	};

	WaveReader();
	WaveReader(const WaveReader& o);
	WaveReader(WaveReader&&);
	WaveReader& operator=(const WaveReader&);
	WaveReader& operator=(WaveReader&&);
	~WaveReader();

	/* fill
	Fills audio buffer 'out' with data coming from Wave, copying it from 'start'
	frame up to 'max'. The buffer is filled starting at 'offset'. */

	Result fill(AudioBuffer& out, Frame start, Frame max, Frame offset, float pitch) const;

	/* wave
	Wave object. Might be null if the channel has no sample. */

	Wave* wave;

private:
	Result fillResampled(AudioBuffer& out, Frame start, Frame max, Frame offset, float pitch) const;
	Result fillCopy(AudioBuffer& out, Frame start, Frame max, Frame offset) const;

	void allocateSrc();
	void moveSrc(SRC_STATE** o);

	/* srcState
	Struct from libsamplerate. */

	SRC_STATE* m_srcState;
};
} // namespace giada::m

#endif
