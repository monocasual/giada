/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/resampler.h"
#include "const.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <new>
#include <utility>

namespace giada::m
{
Resampler::MonoResampler::MonoResampler()
: m_state(nullptr)
, m_input(nullptr)
, m_inputPos(0)
, m_inputLength(0)
, m_usedFrames(0)
{
}

/* -------------------------------------------------------------------------- */

Resampler::MonoResampler::MonoResampler(Resampler::Quality quality)
: MonoResampler()
{
	alloc(quality);
}

/* -------------------------------------------------------------------------- */

Resampler::MonoResampler::~MonoResampler()
{
	src_delete(m_state);
}

/* -------------------------------------------------------------------------- */

long Resampler::MonoResampler::callback(void* self, float** audio)
{
	return static_cast<MonoResampler*>(self)->callback(audio);
}

/* -------------------------------------------------------------------------- */

long Resampler::MonoResampler::callback(float** audio)
{
	assert(audio != nullptr);

	/* Move pointer properly, taking into account read data and number of
	channels in input data. */

	*audio = m_input + m_inputPos;

	/* Returns how many frames have been read in this callback shot. */

	std::size_t frames = 0;

	/* Read in CHUNK_LEN parts, checking if there is enough data left. */

	if (m_inputPos + CHUNK_LEN < m_inputLength)
		frames = CHUNK_LEN;
	else
		frames = m_inputLength - m_inputPos;

	m_usedFrames += frames;
	m_inputPos += frames;

	return static_cast<long>(frames);
}

/* -------------------------------------------------------------------------- */

void Resampler::MonoResampler::alloc(Quality quality)
{
	if (m_state != nullptr)
		src_delete(m_state);
	m_state   = src_callback_new(callback, static_cast<int>(quality), /*channels=*/1, nullptr, this);
	m_quality = quality;
	if (m_state == nullptr)
		throw std::bad_alloc();
	src_reset(m_state);
}

/* -------------------------------------------------------------------------- */

Resampler::Result Resampler::MonoResampler::process(const float* input, std::size_t inputPos, std::size_t inputLength,
    float* output, std::size_t outputLength, float ratio) const
{
	assert(m_state != nullptr); // Must be initialized first!

	m_input       = const_cast<float*>(input);
	m_inputPos    = inputPos;
	m_inputLength = inputLength;
	m_usedFrames  = 0;

	std::size_t generated = src_callback_read(m_state, 1 / ratio, outputLength, output);

	return {
	    .used      = m_usedFrames,
	    .generated = generated};
}

/* -------------------------------------------------------------------------- */

void Resampler::MonoResampler::last() const
{
	src_reset(m_state);
}
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Resampler::Resampler()
{
}

/* -------------------------------------------------------------------------- */

Resampler::Resampler(Quality quality)
: Resampler()
{
	m_left.alloc(quality);
	m_right.alloc(quality);
}

/* -------------------------------------------------------------------------- */

Resampler::Result Resampler::process(
    const mcl::AudioBuffer& input,
    std::size_t             inputStart,
    std::size_t             inputEnd,
    mcl::AudioBuffer&       output,
    std::size_t             outputStart,
    float                   ratio) const
{
	assert(input.countChannels() == G_MAX_IO_CHANS);
	assert(output.countChannels() == G_MAX_IO_CHANS);
	assert(inputStart <= inputEnd);
	assert(inputEnd <= static_cast<std::size_t>(input.countFrames()));
	assert(outputStart < static_cast<std::size_t>(output.countFrames()));

	const float*      inputLeftPtr   = input.getChannelView(0, 0, inputEnd).data();
	const float*      inputRightPtr  = input.getChannelView(1, 0, inputEnd).data();
	float*            outputLeftPtr  = output.getChannelView(0, outputStart).data();
	float*            outputRightPtr = output.getChannelView(1, outputStart).data();
	const std::size_t outputLength   = static_cast<std::size_t>(output.countFrames()) - outputStart;

	const Result left  = m_left.process(inputLeftPtr, inputStart, inputEnd, outputLeftPtr, outputLength, ratio);
	const Result right = m_right.process(inputRightPtr, inputStart, inputEnd, outputRightPtr, outputLength, ratio);

	return {
	    .used      = std::min(left.used, right.used),
	    .generated = std::min(left.generated, right.generated)};
}

/* -------------------------------------------------------------------------- */

void Resampler::last() const
{
	m_left.last();
	m_right.last();
}
} // namespace giada::m
