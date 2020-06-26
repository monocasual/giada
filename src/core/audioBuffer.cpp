/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include <algorithm>
#include "audioBuffer.h"


namespace giada {
namespace m
{
AudioBuffer::AudioBuffer()
: m_data    (nullptr)
, m_size    (0)
, m_channels(0)
{
}


AudioBuffer::AudioBuffer(Frame size, int channels)
: AudioBuffer()
{
	alloc(size, channels);
}


/* -------------------------------------------------------------------------- */


AudioBuffer::AudioBuffer(const AudioBuffer& o)
: m_data    (new float[o.m_size * o.m_channels])
, m_size    (o.m_size)
, m_channels(o.m_channels)
{
	std::copy(o.m_data, o.m_data + (o.m_size * o.m_channels), m_data); 
}


/* -------------------------------------------------------------------------- */


AudioBuffer::~AudioBuffer()
{
	free();
}


/* -------------------------------------------------------------------------- */


float* AudioBuffer::operator [](Frame offset) const
{
	assert(m_data != nullptr);
	assert(offset < m_size);
	return m_data + (offset * m_channels);
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::clear(Frame a, Frame b)
{
	if (m_data == nullptr)
		return;
	if (b == -1) b = m_size;
	std::fill_n(m_data + (a * m_channels), (b - a) * m_channels, 0.0);
}


/* -------------------------------------------------------------------------- */


Frame AudioBuffer::countFrames()   const { return m_size; }
int   AudioBuffer::countSamples()  const { return m_size * m_channels; }
int   AudioBuffer::countChannels() const { return m_channels; }
bool  AudioBuffer::isAllocd()      const { return m_data != nullptr; }



/* -------------------------------------------------------------------------- */


float AudioBuffer::getPeak() const
{
	float peak = 0.0f;
	for (int i = 0; i < countSamples(); i++)
		peak = std::max(peak, m_data[i]);
	return peak;
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::alloc(Frame size, int channels)
{
	assert(channels <= NUM_CHANS);

	free();
	m_size     = size;
	m_channels = channels;
	m_data     = new float[m_size * m_channels];	
	clear();
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::free()
{
	delete[] m_data;
	setData(nullptr, 0, 0);
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::setData(float* data, Frame size, int channels)
{
	assert(channels <= NUM_CHANS);

	m_data     = data;
	m_size     = size;
	m_channels = channels;
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::moveData(AudioBuffer& b)
{
	assert(b.countChannels() <= NUM_CHANS);

	free();
	m_data     = b[0];
	m_size     = b.countFrames();
	m_channels = b.countChannels();
	b.setData(nullptr, 0, 0);
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::copyData(const float* data, Frame frames, int channels, int offset)
{
	assert(m_data != nullptr);
	assert(frames <= m_size - offset);

	if (channels < NUM_CHANS) // i.e. one channel, mono
		for (int i = offset, k = 0; i < m_size; i++, k++)
			for (int j = 0; j < countChannels(); j++)
				(*this)[i][j] = data[k];
	else
	if (channels == NUM_CHANS)
		std::copy_n(data, frames * channels, m_data + (offset * channels));
	else
		assert(false);
}


void AudioBuffer::copyData(const AudioBuffer& b, float gain)
{
	copyData(b[0], b.countFrames(), b.countChannels());
	if (gain != 1.0f)
		applyGain(gain);
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::addData(const AudioBuffer& b, float gain, Pan pan)
{
	assert(m_data != nullptr);
	assert(countFrames() <= b.countFrames());
	assert(b.countChannels() <= NUM_CHANS);

	for (int i = 0; i < countFrames(); i++)
		for (int j = 0; j < countChannels(); j++)
			(*this)[i][j] += b[i][j] * gain * pan[j];
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::applyGain(float g)
{
	for (int i = 0; i < countSamples(); i++)
		m_data[i] *= g;
}
}} // giada::m::