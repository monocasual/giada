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

#include "audioBuffer.h"
#include <algorithm>
#include <cassert>

namespace giada::m
{
AudioBuffer::AudioBuffer()
: m_data(nullptr)
, m_size(0)
, m_channels(0)
, m_viewing(false)
{
}

/* -------------------------------------------------------------------------- */

AudioBuffer::AudioBuffer(Frame size, int channels)
: AudioBuffer()
{
	alloc(size, channels);
}

/* -------------------------------------------------------------------------- */

AudioBuffer::AudioBuffer(float* data, Frame size, int channels)
: m_data(data)
, m_size(size)
, m_channels(channels)
, m_viewing(true)
{
	assert(channels <= NUM_CHANS);
}

/* -------------------------------------------------------------------------- */

AudioBuffer::AudioBuffer(const AudioBuffer& o)
{
	copy(o);
}

/* -------------------------------------------------------------------------- */

AudioBuffer::AudioBuffer(AudioBuffer&& o)
{
	move(std::move(o));
}

/* -------------------------------------------------------------------------- */

AudioBuffer::~AudioBuffer()
{
	if (!m_viewing)
		free();
}

/* -------------------------------------------------------------------------- */

AudioBuffer& AudioBuffer::operator=(const AudioBuffer& o)
{
	if (this == &o)
		return *this;
	copy(o);
	return *this;
}

/* -------------------------------------------------------------------------- */

AudioBuffer& AudioBuffer::operator=(AudioBuffer&& o)
{
	if (this == &o)
		return *this;
	move(std::move(o));
	return *this;
}

/* -------------------------------------------------------------------------- */

float* AudioBuffer::operator[](Frame offset) const
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
	if (b == -1)
		b = m_size;
	std::fill_n(m_data + (a * m_channels), (b - a) * m_channels, 0.0);
}

/* -------------------------------------------------------------------------- */

Frame AudioBuffer::countFrames() const { return m_size; }
int   AudioBuffer::countSamples() const { return m_size * m_channels; }
int   AudioBuffer::countChannels() const { return m_channels; }
bool  AudioBuffer::isAllocd() const { return m_data != nullptr; }

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
	if (m_data == nullptr)
		return;
	delete[] m_data;
	m_data     = nullptr;
	m_size     = 0;
	m_channels = 0;
	m_viewing  = false;
}

/* -------------------------------------------------------------------------- */

void AudioBuffer::sum(const AudioBuffer& b, Frame framesToCopy, Frame srcOffset,
    Frame destOffset, float gain, Pan pan)
{
	copyData<Operation::SUM>(b, framesToCopy, srcOffset, destOffset, gain, pan);
}

void AudioBuffer::set(const AudioBuffer& b, Frame framesToCopy, Frame srcOffset,
    Frame destOffset, float gain, Pan pan)
{
	copyData<Operation::SET>(b, framesToCopy, srcOffset, destOffset, gain, pan);
}

void AudioBuffer::sum(const AudioBuffer& b, float gain, Pan pan)
{
	copyData<Operation::SUM>(b, -1, 0, 0, gain, pan);
}

void AudioBuffer::set(const AudioBuffer& b, float gain, Pan pan)
{
	copyData<Operation::SET>(b, -1, 0, 0, gain, pan);
}

/* -------------------------------------------------------------------------- */

template <AudioBuffer::Operation O>
void AudioBuffer::copyData(const AudioBuffer& b, Frame framesToCopy,
    Frame srcOffset, Frame destOffset, float gain, Pan pan)
{
	const int  srcChannels  = b.countChannels();
	const int  destChannels = countChannels();
	const bool sameChannels = srcChannels == destChannels;

	assert(m_data != nullptr);
	assert(destOffset >= 0 && destOffset < m_size);
	assert(srcChannels <= destChannels);

	/* Make sure the amount of frames to copy lies within the current buffer 
	size. */

	framesToCopy = framesToCopy == -1 ? b.countFrames() : framesToCopy;
	framesToCopy = std::min(framesToCopy, m_size - destOffset);

	/* Case 1) source has less channels than this one: brutally spread source's
	channel 0 over this one (TODO - maybe mixdown source channels first?)
	   Case 2) source has same amount of channels: copy them 1:1. */

	for (int destF = 0, srcF = srcOffset; destF < framesToCopy && destF < b.countFrames(); destF++, srcF++)
	{
		for (int ch = 0; ch < destChannels; ch++)
		{
			if constexpr (O == Operation::SUM)
				sum(destF + destOffset, ch, b[srcF][sameChannels ? ch : 0] * gain * pan[ch]);
			else
				set(destF + destOffset, ch, b[srcF][sameChannels ? ch : 0] * gain * pan[ch]);
		}
	}
}

/* -------------------------------------------------------------------------- */

void AudioBuffer::applyGain(float g)
{
	for (int i = 0; i < countSamples(); i++)
		m_data[i] *= g;
}

/* -------------------------------------------------------------------------- */

void AudioBuffer::sum(Frame f, int channel, float val) { (*this)[f][channel] += val; }
void AudioBuffer::set(Frame f, int channel, float val) { (*this)[f][channel] = val; }

/* -------------------------------------------------------------------------- */

void AudioBuffer::move(AudioBuffer&& o)
{
	assert(o.countChannels() <= NUM_CHANS);

	m_data     = o.m_data;
	m_size     = o.m_size;
	m_channels = o.m_channels;
	m_viewing  = o.m_viewing;

	o.m_data     = nullptr;
	o.m_size     = 0;
	o.m_channels = 0;
	o.m_viewing  = false;
}

/* -------------------------------------------------------------------------- */

void AudioBuffer::copy(const AudioBuffer& o)
{
	m_data     = new float[o.m_size * o.m_channels];
	m_size     = o.m_size;
	m_channels = o.m_channels;
	m_viewing  = o.m_viewing;

	std::copy(o.m_data, o.m_data + (o.m_size * o.m_channels), m_data);
}

/* -------------------------------------------------------------------------- */

template void AudioBuffer::copyData<AudioBuffer::Operation::SUM>(const AudioBuffer&, Frame, Frame, Frame, float, Pan);
template void AudioBuffer::copyData<AudioBuffer::Operation::SET>(const AudioBuffer&, Frame, Frame, Frame, float, Pan);
} // namespace giada::m