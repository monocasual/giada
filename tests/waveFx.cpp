#include "../src/core/waveFx.h"
#include "../src/core/const.h"
#include "../src/core/types.h"
#include "../src/core/wave.h"
#include <catch2/catch.hpp>
#include <memory>

using namespace giada;
using namespace giada::m;

TEST_CASE("waveFx")
{
	static const int SAMPLE_RATE = 44100;
	static const int BUFFER_SIZE = 4000;
	static const int BIT_DEPTH   = 32;

	Wave waveMono(0);
	Wave waveStereo(0);

	waveMono.alloc(BUFFER_SIZE, 1, SAMPLE_RATE, BIT_DEPTH, "path/to/sample-mono.wav");
	waveStereo.alloc(BUFFER_SIZE, 2, SAMPLE_RATE, BIT_DEPTH, "path/to/sample-stereo.wav");

	SECTION("test mono->stereo conversion")
	{
		int prevSize = waveMono.getBuffer().countFrames();

		REQUIRE(wfx::monoToStereo(waveMono) == G_RES_OK);
		REQUIRE(waveMono.getBuffer().countFrames() == prevSize); // size does not change, channels do
		REQUIRE(waveMono.getBuffer().countChannels() == 2);

		SECTION("test mono->stereo conversion for already stereo wave")
		{
			/* Should do nothing. */
			int prevSize = waveStereo.getBuffer().countFrames();

			REQUIRE(wfx::monoToStereo(waveStereo) == G_RES_OK);
			REQUIRE(waveStereo.getBuffer().countFrames() == prevSize);
			REQUIRE(waveStereo.getBuffer().countChannels() == 2);
		}
	}

	SECTION("test silence")
	{
		int a = 20;
		int b = 57;
		wfx::silence(waveStereo, a, b);

		for (int i = a; i < b; i++)
			for (int k = 0; k < waveStereo.getBuffer().countChannels(); k++)
				REQUIRE(waveStereo.getBuffer()[i][k] == 0.0f);
	}

	SECTION("test cut")
	{
		int a        = 47;
		int b        = 210;
		int range    = b - a;
		int prevSize = waveStereo.getBuffer().countFrames();

		wfx::cut(waveStereo, a, b);

		REQUIRE(waveStereo.getBuffer().countFrames() == prevSize - range);
	}

	SECTION("test trim")
	{
		int a    = 47;
		int b    = 210;
		int area = b - a;

		wfx::trim(waveStereo, a, b);

		REQUIRE(waveStereo.getBuffer().countFrames() == area);
	}

	SECTION("test fade")
	{
		int a = 47;
		int b = 500;

		wfx::fade(waveStereo, a, b, wfx::Fade::IN);
		wfx::fade(waveStereo, a, b, wfx::Fade::OUT);

		REQUIRE(waveStereo.getBuffer()[a][0] == 0.0f);
		REQUIRE(waveStereo.getBuffer()[a][1] == 0.0f);
		REQUIRE(waveStereo.getBuffer()[b][0] == 0.0f);
		REQUIRE(waveStereo.getBuffer()[b][1] == 0.0f);
	}

	SECTION("test smooth")
	{
		int a = 11;
		int b = 79;

		wfx::smooth(waveStereo, a, b);

		REQUIRE(waveStereo.getBuffer()[a][0] == 0.0f);
		REQUIRE(waveStereo.getBuffer()[a][1] == 0.0f);
		REQUIRE(waveStereo.getBuffer()[b][0] == 0.0f);
		REQUIRE(waveStereo.getBuffer()[b][1] == 0.0f);
	}
}
