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


#ifndef G_CHANNEL_WAVE_READER_H
#define G_CHANNEL_WAVE_READER_H


#include <samplerate.h>
#include "core/types.h"


namespace giada {
namespace m
{
class Wave;
class WaveReader final
{
public:

    WaveReader();
    WaveReader(const WaveReader&);
    WaveReader(WaveReader&&);
    WaveReader& operator=(const WaveReader&);
    WaveReader& operator=(WaveReader&&);
    ~WaveReader();

    Frame fill(AudioBuffer& out, Frame start, Frame offset, float pitch) const;

	/* wave
	Wave object. Might be null if the channel has no sample. */

	const Wave* wave;

private:

	Frame fillResampled(AudioBuffer& out, Frame start, Frame offset, float pitch) const;
	Frame fillCopy     (AudioBuffer& out, Frame start, Frame offset) const;

	void allocateSrc();
	void moveSrc(SRC_STATE** o);

	/* srcState
	Struct from libsamplerate. */

	SRC_STATE* m_srcState;
};
}} // giada::m::


#endif
