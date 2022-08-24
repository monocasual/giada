/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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
: pitch(G_DEFAULT_PITCH)
, mode(SamplePlayerMode::SINGLE_BASIC)
, shift(0)
, begin(0)
, end(0)
, velocityAsVol(false)
, waveReader(r)
{
}

/* -------------------------------------------------------------------------- */

SamplePlayer::SamplePlayer(const Patch::Channel& p, float samplerateRatio, Resampler* r, Wave* w)
: pitch(p.pitch)
, mode(p.mode)
, shift(p.shift)
, begin(p.begin)
, end(p.end)
, velocityAsVol(p.midiInVeloAsVol)
, waveReader(r)
, onLastFrame(nullptr)
{
	setWave(w, samplerateRatio);
}

/* -------------------------------------------------------------------------- */

bool SamplePlayer::hasWave() const { return waveReader.wave != nullptr; }
bool SamplePlayer::hasLogicalWave() const { return hasWave() && waveReader.wave->isLogical(); }
bool SamplePlayer::hasEditedWave() const { return hasWave() && waveReader.wave->isEdited(); }

/* -------------------------------------------------------------------------- */

bool SamplePlayer::isAnyLoopMode() const
{
	return mode == SamplePlayerMode::LOOP_BASIC ||
	       mode == SamplePlayerMode::LOOP_ONCE ||
	       mode == SamplePlayerMode::LOOP_REPEAT ||
	       mode == SamplePlayerMode::LOOP_ONCE_BAR;
}

/* -------------------------------------------------------------------------- */

Wave* SamplePlayer::getWave() const
{
	return waveReader.wave;
}

ID SamplePlayer::getWaveId() const
{
	if (hasWave())
		return waveReader.wave->id;
	return 0;
}

/* -------------------------------------------------------------------------- */

Frame SamplePlayer::getWaveSize() const
{
	return hasWave() ? waveReader.wave->getBuffer().countFrames() : 0;
}

/* -------------------------------------------------------------------------- */

void SamplePlayer::react(const EventDispatcher::Event& e)
{
	if (e.type == EventDispatcher::EventType::CHANNEL_PITCH)
		pitch = std::any_cast<float>(e.data);
}

/* -------------------------------------------------------------------------- */

void SamplePlayer::render(ChannelShared& shared, Render renderInfo) const
{
	if (waveReader.wave == nullptr)
		return;

	mcl::AudioBuffer&   buf     = shared.audioBuffer;
	Frame               tracker = std::clamp(shared.tracker.load(), begin, end); /* Make sure tracker stays within begin-end range. */
	const ChannelStatus status  = shared.playStatus.load();

	if (renderInfo.mode == Render::Mode::NORMAL)
	{
		tracker = render(buf, tracker, renderInfo.offset, status);
	}
	else
	{
		/* Both modes: 1st = [abcdefghijklmnopq] 
		No need for fancy render() here. You don't want the chance to trigger 
		onLastFrame() at this point which would invalidate the rewind (a listener
		might stop the rendering): fillBuffer() is just enough. Just notify 
		waveReader this is the last read before rewind. */

		tracker = fillBuffer(buf, tracker, 0).used;
		waveReader.last();

		/* Mode::REWIND: 2nd = [abcdefghi|abcdfefg]
		   Mode::STOP:   2nd = [abcdefghi|--------] */

		if (renderInfo.mode == Render::Mode::REWIND)
			tracker = render(buf, begin, renderInfo.offset, status);
		else
			tracker = stop(buf, renderInfo.offset);
	}

	shared.tracker.store(tracker);
}

/* -------------------------------------------------------------------------- */

Frame SamplePlayer::render(mcl::AudioBuffer& buf, Frame tracker, Frame offset, ChannelStatus status) const
{
	/* First pass rendering. */

	WaveReader::Result res = fillBuffer(buf, tracker, offset);
	tracker += res.used;

	/* Second pass rendering: if tracker has looped, special care is needed. If 
	the	channel is in loop mode, fill the second part of the buffer with data
	coming from the sample's head, starting at 'res.generated' offset. */

	if (tracker >= end)
	{
		assert(onLastFrame != nullptr);

		tracker = begin;
		waveReader.last();
		onLastFrame(/*natural=*/true);

		if (shouldLoop(status) && res.generated < buf.countFrames())
			tracker += fillBuffer(buf, tracker, res.generated).used;
	}

	return tracker;
}

/* -------------------------------------------------------------------------- */

Frame SamplePlayer::stop(mcl::AudioBuffer& buf, Frame offset) const
{
	assert(onLastFrame != nullptr);

	onLastFrame(/*natural=*/false);

	if (offset != 0)
		buf.clear(offset);

	return begin;
}

/* -------------------------------------------------------------------------- */

void SamplePlayer::loadWave(ChannelShared& shared, Wave* w, Frame newBegin, Frame newEnd, Frame newShift)
{
	waveReader.wave = w;

	shared.tracker.store(0);
	shared.playStatus.store(w != nullptr ? ChannelStatus::OFF : ChannelStatus::EMPTY);
	shift = 0;
	begin = 0;
	end   = 0;

	if (w != nullptr)
	{
		shift = newShift == -1 ? 0 : newShift;
		begin = newBegin == -1 ? 0 : newBegin;
		end   = newEnd == -1 ? w->getBuffer().countFrames() - 1 : newEnd;
	}
}

/* -------------------------------------------------------------------------- */

void SamplePlayer::setWave(Wave* w, float samplerateRatio)
{
	if (w == nullptr)
	{
		waveReader.wave = nullptr;
		return;
	}

	waveReader.wave = w;

	if (samplerateRatio != 1.0f)
	{
		begin *= samplerateRatio;
		end *= samplerateRatio;
		shift *= samplerateRatio;
	}
}

/* -------------------------------------------------------------------------- */

void SamplePlayer::kickIn(ChannelShared& shared, Frame f)
{
	shared.tracker.store(f);
	shared.playStatus.store(ChannelStatus::PLAY);
}

/* -------------------------------------------------------------------------- */

WaveReader::Result SamplePlayer::fillBuffer(mcl::AudioBuffer& buf, Frame start, Frame offset) const
{
	return waveReader.fill(buf, start, end, offset, pitch);
}

/* -------------------------------------------------------------------------- */

bool SamplePlayer::shouldLoop(ChannelStatus status) const
{
	return (mode == SamplePlayerMode::LOOP_BASIC ||
	           mode == SamplePlayerMode::LOOP_REPEAT ||
	           mode == SamplePlayerMode::SINGLE_ENDLESS) &&
	       status == ChannelStatus::PLAY; // Don't loop if ENDING
}
} // namespace giada::m