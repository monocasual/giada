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

#include "src/core/channels/sampleChannel.h"
#include "src/core/const.h"
#include "src/core/wave.h"

namespace giada::m
{
SampleChannel::SampleChannel()
: inputMonitor(false)
, overdubProtection(false)
, mode(SamplePlayerMode::SINGLE_BASIC)
, pitch(G_DEFAULT_PITCH)
, shift(0)
, velocityAsVol(false)
, m_waves{} // all nullptr by default
{
}

/* -------------------------------------------------------------------------- */

SampleChannel::SampleChannel(const Patch::Channel& p, Wave* w, float samplerateRatio)
: inputMonitor(p.inputMonitor)
, overdubProtection(p.overdubProtection)
, mode(p.mode)
, pitch(p.pitch)
, shift(p.shift)
, velocityAsVol(p.midiInVeloAsVol)
, m_range(p.range)
{
	setWave(w, 0, samplerateRatio);
}

/* -------------------------------------------------------------------------- */

bool SampleChannel::isAnyLoopMode() const
{
	return mode == SamplePlayerMode::LOOP_BASIC ||
	       mode == SamplePlayerMode::LOOP_ONCE ||
	       mode == SamplePlayerMode::LOOP_REPEAT ||
	       mode == SamplePlayerMode::LOOP_ONCE_BAR;
}

/* -------------------------------------------------------------------------- */

bool SampleChannel::isAnyLoopOnceMode() const
{
	return mode == SamplePlayerMode::LOOP_ONCE ||
	       mode == SamplePlayerMode::LOOP_ONCE_BAR;
}

/* -------------------------------------------------------------------------- */

bool SampleChannel::isAnyNonLoopingSingleMode() const
{
	return mode == SamplePlayerMode::SINGLE_BASIC ||
	       mode == SamplePlayerMode::SINGLE_BASIC_PAUSE ||
	       mode == SamplePlayerMode::SINGLE_PRESS ||
	       mode == SamplePlayerMode::SINGLE_RETRIG;
}

/* -------------------------------------------------------------------------- */

bool SampleChannel::hasWave(std::size_t scene) const { return m_waves[scene] != nullptr; }
bool SampleChannel::hasLogicalWave(std::size_t scene) const { return hasWave(scene) && m_waves[scene]->isLogical(); }
bool SampleChannel::hasEditedWave(std::size_t scene) const { return hasWave(scene) && m_waves[scene]->isEdited(); }

/* -------------------------------------------------------------------------- */

Wave* SampleChannel::getWave(std::size_t scene) const
{
	return m_waves[scene];
}

ID SampleChannel::getWaveId(std::size_t scene) const
{
	if (hasWave(scene))
		return m_waves[scene]->id;
	return 0;
}

/* -------------------------------------------------------------------------- */

SampleRange SampleChannel::getRange() const { return m_range; }

/* -------------------------------------------------------------------------- */

Frame SampleChannel::getWaveSize(std::size_t scene) const
{
	return hasWave(scene) ? m_waves[scene]->getBuffer().countFrames() : 0;
}

/* -------------------------------------------------------------------------- */

void SampleChannel::loadWave(Wave* w, std::size_t scene, SampleRange newRange, Frame newShift)
{
	m_waves[scene] = w;

	shift   = 0;
	m_range = {};

	if (w != nullptr)
	{
		shift   = newShift == -1 ? 0 : newShift;
		m_range = newRange.isValid() ? newRange : SampleRange(0, w->getBuffer().countFrames());
	}
}

/* -------------------------------------------------------------------------- */

void SampleChannel::setWave(Wave* w, std::size_t scene, float samplerateRatio)
{
	m_waves[scene] = w;

	if (w == nullptr)
		return;

	if (samplerateRatio != 1.0f)
	{
		m_range *= samplerateRatio;
		shift *= samplerateRatio;
	}
}

/* -------------------------------------------------------------------------- */

void SampleChannel::setRange(SampleRange newRange)
{
	m_range = newRange;
}
} // namespace giada::m
