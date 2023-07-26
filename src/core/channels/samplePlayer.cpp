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

#include "samplePlayer.h"
#include "core/channels/channel.h"
#include "core/wave.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include <algorithm>
#include <cassert>

namespace giada::m
{
SamplePlayer::SamplePlayer(Resampler* r)
: shift(0)
, begin(0)
, end(0)
, velocityAsVol(false)
, waveReader(r)
, wave(nullptr)
{
}

/* -------------------------------------------------------------------------- */

SamplePlayer::SamplePlayer(const Patch::Channel& p, float samplerateRatio, Resampler* r)
: shift(p.shift)
, begin(p.begin)
, end(p.end)
, velocityAsVol(p.midiInVeloAsVol)
, waveReader(r)
, onLastFrame(nullptr)
{
}

/* -------------------------------------------------------------------------- */

bool SamplePlayer::hasWave() const { return wave != nullptr; }
bool SamplePlayer::hasLogicalWave() const { return hasWave() && wave->isLogical(); }
bool SamplePlayer::hasEditedWave() const { return hasWave() && wave->isEdited(); }

/* -------------------------------------------------------------------------- */

Wave* SamplePlayer::getWave() const
{
	return wave;
}

ID SamplePlayer::getWaveId() const
{
	if (hasWave())
		return wave->id;
	return 0;
}

/* -------------------------------------------------------------------------- */

Frame SamplePlayer::getWaveSize() const
{
	return hasWave() ? wave->getBuffer().countFrames() : 0;
}

/* -------------------------------------------------------------------------- */

void SamplePlayer::render(const Channel& ch, Render renderInfo, bool seqIsRunning) const
{
	if (wave == nullptr)
		return;

	mcl::AudioBuffer& buf     = ch.shared->audioBuffer;
	Frame             tracker = std::clamp(ch.shared->tracker.load(), begin, end); /* Make sure tracker stays within begin-end range. */

	if (renderInfo.mode == Render::Mode::NORMAL)
	{
		tracker = render(ch, buf, tracker, renderInfo.offset, seqIsRunning);
	}
	else
	{
		/* Both modes: 1st = [abcdefghijklmnopq] 
		No need for fancy render() here. You don't want the chance to trigger 
		onLastFrame() at this point which would invalidate the rewind (a listener
		might stop the rendering): fillBuffer() is just enough. Just notify 
		waveReader this is the last read before rewind. */

		tracker = fillBuffer(buf, tracker, 0, ch.sampleChannel->pitch).used;
		waveReader.last();

		/* Mode::REWIND: 2nd = [abcdefghi|abcdfefg]
		   Mode::STOP:   2nd = [abcdefghi|--------] */

		if (renderInfo.mode == Render::Mode::REWIND)
			tracker = render(ch, buf, begin, renderInfo.offset, seqIsRunning);
		else
			tracker = stop(buf, renderInfo.offset, seqIsRunning);
	}

	ch.shared->tracker.store(tracker);
}

/* -------------------------------------------------------------------------- */

Frame SamplePlayer::render(const Channel& ch, mcl::AudioBuffer& buf, Frame tracker, Frame offset, bool seqIsRunning) const
{
	const ChannelStatus status = ch.shared->playStatus.load();

	/* First pass rendering. */

	WaveReader::Result res = fillBuffer(buf, tracker, offset, ch.sampleChannel->pitch);
	tracker += res.used;

	/* Second pass rendering: if tracker has looped, special care is needed. If 
	the	channel is in loop mode, fill the second part of the buffer with data
	coming from the sample's head, starting at 'res.generated' offset. */

	if (tracker >= end)
	{
		assert(onLastFrame != nullptr);

		tracker = begin;
		waveReader.last();
		onLastFrame(/*natural=*/true, seqIsRunning);

		if (shouldLoop(ch.sampleChannel->mode, status) && res.generated < buf.countFrames())
			tracker += fillBuffer(buf, tracker, res.generated, ch.sampleChannel->pitch).used;
	}

	return tracker;
}

/* -------------------------------------------------------------------------- */

Frame SamplePlayer::stop(mcl::AudioBuffer& buf, Frame offset, bool seqIsRunning) const
{
	assert(onLastFrame != nullptr);

	onLastFrame(/*natural=*/false, seqIsRunning);

	if (offset != 0)
		buf.clear(offset);

	return begin;
}

/* -------------------------------------------------------------------------- */

void SamplePlayer::kickIn(ChannelShared& shared, Frame f)
{
	shared.tracker.store(f);
	shared.playStatus.store(ChannelStatus::PLAY);
}

/* -------------------------------------------------------------------------- */

WaveReader::Result SamplePlayer::fillBuffer(mcl::AudioBuffer& buf, Frame start, Frame offset, float pitch) const
{
	assert(wave != nullptr);

	return waveReader.fill(*wave, buf, start, end, offset, pitch);
}

/* -------------------------------------------------------------------------- */

bool SamplePlayer::shouldLoop(SamplePlayerMode mode, ChannelStatus status) const
{
	return (mode == SamplePlayerMode::LOOP_BASIC ||
	           mode == SamplePlayerMode::LOOP_REPEAT ||
	           mode == SamplePlayerMode::SINGLE_ENDLESS) &&
	       status == ChannelStatus::PLAY; // Don't loop if ENDING
}
} // namespace giada::m