#include <memory>
#include "../src/core/wave.h"
#include <catch.hpp>


TEST_CASE("Wave")
{
	using namespace giada;

	static const int SAMPLE_RATE = 44100;
	static const int BUFFER_SIZE = 4096;
	static const int CHANNELS = 2;
	static const int BIT_DEPTH = 32;

	/* Each SECTION the TEST_CASE is executed from the start. Any code between 
	this comment and the first SECTION macro is executed before each SECTION. */


	SECTION("test allocation")
	{
		m::Wave wave(1);
		wave.alloc(BUFFER_SIZE, CHANNELS, SAMPLE_RATE, BIT_DEPTH, "path/to/sample.wav");

		SECTION("test basename")
		{
			REQUIRE(wave.getPath() == "path/to/sample.wav");
			REQUIRE(wave.getBasename() == "sample");
			REQUIRE(wave.getBasename(true) == "sample.wav");
		}

		SECTION("test path")
		{
			wave.setPath("path/is/now/different.mp3");

			REQUIRE(wave.getPath() == "path/is/now/different.mp3");

			wave.setPath("path/is/now/different.mp3", 5);

			REQUIRE(wave.getPath() == "path/is/now/different-5.mp3");
		}  

		SECTION("test change name")
		{
			REQUIRE(wave.getPath() == "path/to/sample.wav");
			REQUIRE(wave.getBasename() == "sample");
			REQUIRE(wave.getBasename(true) == "sample.wav");
		}
	}
}
