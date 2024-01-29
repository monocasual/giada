/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "core/channels/channel.h"
#include "core/plugins/pluginHost.h"
#include "core/rendering/sampleAdvance.h"
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

/* -------------------------------------------------------------------------- */

/* onSampleEnd
Things to do when the last frame has been reached. 'natural' == true if the 
rendering has ended because the end of the sample has ben reached. 
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

Frame render_(const Channel& ch, mcl::AudioBuffer& buf, Frame tracker, Frame offset, bool seqIsRunning)
{
	const Frame      begin     = ch.sampleChannel->begin;
	const Frame      end       = ch.sampleChannel->end;
	const float      pitch     = ch.sampleChannel->pitch;
	const Wave&      wave      = *ch.sampleChannel->getWave();
	const Resampler& resampler = ch.shared->resampler.value();

	while (true)
	{
		ReadResult res = readWave(wave, buf, tracker, end, offset, pitch, resampler);
		tracker += res.used;
		offset += res.generated;

		/* Break here if the buffer has been filled completely: there's nothing 
		else do to. */

		if (offset >= buf.countFrames())
			break;

		if (tracker >= end)
		{
			tracker = begin;
			ch.shared->resampler->last();
			const bool shouldLoop = onSampleEnd_(ch, seqIsRunning, /*natural=*/true);
			if (!shouldLoop)
				break;
		}
	}

	return tracker;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void renderSampleChannel(const Channel& ch, bool seqIsRunning)
{
	RenderInfo renderInfo;
	while (ch.shared->renderQueue->pop(renderInfo))
		;

	const Frame       begin   = ch.sampleChannel->begin;
	const Frame       end     = ch.sampleChannel->end;
	mcl::AudioBuffer& buf     = ch.shared->audioBuffer;
	Frame             tracker = std::clamp(ch.shared->tracker.load(), begin, end); /* Make sure tracker stays within begin-end range. */

	if (renderInfo.mode == RenderInfo::Mode::NORMAL)
	{
		tracker = render_(ch, buf, tracker, renderInfo.offset, seqIsRunning);
	}
	else
	{
		/* Both modes: fill whole buffer first: [abcdefghijklmnopq]. */

		render_(ch, buf, tracker, 0, seqIsRunning);

		/* Mode::REWIND: fill buffer from offset:  [abcdefghi|abcdfefg]
		   Mode::STOP:   clear buffer from offset: [abcdefghi|--------] */

		if (renderInfo.mode == RenderInfo::Mode::REWIND)
		{
			tracker = render_(ch, buf, begin, renderInfo.offset, seqIsRunning);
		}
		else
		{
			stop_(ch, buf, renderInfo.offset, seqIsRunning);
			tracker = begin;
		}
	}

	ch.shared->tracker.store(tracker);
}

/* -------------------------------------------------------------------------- */

void renderSampleChannelPlugins(const Channel& ch, PluginHost& pluginHost)
{
	pluginHost.processStack(ch.shared->audioBuffer, ch.plugins, nullptr);
}

/* -------------------------------------------------------------------------- */

void renderSampleChannelInput(const Channel& ch, const mcl::AudioBuffer& in)
{
	ch.shared->audioBuffer.set(in, /*gain=*/1.0f); // add, don't overwrite
}

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
