/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_SAMPLE_CHANNEL_H
#define G_SAMPLE_CHANNEL_H

#include "src/core/patch.h"
#include "src/deps/geompp/src/range.hpp"

namespace giada::m
{
class Wave;
class SampleChannel final
{
public:
	SampleChannel();
	SampleChannel(const Patch::Channel&, Wave*, float samplerateRatio);

	bool  isAnyLoopMode() const;
	bool  isAnyLoopOnceMode() const;
	bool  isAnyNonLoopingSingleMode() const;
	bool  hasWave() const;
	bool  hasLogicalWave() const;
	bool  hasEditedWave() const;
	ID    getWaveId() const;
	Frame getWaveSize() const;
	Wave* getWave() const;

	/* loadWave
	Loads Wave and sets it up (name, markers, ...). Resets begin/end points
	and shift if not specified. */

	void loadWave(Wave*, Frame begin = -1, Frame end = -1, Frame shift = -1);

	/* setWave
	Just sets the pointer to a Wave object. Used during de-serialization. The
	ratio is used to adjust begin/end points in case of patch vs. conf sample
	rate mismatch. If nullptr, set the wave to invalid. */

	void setWave(Wave* w, float samplerateRatio);

	bool                 inputMonitor;
	bool                 overdubProtection;
	SamplePlayerMode     mode;
	float                pitch;
	Frame                shift;
	geompp::Range<Frame> range;
	bool                 velocityAsVol; // Velocity drives volume

private:
	/* wave
	Wave object. Might be null if the channel has no sample. */

	Wave* m_wave;
};
} // namespace giada::m

#endif
