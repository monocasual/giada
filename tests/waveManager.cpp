#include <memory>
#include <samplerate.h>
#include "../src/core/waveManager.h"
#include "../src/core/wave.h"
#include "../src/core/const.h"
#include <catch2/catch.hpp>


using std::string;
using namespace giada::m;


#define G_SAMPLE_RATE 44100
#define G_BUFFER_SIZE 4096
#define G_CHANNELS 2


TEST_CASE("waveManager")
{
	/* Each SECTION the TEST_CASE is executed from the start. Any code between 
	this comment and the first SECTION macro is exectuted before each SECTION. */

	SECTION("test creation")
	{
		waveManager::Result res = waveManager::createFromFile(TEST_RESOURCES_DIR "test.wav",
			/*ID=*/0, /*sampleRate=*/G_SAMPLE_RATE, /*quality=*/SRC_LINEAR);

		REQUIRE(res.status == G_RES_OK);
		REQUIRE(res.wave->getRate() == G_SAMPLE_RATE);
		REQUIRE(res.wave->getChannels() == G_CHANNELS);
		REQUIRE(res.wave->isLogical() == false);
		REQUIRE(res.wave->isEdited() == false);
	}

	SECTION("test recording")
	{
		std::unique_ptr<Wave> wave = waveManager::createEmpty(G_BUFFER_SIZE, 
			G_MAX_IO_CHANS, G_SAMPLE_RATE, "test.wav");

		REQUIRE(wave->getRate() == G_SAMPLE_RATE);
		REQUIRE(wave->getSize() == G_BUFFER_SIZE);
		REQUIRE(wave->getChannels() == G_CHANNELS);
		REQUIRE(wave->isLogical() == true);
		REQUIRE(wave->isEdited() == false);
	}

	SECTION("test resampling")
	{
		waveManager::Result res = waveManager::createFromFile(TEST_RESOURCES_DIR "test.wav",
			/*ID=*/0, /*sampleRate=*/G_SAMPLE_RATE, /*quality=*/SRC_LINEAR);

		int oldSize = res.wave->getSize();
		waveManager::resample(*res.wave.get(), 1, G_SAMPLE_RATE * 2);
		
		REQUIRE(res.wave->getRate() == G_SAMPLE_RATE * 2);
		REQUIRE(res.wave->getSize() == oldSize * 2);
		REQUIRE(res.wave->getChannels() == G_CHANNELS);
		REQUIRE(res.wave->isLogical() == false);
		REQUIRE(res.wave->isEdited() == false);
	}
}
