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

#include "core/rendering/sampleRendering.h"
#include "core/resampler.h"
#include "core/wave.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include <cassert>

namespace giada::m::rendering
{
namespace
{
ReadResult readResampled_(const Wave& wave, mcl::AudioBuffer& dest, Frame start,
    Frame max, Frame offset, float pitch, const Resampler& resampler)
{
	Resampler::Result res = resampler.process(
	    /*input=*/wave.getBuffer()[0],
	    /*inputPos=*/start,
	    /*inputLen=*/max,
	    /*output=*/dest[offset],
	    /*outputLen=*/dest.countFrames() - offset,
	    /*pitch=*/pitch);

	return {
	    static_cast<int>(res.used),
	    static_cast<int>(res.generated)};
}

/* -------------------------------------------------------------------------- */

ReadResult readCopy_(const Wave& wave, mcl::AudioBuffer& dest, Frame start,
    Frame max, Frame offset)
{
	Frame used = dest.countFrames() - offset;
	if (used > max - start)
		used = max - start;

	dest.set(wave.getBuffer(), used, start, offset);

	return {used, used};
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ReadResult readWave(const Wave& wave, mcl::AudioBuffer& out, Frame start, Frame max,
    Frame offset, float pitch, const Resampler& resampler)
{
	assert(start >= 0);
	assert(max <= wave.getBuffer().countFrames());
	assert(offset < out.countFrames());

	if (pitch == 1.0f)
		return readCopy_(wave, out, start, max, offset);
	else
		return readResampled_(wave, out, start, max, offset, pitch, resampler);
}
} // namespace giada::m::rendering
