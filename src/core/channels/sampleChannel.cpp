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

#include "core/channels/sampleChannel.h"
#include "core/const.h"
#include "core/wave.h"

namespace giada::m
{
SampleChannel::SampleChannel()
: inputMonitor(false)
, overdubProtection(false)
, mode(SamplePlayerMode::SINGLE_BASIC)
, pitch(G_DEFAULT_PITCH)
, shift(0)
, begin(0)
, end(0)
, velocityAsVol(false)
, m_wave(nullptr)
{
}

/* -------------------------------------------------------------------------- */

SampleChannel::SampleChannel(const Patch::Channel& p, Wave* w, float samplerateRatio)
: inputMonitor(p.inputMonitor)
, overdubProtection(p.overdubProtection)
, mode(p.mode)
, pitch(p.pitch)
, shift(p.shift)
, begin(p.begin)
, end(p.end)
, velocityAsVol(p.midiInVeloAsVol)
{
	setWave(w, samplerateRatio);
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

bool SampleChannel::hasWave() const { return m_wave != nullptr; }
bool SampleChannel::hasLogicalWave() const { return hasWave() && m_wave->isLogical(); }
bool SampleChannel::hasEditedWave() const { return hasWave() && m_wave->isEdited(); }

/* -------------------------------------------------------------------------- */

Wave* SampleChannel::getWave() const
{
	return m_wave;
}

ID SampleChannel::getWaveId() const
{
	if (hasWave())
		return m_wave->id;
	return 0;
}

/* -------------------------------------------------------------------------- */

Frame SampleChannel::getWaveSize() const
{
	return hasWave() ? m_wave->getBuffer().countFrames() : 0;
}

/* -------------------------------------------------------------------------- */

void SampleChannel::loadWave(Wave* w, Frame newBegin, Frame newEnd, Frame newShift)
{
	m_wave = w;

	shift = 0;
	begin = 0;
	end   = 0;

	if (w != nullptr)
	{
		shift = newShift == -1 ? 0 : newShift;
		begin = newBegin == -1 ? 0 : newBegin;
		end   = newEnd == -1 ? w->getBuffer().countFrames() : newEnd;
	}
}

/* -------------------------------------------------------------------------- */

void SampleChannel::setWave(Wave* w, float samplerateRatio)
{
	m_wave = w;

	if (m_wave == nullptr)
		return;

	if (samplerateRatio != 1.0f)
	{
		begin *= samplerateRatio;
		end *= samplerateRatio;
		shift *= samplerateRatio;
	}
}

} // namespace giada::m
