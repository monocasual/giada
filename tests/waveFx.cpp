#include <memory>
#include "../src/core/model/model.h"
#include "../src/core/const.h"
#include "../src/core/wave.h"
#include "../src/core/waveFx.h"
#include "../src/core/types.h"
#include <catch2/catch.hpp>


using namespace giada;
using namespace giada::m;


Wave& getWave(ID id)
{
	model::WavesLock l(model::waves);
	return model::get(model::waves, id);
}


TEST_CASE("waveFx")
{
	static const ID  WAVE_MONO_ID   = 1;
	static const ID  WAVE_STEREO_ID = 2;
	static const int SAMPLE_RATE    = 44100;
	static const int BUFFER_SIZE    = 4000;
	static const int BIT_DEPTH      = 32;

	std::unique_ptr<Wave> waveMono   = std::make_unique<Wave>(WAVE_MONO_ID);
	std::unique_ptr<Wave> waveStereo = std::make_unique<Wave>(WAVE_STEREO_ID);

	waveMono->alloc(BUFFER_SIZE, 1, SAMPLE_RATE, BIT_DEPTH, "path/to/sample-mono.wav");
	waveStereo->alloc(BUFFER_SIZE, 2, SAMPLE_RATE, BIT_DEPTH, "path/to/sample-stereo.wav");
	
	model::waves.clear();
	model::waves.push(std::move(waveMono));
	model::waves.push(std::move(waveStereo));

	SECTION("test mono->stereo conversion")
	{
		int prevSize = getWave(WAVE_MONO_ID).getSize();

		REQUIRE(wfx::monoToStereo(getWave(WAVE_MONO_ID)) == G_RES_OK);
		REQUIRE(getWave(WAVE_MONO_ID).getSize() == prevSize);  // size does not change, channels do
		REQUIRE(getWave(WAVE_MONO_ID).getChannels() == 2);

		SECTION("test mono->stereo conversion for already stereo wave")
		{
			/* Should do nothing. */
			int prevSize = getWave(WAVE_STEREO_ID).getSize();

			REQUIRE(wfx::monoToStereo(getWave(WAVE_STEREO_ID)) == G_RES_OK);
			REQUIRE(getWave(WAVE_STEREO_ID).getSize() == prevSize);
			REQUIRE(getWave(WAVE_STEREO_ID).getChannels() == 2);
		}
	}

	SECTION("test silence")
	{
		int a = 20;
		int b = 57;
		wfx::silence(getWave(WAVE_STEREO_ID).id, a, b);

		for (int i=a; i<b; i++)
			for (int k=0; k<getWave(WAVE_STEREO_ID).getChannels(); k++)
				REQUIRE(getWave(WAVE_STEREO_ID)[i][k] == 0.0f);
	}

	SECTION("test cut")
	{
		int a = 47;
		int b = 210;
		int range = b - a;
		int prevSize = getWave(WAVE_STEREO_ID).getSize();

		wfx::cut(getWave(WAVE_STEREO_ID).id, a, b);

		REQUIRE(getWave(WAVE_STEREO_ID).getSize() == prevSize - range);
	}

	SECTION("test trim")
	{
		int a = 47;
		int b = 210;
		int area = b - a;

		wfx::trim(getWave(WAVE_STEREO_ID).id, a, b);

		REQUIRE(getWave(WAVE_STEREO_ID).getSize() == area);
	}

	SECTION("test fade")
	{
		int a = 47;
		int b = 500;

		wfx::fade(getWave(WAVE_STEREO_ID).id, a, b, wfx::Fade::IN);
		wfx::fade(getWave(WAVE_STEREO_ID).id, a, b, wfx::Fade::OUT);

		REQUIRE(getWave(WAVE_STEREO_ID).getFrame(a)[0] == 0.0f);
		REQUIRE(getWave(WAVE_STEREO_ID).getFrame(a)[1] == 0.0f);
		REQUIRE(getWave(WAVE_STEREO_ID).getFrame(b)[0] == 0.0f);
		REQUIRE(getWave(WAVE_STEREO_ID).getFrame(b)[1] == 0.0f);
	}

	SECTION("test smooth")
	{
		int a = 11;
		int b = 79;

		wfx::smooth(getWave(WAVE_STEREO_ID).id, a, b);

		REQUIRE(getWave(WAVE_STEREO_ID).getFrame(a)[0] == 0.0f);
		REQUIRE(getWave(WAVE_STEREO_ID).getFrame(a)[1] == 0.0f);
		REQUIRE(getWave(WAVE_STEREO_ID).getFrame(b)[0] == 0.0f);
		REQUIRE(getWave(WAVE_STEREO_ID).getFrame(b)[1] == 0.0f);
	}
}
