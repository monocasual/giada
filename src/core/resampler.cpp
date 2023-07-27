/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/resampler.h"
#include <algorithm>
#include <cassert>
#include <new>
#include <utility>

#include <cstdio>

namespace giada::m
{
Resampler::Resampler()
: m_state(nullptr)
, m_input(nullptr)
, m_inputPos(0)
, m_inputLength(0)
, m_channels(0)
, m_usedFrames(0)
{
}

/* -------------------------------------------------------------------------- */

Resampler::Resampler(Quality quality, int channels)
: Resampler()
{
	alloc(quality, channels);
}

/* -------------------------------------------------------------------------- */

Resampler::Resampler(const Resampler& o)
: Resampler()
{
	*this = o;
}

/* -------------------------------------------------------------------------- */

/* This is a fake move constructor that makes a copy instead. The SRC_STATE
object has a callback that, if moved, would still point to the original object.
TODO: maybe delete the move constructor? */

Resampler::Resampler(Resampler&& o)
: Resampler()
{
	*this = o;
}

/* -------------------------------------------------------------------------- */

Resampler& Resampler::operator=(const Resampler& o)
{
	if (this == &o)
		return *this;
	alloc(o.m_quality, o.m_channels);
	return *this;
}

/* -------------------------------------------------------------------------- */

/* This is a fake move operator: see notes above. */

Resampler& Resampler::operator=(Resampler&& o)
{
	if (this == &o)
		return *this;
	alloc(o.m_quality, o.m_channels);
	return *this;
}

/* -------------------------------------------------------------------------- */

Resampler::~Resampler()
{
	src_delete(m_state);
}

/* -------------------------------------------------------------------------- */

long Resampler::callback(void* self, float** audio)
{
	return static_cast<Resampler*>(self)->callback(audio);
}

/* -------------------------------------------------------------------------- */

long Resampler::callback(float** audio)
{
	assert(audio != nullptr);

	/* Move pointer properly, taking into account read data and number of 
	channels in input data. */

	*audio = m_input + (m_inputPos * m_channels);

	/* Returns how many frames have been read in this callback shot. */

	long frames;

	/* Read in CHUNK_LEN parts, checking if there is enough data left. */

	if (m_inputPos + CHUNK_LEN < m_inputLength)
		frames = CHUNK_LEN;
	else
		frames = m_inputLength - m_inputPos;

	m_usedFrames += frames;
	m_inputPos += frames;

	return frames;
}

/* -------------------------------------------------------------------------- */

void Resampler::alloc(Quality quality, int channels)
{
	if (m_state != nullptr)
		src_delete(m_state);
	m_state    = src_callback_new(callback, static_cast<int>(quality), channels, nullptr, this);
	m_quality  = quality;
	m_channels = channels;
	if (m_state == nullptr)
		throw std::bad_alloc();
	src_reset(m_state);
}

/* -------------------------------------------------------------------------- */

Resampler::Result Resampler::process(float* input, long inputPos, long inputLength,
    float* output, long outputLength, float ratio) const
{
	assert(m_state != nullptr); // Must be initialized first!

	m_input       = input;
	m_inputPos    = inputPos;
	m_inputLength = inputLength;
	m_usedFrames  = 0;

	long generated = src_callback_read(m_state, 1 / ratio, outputLength, output);

	return {m_usedFrames, generated};
}

/* -------------------------------------------------------------------------- */

void Resampler::last() const
{
	src_reset(m_state);
}
} // namespace giada::m
