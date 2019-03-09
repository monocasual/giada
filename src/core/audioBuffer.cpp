#include <new>
#include <cassert>
#include <cstring>
#include "audioBuffer.h"


namespace giada {
namespace m
{
AudioBuffer::AudioBuffer()
	: m_data    (nullptr),
 	  m_size    (0),
	  m_channels(0)
{
}


/* -------------------------------------------------------------------------- */


AudioBuffer::~AudioBuffer()
{
	free();
}


/* -------------------------------------------------------------------------- */


float* AudioBuffer::operator [](int offset) const
{
	assert(m_data != nullptr);
	assert(offset < m_size);
	return m_data + (offset * m_channels);
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::clear(int a, int b)
{
	if (m_data == nullptr)
		return;
	if (b == -1) b = m_size;
	memset(m_data + (a * m_channels), 0, (b - a) * m_channels * sizeof(float));	
}


/* -------------------------------------------------------------------------- */


int AudioBuffer::countFrames()   const { return m_size; }
int AudioBuffer::countSamples()  const { return m_size * m_channels; }
int AudioBuffer::countChannels() const { return m_channels; }
bool AudioBuffer::isAllocd()     const { return m_data != nullptr; }



/* -------------------------------------------------------------------------- */


void AudioBuffer::alloc(int size, int channels)
{
	free();
	m_size     = size;
	m_channels = channels;
	m_data     = new float[m_size * m_channels];	
	clear(); // does nothing if m_data == nullptr
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::free()
{
	delete[] m_data;  // No check required, delete nullptr does nothing
	setData(nullptr, 0, 0);
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::setData(float* data, int size, int channels)
{
	m_data     = data;
	m_size     = size;
	m_channels = channels;
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::moveData(AudioBuffer& b)
{
	free();
	m_data     = b[0];
	m_size     = b.countFrames();
	m_channels = b.countChannels();
	b.setData(nullptr, 0, 0);
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::copyFrame(int frame, float* values)
{
	assert(m_data != nullptr);
	memcpy(m_data + (frame * m_channels), values, m_channels * sizeof(float));
}


/* -------------------------------------------------------------------------- */

void AudioBuffer::copyData(const float* data, int frames, int offset)
{
	assert(m_data != nullptr);
	assert(frames <= m_size - offset);
	memcpy(m_data + (offset * m_channels), data, frames * m_channels * sizeof(float));
}

}} // giada::m::