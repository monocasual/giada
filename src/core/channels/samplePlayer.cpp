/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

using namespace mcl;

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
		pitch = std::get<float>(e.data);
}

/* -------------------------------------------------------------------------- */

void SamplePlayer::render(ChannelShared& shared) const
{
	if (waveReader.wave == nullptr)
		return;

	/* Make sure tracker stays within begin-end range. */

	Frame tracker = std::clamp(shared.tracker.load(), begin, end);

	/* If rewinding, fill the tail first, then reset the tracker to the begin
    point. The rest is performed as usual. */

	if (shared.rewinding)
	{
		if (tracker < end)
		{
			fillBuffer(shared.audioBuffer, tracker, 0);
			waveReader.last();
		}
		shared.rewinding = false;
		tracker          = begin;
	}

	WaveReader::Result res = fillBuffer(shared.audioBuffer, tracker, shared.offset);
	tracker += res.used;

	/* If tracker has looped, special care is needed for the rendering. If the
    channel is in loop mode, fill the second part of the buffer with data
    coming from the sample's head. */

	if (tracker >= end)
	{
		assert(onLastFrame != nullptr);

		tracker = begin;
		waveReader.last();
		onLastFrame();
		if (shouldLoop() && res.generated < shared.audioBuffer.countFrames())
			tracker += fillBuffer(shared.audioBuffer, tracker, res.generated).used;
	}

	shared.offset = 0;
	shared.tracker.store(tracker);
}

/* -------------------------------------------------------------------------- */

void SamplePlayer::loadWave(ChannelShared& shared, Wave* w)
{
	waveReader.wave = w;

	shared.tracker.store(0);
	shared.playStatus.store(w != nullptr ? ChannelStatus::OFF : ChannelStatus::EMPTY);
	shift = 0;
	begin = 0;
	end   = w != nullptr ? w->getBuffer().countFrames() - 1 : 0;
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

WaveReader::Result SamplePlayer::fillBuffer(AudioBuffer& buf, Frame start, Frame offset) const
{
	return waveReader.fill(buf, start, end, offset, pitch);
}

/* -------------------------------------------------------------------------- */

bool SamplePlayer::shouldLoop() const
{
	return mode == SamplePlayerMode::LOOP_BASIC ||
	       mode == SamplePlayerMode::LOOP_REPEAT ||
	       mode == SamplePlayerMode::SINGLE_ENDLESS;
}
} // namespace giada::m