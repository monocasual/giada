#include "../src/core/waveFactory.h"
#include "../src/core/const.h"
#include "../src/core/resampler.h"
#include "../src/core/wave.h"
#include <catch2/catch.hpp>

using namespace giada::m;

TEST_CASE("waveFactory")
{
	constexpr int  G_SAMPLE_RATE = 44100;
	constexpr int  G_BUFFER_SIZE = 4096;
	constexpr auto TEST_WAV_PATH = TEST_RESOURCES_DIR "test.wav";

	SECTION("test creation")
	{
		waveFactory::Result res = waveFactory::createFromFile(TEST_WAV_PATH,
		    /*ID=*/0, /*sampleRate=*/G_SAMPLE_RATE, Resampler::Quality::LINEAR);

		REQUIRE(res.status == G_RES_OK);
		REQUIRE(res.wave->getRate() == G_SAMPLE_RATE);
		REQUIRE(res.wave->getBuffer().countChannels() == G_CHANNELS);
		REQUIRE(res.wave->isLogical() == false);
		REQUIRE(res.wave->isEdited() == false);
	}

	SECTION("test recording")
	{
		std::unique_ptr<Wave> wave = waveFactory::createEmpty(G_BUFFER_SIZE,
		    G_MAX_IO_CHANS, G_SAMPLE_RATE, "test.wav");

		REQUIRE(wave->getRate() == G_SAMPLE_RATE);
		REQUIRE(wave->getBuffer().countFrames() == G_BUFFER_SIZE);
		REQUIRE(wave->getBuffer().countChannels() == G_CHANNELS);
		REQUIRE(wave->isLogical() == true);
		REQUIRE(wave->isEdited() == false);
	}

	SECTION("test resampling")
	{
		waveFactory::Result res = waveFactory::createFromFile(TEST_WAV_PATH,
		    /*ID=*/0, /*sampleRate=*/G_SAMPLE_RATE, Resampler::Quality::LINEAR);

		int oldSize = res.wave->getBuffer().countFrames();
		waveFactory::resample(*res.wave.get(), Resampler::Quality::LINEAR, G_SAMPLE_RATE * 2);

		REQUIRE(res.wave->getRate() == G_SAMPLE_RATE * 2);
		REQUIRE(res.wave->getBuffer().countFrames() == oldSize * 2);
		REQUIRE(res.wave->getBuffer().countChannels() == G_CHANNELS);
		REQUIRE(res.wave->isLogical() == false);
		REQUIRE(res.wave->isEdited() == false);
	}
}
