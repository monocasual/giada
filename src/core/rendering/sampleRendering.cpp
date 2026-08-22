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

#include "src/core/rendering/sampleRendering.h"
#include "src/core/channels/channel.h"
#include "src/core/plugins/pluginHost.h"
#include "src/core/rendering/sampleAdvance.h"
#include "src/core/resampler.h"
#include "src/core/stretcher.h"
#include "src/core/wave.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include <cassert>

namespace giada::m::rendering
{
namespace
{
ReadResult readResampled_(const Sample& sample, mcl::AudioBuffer& dest, Frame start,
    Frame offset, const Resampler& resampler)
{
	assert(sample.wave != nullptr);

	const float* srcPtr = sample.wave->getBuffer().getChannelView(0).data();
	float*       dstPtr = dest.getChannelView(0).data() + offset;

	Resampler::Result res = resampler.process(
	    /*input=*/srcPtr,
	    /*channelStride=*/sample.wave->getBuffer().countFrames(), // true full buffer size, never shrinks
	    /*inputPos=*/start,
	    /*inputLength=*/sample.range.getB(),
	    /*output=*/dstPtr,
	    /*outputLength=*/dest.countFrames() - offset,
	    /*ratio=*/sample.pitch);

	return {
	    static_cast<Frame>(res.used),
	    static_cast<Frame>(res.generated),
	    true};
}

/* -------------------------------------------------------------------------- */

ReadResult readStretched_(const Sample& sample, mcl::AudioBuffer& dest, Frame start,
    Frame offset, Stretcher& stretcher)
{
	assert(sample.wave != nullptr);

	Stretcher::Result res = stretcher.process(
	    /*input=*/sample.wave->getBuffer(),
	    /*inputStart=*/start,
	    /*inputEnd=*/sample.range.getB(),
	    /*output=*/dest,
	    /*outputStart=*/offset,
	    /*timeRatio=*/sample.time,
	    /*pitchRatio=*/sample.pitch);

	return {
	    static_cast<Frame>(res.used),
	    static_cast<Frame>(res.generated),
	    res.finished};
}

/* -------------------------------------------------------------------------- */

ReadResult readCopy_(const Sample& sample, mcl::AudioBuffer& dest, Frame start,
    Frame offset)
{
	assert(sample.wave != nullptr);

	Frame used = dest.countFrames() - offset;
	if (used > sample.range.getB() - start)
		used = sample.range.getB() - start;

	dest.setAll(sample.wave->getBuffer(), used, start, offset);

	return {used, used, true};
}

/* -------------------------------------------------------------------------- */

/* onSampleEnd
Things to do when the last frame has been reached. 'natural' == true if the
rendering has ended because the end of the sample has been reached.
'natural' == false if the rendering has been manually interrupted (by a
RenderInfo::Mode::STOP type). Returns whether the sample should loop or not. */

bool onSampleEnd_(const Channel& ch, bool seqIsRunning, bool natural)
{
	ChannelShared& shared         = *ch.shared;
	const bool     isLoop         = ch.sampleChannel->isAnyLoopMode();
	const bool     isLoopOnce     = ch.sampleChannel->isAnyLoopOnceMode();
	const bool     isSingleNoLoop = ch.sampleChannel->isAnyNonLoopingSingleMode();
	bool           shouldLoop     = false;

	switch (shared.playStatus.load())
	{
	case ChannelStatus::PLAY:
		/* Stop LOOP_* when the sequencer is off, or SINGLE_* except for
		SINGLE_ENDLESS, which runs forever unless it's in ENDING mode.
		Other loop once modes are put in wait mode. */
		if (isSingleNoLoop || (isLoop && !seqIsRunning) || !natural)
			shared.playStatus.store(ChannelStatus::OFF);
		else if (isLoopOnce)
			shared.playStatus.store(ChannelStatus::WAIT);
		else
			shouldLoop = true;
		break;

	case ChannelStatus::ENDING:
		shared.playStatus.store(ChannelStatus::OFF);
		break;

	default:
		break;
	}

	return shouldLoop;
}

/* -------------------------------------------------------------------------- */

void stop_(const Channel& ch, mcl::AudioBuffer& buf, Frame offset, bool seqIsRunning)
{
	onSampleEnd_(ch, seqIsRunning, /*natural=*/false);
	if (offset != 0)
		buf.clear(offset);
}

/* -------------------------------------------------------------------------- */

Frame render_(const Channel& ch, mcl::AudioBuffer& buf, Scene scene, Frame tracker, Frame offset, bool seqIsRunning, bool testEnd)
{
	const Sample&    sample    = ch.sampleChannel->getSample(scene);
	const Resampler& resampler = ch.shared->resampler.value();
	Stretcher&       stretcher = ch.shared->stretcher.value();

	if (sample.wave == nullptr)
		return tracker;

	while (true)
	{
		ReadResult res = readWave(sample, buf, tracker, offset, resampler, stretcher);
		tracker += res.used;
		offset += res.generated;

		if (tracker >= sample.range.getB() && res.finished)
		{
			tracker = sample.range.getA();
			ch.shared->resampler->last();
			ch.shared->stretcher->last();

			if (testEnd)
			{
				const bool shouldLoop = onSampleEnd_(ch, seqIsRunning, /*natural=*/true);
				if (!shouldLoop)
					break;
			}
		}

		/* Break here if the buffer has been filled completely: there's nothing
		else do to. */

		if (offset >= buf.countFrames())
			break;
	}

	return tracker;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void renderSampleChannel(const Channel& ch, Scene scene, bool seqIsRunning)
{
	RenderInfo renderInfo;
	while (ch.shared->renderQueue->try_dequeue(renderInfo))
		;

	const auto        range     = ch.sampleChannel->getRange(scene);
	const Resampler&  resampler = ch.shared->resampler.value();
	Stretcher&        stretcher = ch.shared->stretcher.value();
	mcl::AudioBuffer& buf       = ch.shared->audioBuffer;
	Frame             tracker   = std::clamp(ch.shared->tracker.load(), range.getA(), range.getB()); /* Make sure tracker stays within begin-end range. */

	if (renderInfo.mode == RenderInfo::Mode::NORMAL)
	{
		tracker = render_(ch, buf, scene, tracker, renderInfo.offset, seqIsRunning, /*testEnd=*/true);
	}
	else
	{
		/* Both modes: fill whole buffer first: [abcdefghijklmnopq]. Also tell
		resampler this is the last read before either a rewind or a stop. Don't run
		complex logic on sample end (testEnd=false), we just want to fill the buffer
		as much as possible here. */

		render_(ch, buf, scene, tracker, 0, seqIsRunning, /*testEnd=*/false);
		resampler.last();
		stretcher.last();

		/* Mode::REWIND: fill buffer from offset:  [abcdefghi|abcdfefg]
		   Mode::STOP:   clear buffer from offset: [abcdefghi|--------] */

		if (renderInfo.mode == RenderInfo::Mode::REWIND)
		{
			tracker = render_(ch, buf, scene, range.getA(), renderInfo.offset, seqIsRunning, /*testEnd=*/true);
		}
		else
		{
			stop_(ch, buf, renderInfo.offset, seqIsRunning);
			tracker = range.getA();
		}
	}

	ch.shared->tracker.store(tracker);
}

/* -------------------------------------------------------------------------- */

void renderSampleChannelInput(const Channel& ch, const mcl::AudioBuffer& in)
{
	ch.shared->audioBuffer.set(in, 0, 0, /*gain=*/1.0f); // add, don't overwrite
	ch.shared->audioBuffer.set(in, 1, 1, /*gain=*/1.0f); // add, don't overwrite
}

/* -------------------------------------------------------------------------- */

ReadResult readWave(const Sample& sample, mcl::AudioBuffer& out, Frame start,
    Frame offset, const Resampler& resampler, Stretcher& stretcher)
{
	assert(sample.wave != nullptr);
	assert(start >= 0);
	assert(sample.range.getB() <= sample.wave->getBuffer().countFrames());
	assert(offset < out.countFrames());

	if (sample.playbackMode == PlaybackMode::TAPE)
	{
		if (sample.pitch == 1.0f)
			return readCopy_(sample, out, start, offset);
		else
			return readResampled_(sample, out, start, offset, resampler);
	}
	else // PlaybackMode::ELASTIC
		return readStretched_(sample, out, start, offset, stretcher);
}
} // namespace giada::m::rendering
