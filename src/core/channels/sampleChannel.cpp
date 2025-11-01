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
, velocityAsVol(false)
{
}

/* -------------------------------------------------------------------------- */

SampleChannel::SampleChannel(const Patch::Channel& p, const SceneArray<Sample>& samples, float samplerateRatio)
: inputMonitor(p.inputMonitor)
, overdubProtection(p.overdubProtection)
, mode(p.mode)
, velocityAsVol(p.midiInVeloAsVol)
{
	std::size_t scene = 0;
	for (const Sample& sample : samples)
		setSample(sample, scene++, samplerateRatio);
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

bool SampleChannel::hasWave(std::size_t scene) const { return m_samples[scene].wave != nullptr; }
bool SampleChannel::hasLogicalWave(std::size_t scene) const { return hasWave(scene) && m_samples[scene].wave->isLogical(); }
bool SampleChannel::hasEditedWave(std::size_t scene) const { return hasWave(scene) && m_samples[scene].wave->isEdited(); }

/* -------------------------------------------------------------------------- */

Wave* SampleChannel::getWave(std::size_t scene) const
{
	return m_samples[scene].wave;
}

ID SampleChannel::getWaveId(std::size_t scene) const
{
	if (hasWave(scene))
		return m_samples[scene].wave->id;
	return 0;
}

/* -------------------------------------------------------------------------- */

SampleRange SampleChannel::getRange(std::size_t scene) const { return m_samples[scene].range; }

/* -------------------------------------------------------------------------- */

Frame SampleChannel::getShift(std::size_t scene) const { return m_samples[scene].shift; }

/* -------------------------------------------------------------------------- */

float SampleChannel::getPitch(std::size_t scene) const { return m_samples[scene].pitch; }

/* -------------------------------------------------------------------------- */

const SceneArray<Sample>& SampleChannel::getSamples() const { return m_samples; }

/* -------------------------------------------------------------------------- */

const Sample& SampleChannel::getSample(std::size_t scene) const
{
	return m_samples[scene];
}

/* -------------------------------------------------------------------------- */

Frame SampleChannel::getWaveSize(std::size_t scene) const
{
	return hasWave(scene) ? m_samples[scene].wave->getBuffer().countFrames() : 0;
}

/* -------------------------------------------------------------------------- */

void SampleChannel::loadSample(const Sample& s, std::size_t scene)
{
	m_samples[scene] = s;

	if (s.wave != nullptr)
	{
		m_samples[scene].shift = s.shift == -1 ? 0 : s.shift;
		m_samples[scene].range = s.range.isValid() ? s.range : SampleRange(0, s.wave->getBuffer().countFrames());
	}
}

/* -------------------------------------------------------------------------- */

void SampleChannel::setWave(Wave* w, std::size_t scene, float samplerateRatio)
{
	m_samples[scene].wave = w;
	if (w != nullptr)
		adjustSampleByRate(scene, samplerateRatio);
}

/* -------------------------------------------------------------------------- */

void SampleChannel::setRange(SampleRange newRange, std::size_t scene)
{
	m_samples[scene].range = newRange;
}

/* -------------------------------------------------------------------------- */

void SampleChannel::setSample(const Sample& sample, std::size_t scene, float samplerateRatio)
{
	m_samples[scene] = sample;
	if (sample.wave != nullptr)
		adjustSampleByRate(scene, samplerateRatio);
}

/* -------------------------------------------------------------------------- */

void SampleChannel::setShift(Frame shift, std::size_t scene)
{
	m_samples[scene].shift = shift;
}

/* -------------------------------------------------------------------------- */

void SampleChannel::setPitch(float pitch, std::size_t scene)
{
	m_samples[scene].pitch = pitch;
}

/* -------------------------------------------------------------------------- */

void SampleChannel::adjustSampleByRate(std::size_t scene, float samplerateRatio)
{
	m_samples[scene].range *= samplerateRatio;
	m_samples[scene].shift *= samplerateRatio;
}
} // namespace giada::m
