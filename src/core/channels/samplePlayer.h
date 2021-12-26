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

#ifndef G_CHANNEL_SAMPLE_PLAYER_H
#define G_CHANNEL_SAMPLE_PLAYER_H

#include "core/channels/waveReader.h"
#include "core/const.h"
#include "core/eventDispatcher.h"
#include "core/patch.h"
#include "core/sequencer.h"
#include "core/types.h"
#include <functional>

namespace giada::m
{
class Channel;
template <typename WaveReaderI, typename WaveI>
class SamplePlayer final
{
public:
	SamplePlayer(Resampler* r);
	SamplePlayer(const Patch::Channel& p, float samplerateRatio, Resampler* r, WaveI* w);

	bool   hasWave() const;
	bool   hasLogicalWave() const;
	bool   hasEditedWave() const;
	bool   isAnyLoopMode() const;
	ID     getWaveId() const;
	Frame  getWaveSize() const;
	WaveI* getWave() const;
	void   render(const Channel& ch) const;

	void react(const EventDispatcher::Event& e);

	/* loadWave
	Loads Wave 'w' into channel ch and sets it up (name, markers, ...). */

	void loadWave(Channel& ch, WaveI* w);

	/* setWave
	Just sets the pointer to a Wave object. Used during de-serialization. The
	ratio is used to adjust begin/end points in case of patch vs. conf sample
	rate mismatch. If nullptr, set the wave to invalid. */

	void setWave(WaveI* w, float samplerateRatio);

	/* kickIn
	Starts the player right away at frame 'f'. Used when launching a loop after
	being live recorded. */

	void kickIn(Channel& ch, Frame f);

	float            pitch;
	SamplePlayerMode mode;
	Frame            shift;
	Frame            begin;
	Frame            end;
	bool             velocityAsVol; // Velocity drives volume
	WaveReaderI      waveReader;

	std::function<void()> onLastFrame;

private:
	bool                         isPlaying(const Channel& ch) const;
	typename WaveReaderI::Result fillBuffer(const Channel& ch, Frame start, Frame offset) const;
	bool                         shouldLoop(const Channel& ch) const;
};

/* SamplePlayerC, SamplePlayerM
Aliases for the both concrete (C) and mock (M) SamplePlayer class. */

using SamplePlayerC = SamplePlayer<WaveReaderC, Wave>;
using SamplePlayerM = SamplePlayer<WaveReaderM, WaveMock>;

extern template class SamplePlayer<WaveReaderC, Wave>;
#ifdef WITH_TESTS
extern template class SamplePlayer<WaveReaderM, WaveMock>;
#endif
} // namespace giada::m

#endif
