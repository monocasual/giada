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
#ifdef WITH_TESTS
#include "../tests/mocks/waveMock.h"
#include "../tests/mocks/waveReaderMock.h"
#endif

namespace giada::m
{
template <typename WaveReaderI, typename WaveI>
SamplePlayer<WaveReaderI, WaveI>::SamplePlayer(Resampler* r)
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

template <typename WaveReaderI, typename WaveI>
SamplePlayer<WaveReaderI, WaveI>::SamplePlayer(const Patch::Channel& p, float samplerateRatio, Resampler* r, WaveI* w)
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

template <typename WaveReaderI, typename WaveI>
bool SamplePlayer<WaveReaderI, WaveI>::hasWave() const { return waveReader.wave != nullptr; }
template <typename WaveReaderI, typename WaveI>
bool SamplePlayer<WaveReaderI, WaveI>::hasLogicalWave() const { return hasWave() && waveReader.wave->isLogical(); }
template <typename WaveReaderI, typename WaveI>
bool SamplePlayer<WaveReaderI, WaveI>::hasEditedWave() const { return hasWave() && waveReader.wave->isEdited(); }

/* -------------------------------------------------------------------------- */

template <typename WaveReaderI, typename WaveI>
bool SamplePlayer<WaveReaderI, WaveI>::isAnyLoopMode() const
{
	return mode == SamplePlayerMode::LOOP_BASIC ||
	       mode == SamplePlayerMode::LOOP_ONCE ||
	       mode == SamplePlayerMode::LOOP_REPEAT ||
	       mode == SamplePlayerMode::LOOP_ONCE_BAR;
}

/* -------------------------------------------------------------------------- */

template <typename WaveReaderI, typename WaveI>
WaveI* SamplePlayer<WaveReaderI, WaveI>::getWave() const
{
	return waveReader.wave;
}

template <typename WaveReaderI, typename WaveI>
ID SamplePlayer<WaveReaderI, WaveI>::getWaveId() const
{
	if (hasWave())
		return waveReader.wave->id;
	return 0;
}

/* -------------------------------------------------------------------------- */

template <typename WaveReaderI, typename WaveI>
Frame SamplePlayer<WaveReaderI, WaveI>::getWaveSize() const
{
	return hasWave() ? waveReader.wave->getBuffer().countFrames() : 0;
}

/* -------------------------------------------------------------------------- */

template <typename WaveReaderI, typename WaveI>
void SamplePlayer<WaveReaderI, WaveI>::react(const EventDispatcher::Event& e)
{
	if (e.type == EventDispatcher::EventType::CHANNEL_PITCH)
		pitch = std::get<float>(e.data);
}

/* -------------------------------------------------------------------------- */

template <typename WaveReaderI, typename WaveI>
void SamplePlayer<WaveReaderI, WaveI>::render(const Channel& ch) const
{
	if (!isPlaying(ch))
		return;

	/* Make sure tracker stays within begin-end range. */

	Frame tracker = std::clamp(ch.shared->tracker.load(), begin, end);

	/* If rewinding, fill the tail first, then reset the tracker to the begin
    point. The rest is performed as usual. */

	if (ch.shared->rewinding)
	{
		if (tracker < end)
		{
			fillBuffer(ch, tracker, 0);
			waveReader.last();
		}
		ch.shared->rewinding = false;
		tracker              = begin;
	}

	auto res = fillBuffer(ch, tracker, ch.shared->offset);
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
		if (shouldLoop(ch) && res.generated < ch.shared->audioBuffer.countFrames())
			tracker += fillBuffer(ch, tracker, res.generated).used;
	}

	ch.shared->offset = 0;
	ch.shared->tracker.store(tracker);
}

/* -------------------------------------------------------------------------- */

template <typename WaveReaderI, typename WaveI>
void SamplePlayer<WaveReaderI, WaveI>::loadWave(Channel& ch, WaveI* w)
{
	waveReader.wave = w;

	ch.shared->tracker.store(0);
	shift = 0;
	begin = 0;

	if (w != nullptr)
	{
		ch.shared->playStatus.store(ChannelStatus::OFF);
		ch.name = w->getBasename(/*ext=*/false);
		end     = w->getBuffer().countFrames() - 1;
	}
	else
	{
		ch.shared->playStatus.store(ChannelStatus::EMPTY);
		ch.name = "";
		end     = 0;
	}
}

/* -------------------------------------------------------------------------- */

template <typename WaveReaderI, typename WaveI>
void SamplePlayer<WaveReaderI, WaveI>::setWave(WaveI* w, float samplerateRatio)
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

template <typename WaveReaderI, typename WaveI>
void SamplePlayer<WaveReaderI, WaveI>::kickIn(Channel& ch, Frame f)
{
	ch.shared->tracker.store(f);
	ch.shared->playStatus.store(ChannelStatus::PLAY);
}

/* -------------------------------------------------------------------------- */

template <typename WaveReaderI, typename WaveI>
bool SamplePlayer<WaveReaderI, WaveI>::isPlaying(const Channel& ch) const
{
	return waveReader.wave != nullptr && ch.isPlaying();
}

/* -------------------------------------------------------------------------- */

template <typename WaveReaderI, typename WaveI>
typename WaveReaderI::Result
SamplePlayer<WaveReaderI, WaveI>::fillBuffer(const Channel& ch, Frame start, Frame offset) const
{
	return waveReader.fill(ch.shared->audioBuffer, start, end, offset, pitch);
}

/* -------------------------------------------------------------------------- */

template <typename WaveReaderI, typename WaveI>
bool SamplePlayer<WaveReaderI, WaveI>::shouldLoop(const Channel& ch) const
{
	const ChannelStatus playStatus = ch.shared->playStatus.load();

	return (mode == SamplePlayerMode::LOOP_BASIC ||
	           mode == SamplePlayerMode::LOOP_REPEAT ||
	           mode == SamplePlayerMode::SINGLE_ENDLESS) &&
	       playStatus == ChannelStatus::PLAY;
}
/* -------------------------------------------------------------------------- */

template class SamplePlayer<WaveReaderC, Wave>;
#ifdef WITH_TESTS
template class SamplePlayer<WaveReaderM, WaveMock>;
#endif
} // namespace giada::m