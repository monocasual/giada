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
RenderInfo::Mode::STOP type). */

void onSampleEnd_(const Channel& ch, bool seqIsRunning, bool natural)
{
	ChannelShared&         shared = *ch.shared;
	const SamplePlayerMode mode   = ch.sampleChannel->mode;
	const bool             isLoop = ch.sampleChannel->isAnyLoopMode();

	switch (shared.playStatus.load())
	{
	case ChannelStatus::PLAY:
		/* Stop LOOP_* when the sequencer is off, or SINGLE_* except for
		SINGLE_ENDLESS, which runs forever unless it's in ENDING mode. 
		Other loop once modes are put in wait mode. */
		if ((mode == SamplePlayerMode::SINGLE_BASIC ||
		        mode == SamplePlayerMode::SINGLE_BASIC_PAUSE ||
		        mode == SamplePlayerMode::SINGLE_PRESS ||
		        mode == SamplePlayerMode::SINGLE_RETRIG) ||
		    (isLoop && !seqIsRunning) || !natural)
			shared.playStatus.store(ChannelStatus::OFF);
		else if (mode == SamplePlayerMode::LOOP_ONCE || mode == SamplePlayerMode::LOOP_ONCE_BAR)
			shared.playStatus.store(ChannelStatus::WAIT);
		break;

	case ChannelStatus::ENDING:
		shared.playStatus.store(ChannelStatus::OFF);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void stop_(const Channel& ch, mcl::AudioBuffer& buf, Frame offset, bool seqIsRunning)
{
	onSampleEnd_(ch, seqIsRunning, /*natural=*/false);
	if (offset != 0)
		buf.clear(offset);
}

/* -------------------------------------------------------------------------- */

bool shouldLoop_(SamplePlayerMode mode, ChannelStatus status)
{
	return (mode == SamplePlayerMode::LOOP_BASIC ||
	           mode == SamplePlayerMode::LOOP_REPEAT ||
	           mode == SamplePlayerMode::SINGLE_ENDLESS) &&
	       status == ChannelStatus::PLAY; // Don't loop if ENDING
}

/* -------------------------------------------------------------------------- */

Frame render_(const Channel& ch, mcl::AudioBuffer& buf, Frame tracker, Frame offset, bool seqIsRunning)
{
	const ChannelStatus    status    = ch.shared->playStatus.load();
	const Frame            begin     = ch.sampleChannel->begin;
	const Frame            end       = ch.sampleChannel->end;
	const float            pitch     = ch.sampleChannel->pitch;
	const SamplePlayerMode mode      = ch.sampleChannel->mode;
	const Wave&            wave      = *ch.sampleChannel->getWave();
	const Resampler&       resampler = ch.shared->resampler.value();

	/* First pass rendering. */

	rendering::ReadResult res = rendering::readWave(wave, buf, tracker, end, offset, pitch, resampler);
	tracker += res.used;

	/* Second pass rendering: if tracker has looped, special care is needed. If 
	the	channel is in loop mode, fill the second part of the buffer with data
	coming from the sample's head, starting at 'res.generated' offset. */

	if (tracker >= end)
	{
		tracker = begin;
		ch.shared->resampler->last();
		onSampleEnd_(ch, seqIsRunning, /*natural=*/true);

		if (shouldLoop_(mode, status) && res.generated < buf.countFrames())
			tracker += rendering::readWave(wave, buf, tracker, end, res.generated, pitch, resampler).used;
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

	const Frame      begin     = ch.sampleChannel->begin;
	const Frame      end       = ch.sampleChannel->end;
	const float      pitch     = ch.sampleChannel->pitch;
	const Resampler& resampler = ch.shared->resampler.value();

	mcl::AudioBuffer& buf     = ch.shared->audioBuffer;
	Frame             tracker = std::clamp(ch.shared->tracker.load(), begin, end); /* Make sure tracker stays within begin-end range. */

	if (renderInfo.mode == RenderInfo::Mode::NORMAL)
	{
		tracker = render_(ch, buf, tracker, renderInfo.offset, seqIsRunning);
	}
	else
	{
		/* Both modes: 1st = [abcdefghijklmnopq] 
		No need for fancy render_ here. You don't want the chance to trigger 
		onSampleEnd_ at this point which would invalidate the rewind (a listener
		might stop the rendering): readWave() is just enough. Just notify
		waveReader this is the last read before rewind. */

		tracker = readWave(*ch.sampleChannel->getWave(), buf, tracker, end, 0, pitch, resampler).used;
		resampler.last();

		/* Mode::REWIND: 2nd = [abcdefghi|abcdfefg]
		   Mode::STOP:   2nd = [abcdefghi|--------] */

		if (renderInfo.mode == RenderInfo::Mode::REWIND)
			tracker = render_(ch, buf, begin, renderInfo.offset, seqIsRunning);
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
