#include <memory>
#include "../src/core/wave.h"
#include "catch/single_include/catch.hpp"


using std::string;


TEST_CASE("Test Wave class")
{
	static const int SAMPLE_RATE = 44100;
	static const int BUFFER_SIZE = 4096;
	static const int CHANNELS = 2;
	static const int BIT_DEPTH = 32;

	/* Each SECTION the TEST_CASE is executed from the start. Any code between 
	this comment and the first SECTION macro is exectuted before each SECTION. */

	std::unique_ptr<Wave> wave;
	
	SECTION("test basename")
	{
		wave = std::unique_ptr<Wave>(new Wave(nullptr, BUFFER_SIZE, CHANNELS, 
			SAMPLE_RATE, BIT_DEPTH, "path/to/sample.wav"));

		REQUIRE(wave->getPath() == "path/to/sample.wav");
		REQUIRE(wave->getBasename() == "sample");
		REQUIRE(wave->getBasename(true) == "sample.wav");
	}  

	SECTION("test change name")
	{
		wave = std::unique_ptr<Wave>(new Wave(nullptr, BUFFER_SIZE, CHANNELS, 
			SAMPLE_RATE, BIT_DEPTH, "path/to/sample.wav"));

		REQUIRE(wave->getPath() == "path/to/waveform.wav");
		REQUIRE(wave->getBasename() == "waveform");
		REQUIRE(wave->getBasename(true) == "waveform.wav");
	}

	SECTION("test memory cleanup")
	{
		float* data = new float[BUFFER_SIZE];

		wave = std::unique_ptr<Wave>(new Wave(data, BUFFER_SIZE, CHANNELS, 
			SAMPLE_RATE, BIT_DEPTH, "path/to/sample.wav"));
		wave->clear();

		REQUIRE(wave->getData() == nullptr);
		REQUIRE(wave->getPath() == "");
		REQUIRE(wave->getSize() == 0);
	}
}
