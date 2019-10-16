#include <memory>
#include "../src/core/const.h"
#include "../src/core/wave.h"
#include "../src/core/waveFx.h"
#include <catch.hpp>


using std::string;
using namespace giada::m;


TEST_CASE("waveFx")
{
	static const int SAMPLE_RATE = 44100;
	static const int BUFFER_SIZE = 4000;
	static const int BIT_DEPTH = 32;

	Wave waveMono(1);
	Wave waveStereo(1);
	waveMono.alloc(BUFFER_SIZE, 1, SAMPLE_RATE, BIT_DEPTH, "path/to/sample.wav");
	waveStereo.alloc(BUFFER_SIZE, 2, SAMPLE_RATE, BIT_DEPTH, "path/to/sample.wav");

	SECTION("test mono->stereo conversion")
	{
		int prevSize = waveMono.getSize();

		REQUIRE(wfx::monoToStereo(waveMono) == G_RES_OK);
		REQUIRE(waveMono.getSize() == prevSize);  // size does not change, channels do
		REQUIRE(waveMono.getChannels() == 2);

		SECTION("test mono->stereo conversion for already stereo wave")
		{
			/* Should do nothing. */
			int prevSize = waveStereo.getSize();

			REQUIRE(wfx::monoToStereo(waveStereo) == G_RES_OK);
			REQUIRE(waveStereo.getSize() == prevSize);
			REQUIRE(waveStereo.getChannels() == 2);
		}
	}

	SECTION("test silence")
	{
		int a = 20;
		int b = 57;
		wfx::silence(waveStereo.id, a, b);

		for (int i=a; i<b; i++)
			for (int k=0; k<waveStereo.getChannels(); k++)
				REQUIRE(waveStereo[i][k] == 0.0f);

		SECTION("test silence (mono)")
		{
			wfx::silence(waveMono.id, a, b);

			for (int i=a; i<b; i++)
				for (int k=0; k<waveMono.getChannels(); k++)
					REQUIRE(waveMono[i][k] == 0.0f);
		}
	}

	SECTION("test cut")
	{
		int a = 47;
		int b = 210;
		int range = b - a;
		int prevSize = waveStereo.getSize();

		wfx::cut(waveStereo.id, a, b);

		REQUIRE(waveStereo.getSize() == prevSize - range);

		SECTION("test cut (mono)")
		{
			prevSize = waveMono.getSize();
			wfx::cut(waveMono.id, a, b);

			REQUIRE(waveMono.getSize() == prevSize - range);
		}
	}

	SECTION("test trim")
	{
		int a = 47;
		int b = 210;
		int area = b - a;

		wfx::trim(waveStereo.id, a, b);

		REQUIRE(waveStereo.getSize() == area);

		SECTION("test trim (mono)")
		{
			wfx::trim(waveMono.id, a, b);

			REQUIRE(waveMono.getSize() == area);
		}
	}

	SECTION("test fade")
	{
		int a = 47;
		int b = 500;

		wfx::fade(waveStereo.id, a, b, wfx::FADE_IN);
		wfx::fade(waveStereo.id, a, b, wfx::FADE_OUT);

		REQUIRE(waveStereo.getFrame(a)[0] == 0.0f);
		REQUIRE(waveStereo.getFrame(a)[1] == 0.0f);
		REQUIRE(waveStereo.getFrame(b)[0] == 0.0f);
		REQUIRE(waveStereo.getFrame(b)[1] == 0.0f);

		SECTION("test fade (mono)")
		{
			wfx::fade(waveMono.id, a, b, wfx::FADE_IN);
			wfx::fade(waveMono.id, a, b, wfx::FADE_OUT);

			REQUIRE(waveMono.getFrame(a)[0] == 0.0f);
			REQUIRE(waveMono.getFrame(b)[0] == 0.0f);		
		}
	}

	SECTION("test smooth")
	{
		int a = 11;
		int b = 79;

		wfx::smooth(waveStereo.id, a, b);

		REQUIRE(waveStereo.getFrame(a)[0] == 0.0f);
		REQUIRE(waveStereo.getFrame(a)[1] == 0.0f);
		REQUIRE(waveStereo.getFrame(b)[0] == 0.0f);
		REQUIRE(waveStereo.getFrame(b)[1] == 0.0f);
		
		SECTION("test smooth (mono)")
		{
			wfx::smooth(waveMono.id, a, b);
			REQUIRE(waveMono.getFrame(a)[0] == 0.0f);
			REQUIRE(waveMono.getFrame(b)[0] == 0.0f);		
		}
	}
}
