#include "../src/core/channels/waveReader.h"
#include "../src/core/resampler.h"
#include "../src/core/wave.h"
#include "../src/utils/vector.h"
#include <catch2/catch.hpp>
#include <memory>

TEST_CASE("WaveReader")
{
	using namespace giada;

	constexpr int BUFFER_SIZE  = 1024;
	constexpr int NUM_CHANNELS = 2;

	m::Wave wave(0);
	wave.getBuffer().alloc(BUFFER_SIZE, NUM_CHANNELS);
	wave.getBuffer().forEachFrame([](float* f, int i) {
		f[0] = static_cast<float>(i + 1);
		f[1] = static_cast<float>(i + 1);
	});
	m::Resampler  resampler;
	m::WaveReader waveReader(&resampler);

	SECTION("Test initialization")
	{
		REQUIRE(waveReader.wave == nullptr);
	}

	waveReader.wave = &wave;

	SECTION("Test fill, pitch 1.0")
	{
		mcl::AudioBuffer out(BUFFER_SIZE, NUM_CHANNELS);

		SECTION("Regular fill")
		{
			waveReader.fill(out, /*start=*/0, BUFFER_SIZE, /*offset=*/0, /*pitch=*/1.0f);

			bool allFilled = true;
			out.forEachSample([&allFilled](const float& f, int) {
				if (f == 0.0f)
					allFilled = false;
			});

			REQUIRE(allFilled);
		}

		SECTION("Partial fill")
		{
			waveReader.fill(out, /*start=*/0, BUFFER_SIZE, /*offset=*/BUFFER_SIZE / 2, /*pitch=*/1.0f);

			int numFramesFilled = 0;
			out.forEachFrame([&numFramesFilled](const float* f, int) {
				if (f[0] != 0.0f)
					numFramesFilled++;
			});

			REQUIRE(numFramesFilled == BUFFER_SIZE / 2);
			REQUIRE(out[(BUFFER_SIZE / 2) - 1][0] == 0.0f);
			REQUIRE(out[BUFFER_SIZE / 2][0] != 0.0f);
		}
	}
}
