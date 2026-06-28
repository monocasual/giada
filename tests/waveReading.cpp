#include "../src/core/resampler.h"
#include "../src/core/wave.h"
#include "../src/utils/vector.h"
#include "src/core/rendering/sampleRendering.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>

TEST_CASE("WaveReading")
{
	using namespace giada;

	constexpr int BUFFER_SIZE  = 1024;
	constexpr int NUM_CHANNELS = 2;

	m::Wave wave({});
	wave.getBuffer().alloc(BUFFER_SIZE, NUM_CHANNELS);
	for (int i = 0; i < wave.getBuffer().countFrames(); ++i)
	{
		wave.getBuffer().at(i, 0) = static_cast<float>(i + 1);
		wave.getBuffer().at(i, 1) = static_cast<float>(i + 1);
	}
	m::Resampler resampler;

	SECTION("Test fill, pitch 1.0")
	{
		mcl::AudioBuffer out(BUFFER_SIZE, NUM_CHANNELS);

		SECTION("Regular fill")
		{
			m::rendering::ReadResult res = rendering::readWave(wave, out,
			    /*start=*/0, BUFFER_SIZE, /*offset=*/0, /*pitch=*/1.0f, resampler);

			bool allFilled       = true;
			int  numFramesFilled = 0;
			for (int i = 0; i < out.countFrames(); ++i)
			{
				if (out.at(i, 0) == 0.0f)
					allFilled = false;
				else
					numFramesFilled++;
			}

			REQUIRE(allFilled);
			REQUIRE(numFramesFilled == res.used);
			REQUIRE(numFramesFilled == res.generated);
		}

		SECTION("Partial fill")
		{
			m::rendering::ReadResult res = rendering::readWave(wave, out,
			    /*start=*/0, BUFFER_SIZE, /*offset=*/BUFFER_SIZE / 2, /*pitch=*/1.0f, resampler);

			int numFramesFilled = 0;
			for (int i = 0; i < out.countFrames(); ++i)
			{
				if (out.at(i, 0) != 0.0f)
					numFramesFilled++;
			}

			REQUIRE(numFramesFilled == BUFFER_SIZE / 2);
			REQUIRE(out.at((BUFFER_SIZE / 2) - 1, 0) == 0.0f);
			REQUIRE(out.at(BUFFER_SIZE / 2, 0) != 0.0f);
			REQUIRE(numFramesFilled == res.used);
			REQUIRE(numFramesFilled == res.generated);
		}
	}
}
