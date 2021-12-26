#ifndef G_TESTS_WAVE_MOCK_H
#define G_TESTS_WAVE_MOCK_H

#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"

namespace giada::m
{
class WaveMock
{
public:
	WaveMock(int bufferSize, int channels)
	: m_buffer(bufferSize, channels)
	{
		// [1..1025]
		for (int i = 0; i < m_buffer.countFrames(); i++)
			for (int j = 0; j < m_buffer.countChannels(); j++)
				m_buffer[i][j] = static_cast<float>(i + 1);
	}

	mcl::AudioBuffer&       getBuffer() { return m_buffer; }
	const mcl::AudioBuffer& getBuffer() const { return m_buffer; }

	bool isLogical() const { return true; }
	bool isEdited() const { return false; }

	std::string getBasename(bool /*ext*/) const { return ""; }

	ID id = 0;

private:
	mcl::AudioBuffer m_buffer;
};
} // namespace giada::m

#endif