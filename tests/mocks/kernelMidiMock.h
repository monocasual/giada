#ifndef G_TESTS_KERNELMIDI_MOCK_H
#define G_TESTS_KERNELMIDI_MOCK_H

namespace giada::m
{
class KernelMidiMock
{
public:
	void send(uint32_t s)
	{
		sent.push_back(s);
	}

	std::vector<uint32_t> sent;
};
} // namespace giada::m

#endif