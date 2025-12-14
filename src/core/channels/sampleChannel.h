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
	SampleChannel(const Patch::Channel&, const SceneArray<Sample>&, float samplerateRatio);

	bool        isAnyLoopMode() const;
	bool        isAnyLoopOnceMode() const;
	bool        isAnyNonLoopingSingleMode() const;
	bool        hasWave(Scene) const;
	bool        hasLogicalWave(Scene) const;
	bool        hasEditedWave(Scene) const;
	ID          getWaveId(Scene) const;
	Frame       getWaveSize(Scene) const;
	Wave*       getWave(Scene) const;
	SampleRange getRange(Scene) const;
	Frame       getShift(Scene) const;
	float       getPitch(Scene) const;

	const SceneArray<Sample>& getSamples() const;
	const Sample&             getSample(Scene) const;

	/* loadSample
	Loads Wave and sets it up (name, markers, ...). Resets begin/end points
	and shift if not specified (-1). */

	void loadSample(const Sample&, Scene);

	/* setWave
	Just sets the pointer to a Wave object. Used during de-serialization. The
	ratio is used to adjust begin/end points in case of patch vs. conf sample
	rate mismatch. If nullptr, set the wave to invalid. */

	void setWave(Wave* w, Scene, float samplerateRatio);

	void setRange(SampleRange, Scene);
	void setSample(const Sample&, Scene, float samplerateRatio);
	void setShift(Frame, Scene);
	void setPitch(float, Scene);

	bool             inputMonitor;
	bool             overdubProtection;
	SamplePlayerMode mode;
	bool             velocityAsVol; // Velocity drives volume

private:
	/* adjustSampleByRate
	Adjusts all frame-based properties (range, shift, ...) by a certain samplerateRatio. */

	void adjustSampleByRate(Scene, float samplerateRatio);

	/* m_samples
	Array of Samples, one per scene. */

	SceneArray<Sample> m_samples;
};
} // namespace giada::m

#endif
