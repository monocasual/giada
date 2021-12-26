#ifndef G_TESTS_WAVEREADER_MOCK_H
#define G_TESTS_WAVEREADER_MOCK_H

namespace giada::m
{
class WaveReaderMock
{
public:
	struct Result
	{
		Frame used, generated;
	};

	WaveReaderMock(giada::m::Resampler*) {}
};
} // namespace giada::m

#endif