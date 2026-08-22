/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_STRETCHER_H
#define G_STRETCHER_H

#include <rubberband/RubberBandStretcher.h>

namespace mcl
{
class AudioBuffer;
}

namespace giada::m
{
/* Stretcher
A small wrapper around Rubber Band's time-stretching engine. */

class Stretcher final
{
public:
	/* Result
	Reports the outcome of a single call to process(): how many input frames
	were consumed and how many output frames were generated. 'finished' is
	different: it's true only once the **entire** stretch operation has been
	fully drained and the stretcher reset(), which may take several calls
	past the point where input runs out. Use 'finished' to determine whether
	stretching has truly ended. */

	struct Result
	{
		std::size_t used      = 0;
		std::size_t generated = 0;
		bool        finished  = false;
	};

	Stretcher(int sampleRate);

	/* last
	Notifies Rubber Band that this is the last chunk of audio. */

	void last();

	/* process
	Stretches/pitch-shifts a stereo buffer's [inputStart, inputEnd) range into
	'output', starting at 'outputStart', using the given time/pitch ratios.
	Since Rubber Band doesn't guarantee a fixed input/output ratio per call,
	this may take several calls to fully drain real audio after the input
	range ends: see Result::finished. */

	Result process(
	    const mcl::AudioBuffer& input,
	    std::size_t             inputStart,
	    std::size_t             inputEnd,
	    mcl::AudioBuffer&       output,
	    std::size_t             outputStart,
	    double                  timeRatio,
	    double                  pitchRatio);

private:
	RubberBand::RubberBandStretcher m_stretcher;
};
} // namespace giada::m

#endif