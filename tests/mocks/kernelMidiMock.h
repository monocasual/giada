#ifndef G_TESTS_KERNELMIDI_MOCK_H
#define G_TESTS_KERNELMIDI_MOCK_H

#include "../../src/core/midiEvent.h"

namespace giada::m
{
class KernelMidiMock
{
public:
	void send(const MidiEvent& e)
	{
		sent.push_back(e);
	}

	std::vector<MidiEvent> sent;
};
} // namespace giada::m

#endif