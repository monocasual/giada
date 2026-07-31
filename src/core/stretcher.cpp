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

#include "src/core/stretcher.h"
#include "src/core/const.h"
#include <cassert>

namespace giada::m
{
Stretcher::Stretcher(int sampleRate)
: m_stretcher(RubberBand::RubberBandStretcher(
      sampleRate, G_MAX_IO_CHANS, RubberBand::RubberBandStretcher::OptionProcessRealTime))
{
}

/* -------------------------------------------------------------------------- */

void Stretcher::setTempo(double ratio) { m_stretcher.setTimeRatio(ratio); }

/* -------------------------------------------------------------------------- */

void Stretcher::setPitch(double ratio) { m_stretcher.setPitchScale(ratio); }

/* -------------------------------------------------------------------------- */

void Stretcher::last() { m_stretcher.reset(); }

/* -------------------------------------------------------------------------- */

Stretcher::Result Stretcher::process(const float* input, std::size_t inputLength,
    std::size_t inputStart, float* output, std::size_t outputLength,
    std::size_t outputStart, double timeRatio, double pitchRatio)
{
	/* General algorithm -
	1. While the output buffer still has room:
	2. Ask the stretcher: how much output is already buffered with no extra work,
	via available()
	    - If input has run out AND this returns "nothing, ever again",
	    we're truly finished. Stop.
	3. If what's buffered isn't enough to fill the rest of the output buffer,
	AND there's still source audio left:
	    a. Ask the stretcher how many input frames it wants before it can produce
	    more output (getSamplesRequired())
	    b. Hand it that many frames, or fewer, if the input source is about to run
	    out (process()), flagging "final" if this is truly the last chunk of source audio
	    c. Re-check how much is now buffered
	4. Take as much as fits from what's buffered into the caller's output buffer
	(retrieve()). Anything left over stays queued inside the stretcher for next time
	5. If the output buffer is now full, then stop and return to caller (there may
	still be more real audio queued for later calls)
	6. Only once step 2 confirms "nothing left, ever" do we reset() the stretcher,
	not simply when the source has run dry, since real audio can still be queued
	internally by librubberband for several more calls after the last input was
	fed (especially when stretching audio longer).

	Note on interleaved audio -
	Rubber Band expects de-interleaved audio as an array of channel pointers (one
	pointer per channel), while Stretcher::process() uses a flat stereo buffer for
	a simpler API. So here we build temporary channel-pointer arrays that point
	into the flat input/output buffers:

	- inputPtrs[0]  -> left channel
	- inputPtrs[1]  -> right channel
	- outputPtrs[0] -> left channel
	- outputPtrs[1] -> right channel

	This is only an adapter for the Rubber Band API; the actual audio data remains
	flat and planar in the caller-facing interface. */

	assert(input != nullptr);
	assert(output != nullptr);
	assert(inputStart <= inputLength); // <= now, since a drain-only call may pass inputStart == inputLength
	assert(outputStart < outputLength);

	m_stretcher.setTimeRatio(timeRatio);
	m_stretcher.setPitchScale(pitchRatio);

	std::size_t framesGenerated  = 0;
	std::size_t framesUsed       = 0;
	bool        outputIsEmpty    = true;
	bool        inputIsAvailable = inputStart < inputLength;
	bool        fullyDrained     = false; // true only once available() actually reaches -1

	while (outputIsEmpty)
	{
		/* We check available() first, before doing anything else, because it's
		a free status check that tells us whether the stretcher already has
		output waiting, possibly enough to skip calling process() entirely
		this iteration. Here specifically, a return of -1 means "input ended
		AND every last frame has been retrieved": the stretcher is completely
		spent and will never produce anything else, no matter what we do, so we
		bail out immediately instead of looping further. */

		if (!inputIsAvailable && m_stretcher.available() < 0)
		{
			fullyDrained = true;
			break;
		}

		std::size_t       framesAvailable = static_cast<std::size_t>(std::max(m_stretcher.available(), 0));
		const std::size_t outputRemaining = outputLength - outputStart - framesGenerated;

		/* Feed more input to the stretcher only if what's already available()
		isn't enough to cover the rest of the output buffer. */

		if (framesAvailable < outputRemaining && inputIsAvailable)
		{
			const float* inputPtrs[G_MAX_IO_CHANS] = {
			    input + inputStart + framesUsed,
			    input + inputStart + inputLength + framesUsed};

			/* stretcher.getSamplesRequired() means: "how much input the stretcher
			needs before it can do any work?" This tells us the right-sized batch
			to hand over next. */

			const std::size_t framesRequired  = m_stretcher.getSamplesRequired();
			const std::size_t framesRemaining = inputLength - inputStart - framesUsed;
			const std::size_t framesToProcess = std::min(framesRequired, framesRemaining);

			/* If this batch covers everything left in the input file, this is
			the last input we'll ever hand over. Tell the stretcher so via the
			`final` flag below, so it knows to flush its internal buffers rather
			than wait for more input that isn't coming. */

			inputIsAvailable = framesToProcess < framesRemaining;

			/* strecther.process(): this is the call that consumes input. It may
			produce more output than fits in our output buffer (the surplus stays
			queued inside the stretcher until a later call to available()/retrieve()),
			or it may produce nothing yet if it's still filling its internal window. */

			m_stretcher.process(inputPtrs, framesToProcess, !inputIsAvailable);
			framesUsed += framesToProcess;

			/* Re-check available(). The process() call above may have just been
			the final flush, in which case draining could already be complete
			(available() now -1) with nothing to retrieve. */

			if (!inputIsAvailable && m_stretcher.available() < 0)
			{
				fullyDrained = true;
				break;
			}
			framesAvailable = static_cast<std::size_t>(std::max(m_stretcher.available(), 0));
		}

		float* outputPtrs[G_MAX_IO_CHANS] = {
		    output + outputStart + framesGenerated,
		    output + outputLength + outputStart + framesGenerated};

		/* stretcher.retrieve(): gets stretched data. We only ever take as much as
		our buffer has room for (outputRemaining); anything left over stays queued
		inside the stretcher for the next callback. */

		const std::size_t framesToRetrieve = std::min(framesAvailable, outputRemaining);
		const std::size_t framesRetrieved  = m_stretcher.retrieve(outputPtrs, framesToRetrieve);

		framesGenerated += framesRetrieved;
		outputIsEmpty = framesGenerated < (outputLength - outputStart);

		/* Safety check: input is gone and nothing is available from the stretcher,
		but make sure to avoid spinning forever just in case retrieve() still
		returns nothing here. */

		if (!inputIsAvailable && framesAvailable == 0 && framesRetrieved == 0)
		{
			fullyDrained = true;
			break;
		}
	}

	/* Only reset once the stretcher has truly given up everything it will ever
	produce, NOT just because the source file ran out of input. A long tail (e.g.
	when slowing audio down) can still have real, unread output queued up after
	input ends, spanning into future callbacks. */

	if (fullyDrained)
		m_stretcher.reset();

	return {
	    .used      = static_cast<long>(framesUsed),
	    .generated = static_cast<long>(framesGenerated)};
}
} // namespace giada::m
