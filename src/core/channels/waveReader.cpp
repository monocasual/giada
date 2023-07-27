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

#include "waveReader.h"
#include "core/const.h"
#include "core/model/model.h"
#include "core/wave.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "utils/log.h"
#include <algorithm>
#include <cassert>
#include <memory>

namespace giada::m
{
WaveReader::Result WaveReader::fill(const Wave& wave, mcl::AudioBuffer& out, Frame start, Frame max,
    Frame offset, float pitch, const Resampler& resampler) const
{
	assert(start >= 0);
	assert(max <= wave.getBuffer().countFrames());
	assert(offset < out.countFrames());

	if (pitch == 1.0f)
		return fillCopy(wave, out, start, max, offset);
	else
		return fillResampled(wave, out, start, max, offset, pitch, resampler);
}

/* -------------------------------------------------------------------------- */

WaveReader::Result WaveReader::fillResampled(const Wave& wave, mcl::AudioBuffer& dest, Frame start,
    Frame max, Frame offset, float pitch, const Resampler& resampler) const
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

WaveReader::Result WaveReader::fillCopy(const Wave& wave, mcl::AudioBuffer& dest, Frame start,
    Frame max, Frame offset) const
{
	Frame used = dest.countFrames() - offset;
	if (used > max - start)
		used = max - start;

	dest.set(wave.getBuffer(), used, start, offset);

	return {used, used};
}
} // namespace giada::m
