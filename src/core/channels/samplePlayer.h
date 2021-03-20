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

#include "core/channels/sampleAdvancer.h"
#include "core/channels/sampleReactor.h"
#include "core/channels/waveReader.h"
#include "core/const.h"
#include "core/types.h"

namespace giada::m::channel
{
struct Data;
}
namespace giada::m::patch
{
struct Channel;
}
namespace giada::m::samplePlayer
{
struct Data
{
	Data();
	Data(const patch::Channel& p, float samplerateRatio);
	Data(const Data& o) = default;
	Data(Data&& o)      = default;
	Data& operator=(const Data&) = default;
	Data& operator=(Data&&) = default;

	bool  hasWave() const;
	bool  hasLogicalWave() const;
	bool  hasEditedWave() const;
	bool  isAnyLoopMode() const;
	ID    getWaveId() const;
	Frame getWaveSize() const;
	Wave* getWave() const;

	float            pitch;
	SamplePlayerMode mode;
	Frame            shift;
	Frame            begin;
	Frame            end;
	bool             velocityAsVol; // Velocity drives volume
	WaveReader       waveReader;
};

void react(channel::Data& ch, const eventDispatcher::Event& e);
void advance(const channel::Data& ch, const sequencer::Event& e);
void render(const channel::Data& ch);

/* loadWave
Loads Wave 'w' into channel ch and sets it up (name, markers, ...). */

void loadWave(channel::Data& ch, Wave* w);

/* setWave
Just sets the pointer to a Wave object. Used during de-serialization. The
ratio is used to adjust begin/end points in case of patch vs. conf sample
rate mismatch. If nullptr, set the wave to invalid. */

void setWave(channel::Data& ch, Wave* w, float samplerateRatio);

/* kickIn
Starts the player right away at frame 'f'. Used when launching a loop after
being live recorded. */

void kickIn(channel::Data& ch, Frame f);
} // namespace giada::m::samplePlayer

#endif
