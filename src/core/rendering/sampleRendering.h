/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_RENDERING_SAMPLE_RENDERING_H
#define G_RENDERING_SAMPLE_RENDERING_H

#include "core/types.h"

namespace mcl
{
class AudioBuffer;
}

namespace giada::m
{
class Wave;
class Resampler;
} // namespace giada::m

namespace giada::m::rendering
{
/* ReadResult
A ReadResult object is returned by the readWave() function below, containing the 
number of frames used and generated from a buffer filling operation. The two 
values are different only when pitch is != 1.0, where a chunk of audio in input 
(used) might result in a longer or shorter portion of audio in output (generated). */

struct ReadResult
{
	Frame used, generated;
};

ReadResult readWave(const Wave&, mcl::AudioBuffer&, Frame start, Frame max, Frame offset, float pitch, const Resampler&);
} // namespace giada::m::rendering

#endif
