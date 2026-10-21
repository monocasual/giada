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
#include "src/core/model/wave.h"
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
ReadResult readResampled_(
    const model::SampleChannelView& view,
    Frame                           start,
    Frame                           offset)
{
	const model::Channel::Rendering& rendering = view.getRendering();
	const model::Wave&               wave      = *view.getWave();
	const Sample&                    sample    = view.getSample();
	const Resampler::Result          res       = rendering.resampleToAudioBufferFrom(
        wave.getBuffer(), start, sample.range.getB(), offset, sample.pitch);

	return {
	    static_cast<Frame>(res.used),
	    static_cast<Frame>(res.generated),
	    true};
}

/* -------------------------------------------------------------------------- */

ReadResult readStretched_(
    const model::SampleChannelView& view,
    Frame                           start,
    Frame                           offset)
{
	const model::Channel::Rendering& rendering = view.getRendering();
	const model::Wave&               wave      = *view.getWave();
	const Sample&                    sample    = view.getSample();
	const Stretcher::Result          res       = rendering.stretchToAudioBufferFrom(
        wave.getBuffer(), start, sample.range.getB(), offset, sample.time, sample.pitch);

	return {
	    static_cast<Frame>(res.used),
	    static_cast<Frame>(res.generated),
	    res.finished};
}

/* -------------------------------------------------------------------------- */

ReadResult readCopy_(
    const model::SampleChannelView& view,
    Frame                           start,
    Frame                           offset)
{
	const model::Channel::Rendering& rendering = view.getRendering();
	const model::Wave&               wave      = *view.getWave();
	const Sample&                    sample    = view.getSample();
	Frame                            used      = rendering.getAudioBuffer().countFrames() - offset;
	if (used > sample.range.getB() - start)
		used = sample.range.getB() - start;

	rendering.copyToAudioBufferFrom(wave.getBuffer(), used, start, offset);

	return {used, used, true};
}

/* -------------------------------------------------------------------------- */

/* onSampleEnd
Things to do when the last frame has been reached. 'natural' == true if the
rendering has ended because the end of the sample has been reached.
'natural' == false if the rendering has been manually interrupted (by a
RenderInfo::Mode::STOP type). Returns whether the sample should loop or not. */

bool onSampleEnd_(
    const model::SampleChannelView& view,
    bool                            seqIsRunning,
    bool                            natural)
{
	const model::Channel&             ch             = view.getChannel();
	const model::Channel::Parameters& params         = view.getParameters();
	const bool                        isLoop         = ch.sampleChannel->isAnyLoopMode();
	const bool                        isLoopOnce     = ch.sampleChannel->isAnyLoopOnceMode();
	const bool                        isSingleNoLoop = ch.sampleChannel->isAnyNonLoopingSingleMode();
	bool                              shouldLoop     = false;

	switch (params.getPlayStatus())
	{
	case ChannelStatus::PLAY:
		/* Stop LOOP_* when the sequencer is off, or SINGLE_* except for
		SINGLE_ENDLESS, which runs forever unless it's in ENDING mode.
		Other loop once modes are put in wait mode. */
		if (isSingleNoLoop || (isLoop && !seqIsRunning) || !natural)
			params.setPlayStatus(ChannelStatus::OFF);
		else if (isLoopOnce)
			params.setPlayStatus(ChannelStatus::WAIT);
		else
			shouldLoop = true;
		break;

	case ChannelStatus::ENDING:
		params.setPlayStatus(ChannelStatus::OFF);
		break;

	default:
		break;
	}

	return shouldLoop;
}

/* -------------------------------------------------------------------------- */

void stop_(
    const model::SampleChannelView& view,
    Frame                           offset,
    bool                            seqIsRunning)
{
	onSampleEnd_(view, seqIsRunning, /*natural=*/false);
	const model::Channel::Rendering& rendering = view.getRendering();
	if (offset != 0)
		rendering.clearAudioBuffer(offset);
}

/* -------------------------------------------------------------------------- */

Frame render_(
    const model::SampleChannelView& view,
    Frame                           tracker,
    Frame                           offset,
    bool                            seqIsRunning,
    bool                            testEnd)
{
	if (!view.hasWave())
		return tracker;

	const Sample&                    sample    = view.getSample();
	const model::Channel::Rendering& rendering = view.getRendering();

	while (true)
	{
		ReadResult res = readWave(view, tracker, offset);
		tracker += res.used;
		offset += res.generated;

		if (tracker >= sample.range.getB() && res.finished)
		{
			tracker = sample.range.getA();
			rendering.notifyLastChunk();

			if (testEnd)
			{
				const bool shouldLoop = onSampleEnd_(view, seqIsRunning, /*natural=*/true);
				if (!shouldLoop)
					break;
			}
		}

		/* Break here if the buffer has been filled completely: there's nothing
		else do to. */

		if (offset >= rendering.getAudioBuffer().countFrames())
			break;
	}

	return tracker;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void renderSampleChannel(
    const model::SampleChannelView& view,
    RenderInfo                      localRenderInfo,
    bool                            seqIsRunning)
{
	/* Each audio block, a sample channel can receive a "render command" (play
	normally, rewind, stop) from two places:

	  - local: decided by the audio thread itself while it reads the sequencer
	    (e.g. the loop restarted, so rewind at frame 100 of this block);
	  - external: written by another thread, like the UI or MIDI (e.g. the user
	    pressed the kill key). Those threads can't know where they are inside
	    the block, so the offset is always 0.

	If an external command is waiting (i.e RenderInfo::Mode is NOT NONE), use
	it: user actions win over the sequencer. Reading it also clears it, so it
	runs only once. If there is no command from anywhere, just play normally
	from the start of the block. */

	const model::Channel::Rendering&  rendering          = view.getRendering();
	const model::Channel::Parameters& params             = view.getParameters();
	const RenderInfo::Mode            externalRenderMode = rendering.getPendingRenderMode();

	RenderInfo renderInfo = externalRenderMode != RenderInfo::Mode::NONE ? RenderInfo{externalRenderMode, 0} : localRenderInfo;
	if (renderInfo.mode == RenderInfo::Mode::NONE)
		renderInfo.mode = RenderInfo::Mode::NORMAL;

	const auto range   = view.getSample().range;
	Frame      tracker = std::clamp(params.getTracker(), range.getA(), range.getB()); /* Make sure tracker stays within begin-end range. */

	if (renderInfo.mode == RenderInfo::Mode::NORMAL)
	{
		tracker = render_(view, tracker, renderInfo.offset, seqIsRunning, /*testEnd=*/true);
	}
	else
	{
		/* Both modes: fill whole buffer first: [abcdefghijklmnopq]. Also tell
		resampler this is the last read before either a rewind or a stop. Don't run
		complex logic on sample end (testEnd=false), we just want to fill the buffer
		as much as possible here. */

		render_(view, tracker, 0, seqIsRunning, /*testEnd=*/false);
		rendering.notifyLastChunk();

		/* Mode::REWIND: fill buffer from offset:  [abcdefghi|abcdfefg]
		   Mode::STOP:   clear buffer from offset: [abcdefghi|--------] */

		if (renderInfo.mode == RenderInfo::Mode::REWIND)
		{
			tracker = render_(view, range.getA(), renderInfo.offset, seqIsRunning, /*testEnd=*/true);
		}
		else
		{
			stop_(view, renderInfo.offset, seqIsRunning);
			tracker = range.getA();
		}
	}

	params.setTracker(tracker);
}

/* -------------------------------------------------------------------------- */

void renderSampleChannelInput(const Channel& ch, const mcl::AudioBuffer& in)
{
	ch.shared->audioBuffer.set(in, 0, 0, /*gain=*/1.0f); // add, don't overwrite
	ch.shared->audioBuffer.set(in, 1, 1, /*gain=*/1.0f); // add, don't overwrite
}

/* -------------------------------------------------------------------------- */

ReadResult readWave(
    const model::SampleChannelView& view,
    Frame                           start,
    Frame                           offset)
{
	assert(view.hasWave());

	const model::Channel::Rendering& rendering = view.getRendering();
	const model::Wave&               wave      = *view.getWave();
	const Sample&                    sample    = view.getSample();

	assert(start >= 0);
	assert(sample.range.getB() <= wave.getBuffer().countFrames());
	assert(offset < rendering.getAudioBuffer().countFrames());

	if (sample.playbackMode == PlaybackMode::TAPE)
	{
		if (sample.pitch == 1.0f)
			return readCopy_(view, start, offset);
		else
			return readResampled_(view, start, offset);
	}
	else // PlaybackMode::ELASTIC
		return readStretched_(view, start, offset);
}
} // namespace giada::m::rendering
