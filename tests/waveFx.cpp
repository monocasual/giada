#include <memory>
#include "../src/core/const.h"
#include "../src/core/wave.h"
#include "../src/core/waveFx.h"
#include "catch/single_include/catch.hpp"


using std::string;
using namespace giada::m;


TEST_CASE("Test waveFx")
{
	static const int SAMPLE_RATE = 44100;
	static const int BUFFER_SIZE = 4000;
	static const int BIT_DEPTH = 32;

	std::unique_ptr<Wave> waveMono = std::unique_ptr<Wave>(new Wave(new float[BUFFER_SIZE], 
		BUFFER_SIZE, 1, SAMPLE_RATE, BIT_DEPTH, "path/to/sample.wav"));

	std::unique_ptr<Wave> waveStereo = std::unique_ptr<Wave>(new Wave(new float[BUFFER_SIZE * 2], 
		BUFFER_SIZE, 2, SAMPLE_RATE, BIT_DEPTH, "path/to/sample.wav"));

	SECTION("test mono->stereo conversion")
	{
		int prevSize = waveMono->getSize();

		REQUIRE(wfx::monoToStereo(waveMono.get()) == G_RES_OK);
		REQUIRE(waveMono->getSize() == prevSize);  // size does not change, channels do
		REQUIRE(waveMono->getChannels() == 2);

		SECTION("test mono->stereo conversion for already stereo wave")
		{
			/* Should do nothing. */
			int prevSize = waveStereo->getSize();

			REQUIRE(wfx::monoToStereo(waveStereo.get()) == G_RES_OK);
			REQUIRE(waveStereo->getSize() == prevSize);
			REQUIRE(waveStereo->getChannels() == 2);
		}
	}

	SECTION("test silence")
	{
		int a = 20;
		int b = 57;
		wfx::silence(waveStereo.get(), a, b);

		for (int i=a; i<b; i++) {
			float* frame = waveStereo->getFrame(i);
			for (int k=0; k<waveStereo->getChannels(); k++)
				REQUIRE(frame[k] == 0.0f);
		}

		SECTION("test silence (mono)")
		{
			wfx::silence(waveMono.get(), a, b);

			for (int i=a; i<b; i++) {
				float* frame = waveMono->getFrame(i);
				for (int k=0; k<waveMono->getChannels(); k++)
					REQUIRE(frame[k] == 0.0f);
			}
		}
	}

	SECTION("test cut")
	{
		int a = 47;
		int b = 210;
		int range = b - a;
		int prevSize = waveStereo->getSize();

		REQUIRE(wfx::cut(waveStereo.get(), a, b) == G_RES_OK);
		REQUIRE(waveStereo->getSize() == prevSize - range);

		SECTION("test cut (mono)")
		{
			prevSize = waveMono->getSize();
			REQUIRE(wfx::cut(waveMono.get(), a, b) == G_RES_OK);
			REQUIRE(waveMono->getSize() == prevSize - range);
		}
	}

	SECTION("test trim")
	{
		int a = 47;
		int b = 210;
		int area = b - a;

		REQUIRE(wfx::trim(waveStereo.get(), a, b) == G_RES_OK);
		REQUIRE(waveStereo->getSize() == area);

		SECTION("test trim (mono)")
		{
			REQUIRE(wfx::trim(waveMono.get(), a, b) == G_RES_OK);
			REQUIRE(waveMono->getSize() == area);
		}
	}

	SECTION("test fade")
	{
		int a = 47;
		int b = 210;

		wfx::fade(waveStereo.get(), a, b, wfx::FADE_IN);
		wfx::fade(waveStereo.get(), a, b, wfx::FADE_OUT);

		REQUIRE(waveStereo->getFrame(a)[0] == 0.0f);
		REQUIRE(waveStereo->getFrame(a)[1] == 0.0f);
		REQUIRE(waveStereo->getFrame(b)[0] == 0.0f);
		REQUIRE(waveStereo->getFrame(b)[1] == 0.0f);

		SECTION("test fade (mono)")
		{
			wfx::fade(waveMono.get(), a, b, wfx::FADE_IN);
			wfx::fade(waveMono.get(), a, b, wfx::FADE_OUT);

			REQUIRE(waveMono->getFrame(a)[0] == 0.0f);
			REQUIRE(waveMono->getFrame(b)[0] == 0.0f);		
		}
	}

	SECTION("test smooth")
	{
		int a = 11;
		int b = 79;

		wfx::smooth(waveStereo.get(), a, b);

		REQUIRE(waveStereo->getFrame(a)[0] == 0.0f);
		REQUIRE(waveStereo->getFrame(a)[1] == 0.0f);
		REQUIRE(waveStereo->getFrame(b)[0] == 0.0f);
		REQUIRE(waveStereo->getFrame(b)[1] == 0.0f);
		
		SECTION("test smooth (mono)")
		{
			wfx::smooth(waveMono.get(), a, b);
			REQUIRE(waveMono->getFrame(a)[0] == 0.0f);
			REQUIRE(waveMono->getFrame(b)[0] == 0.0f);		
		}
	}
}
