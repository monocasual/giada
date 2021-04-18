#include "../src/core/audioBuffer.h"
#include <catch2/catch.hpp>
#include <memory>

TEST_CASE("AudioBuffer")
{
	using namespace giada::m;

	static const int BUFFER_SIZE = 4096;

	/* Each SECTION the TEST_CASE is executed from the start. Any code between 
	this comment and the first SECTION macro is exectuted before each SECTION. */

	AudioBuffer buffer;
	buffer.alloc(BUFFER_SIZE, 2);

	SECTION("test allocation")
	{
		SECTION("test mono")
		{
			buffer.alloc(BUFFER_SIZE, 1);
			REQUIRE(buffer.countFrames() == BUFFER_SIZE);
			REQUIRE(buffer.countSamples() == BUFFER_SIZE);
			REQUIRE(buffer.countChannels() == 1);
		}

		SECTION("test stereo")
		{
			REQUIRE(buffer.countFrames() == BUFFER_SIZE);
			REQUIRE(buffer.countSamples() == BUFFER_SIZE * 2);
			REQUIRE(buffer.countChannels() == 2);
		}

		buffer.free();

		REQUIRE(buffer.countFrames() == 0);
		REQUIRE(buffer.countSamples() == 0);
		REQUIRE(buffer.countChannels() == 0);
	}

	SECTION("test clear all")
	{
		buffer.clear();
		for (int i = 0; i < buffer.countFrames(); i++)
			for (int k = 0; k < buffer.countChannels(); k++)
				REQUIRE(buffer[i][k] == 0.0f);
		buffer.free();
	}

	SECTION("test clear range")
	{
		for (int i = 0; i < buffer.countFrames(); i++)
			for (int k = 0; k < buffer.countChannels(); k++)
				buffer[i][k] = 1.0f;

		buffer.clear(5, 6);

		for (int k = 0; k < buffer.countChannels(); k++)
			REQUIRE(buffer[5][k] == 0.0f);

		buffer.free();
	}

	SECTION("test copy")
	{
		AudioBuffer other(BUFFER_SIZE, 2);

		for (int i = 0; i < other.countFrames(); i++)
			for (int k = 0; k < other.countChannels(); k++)
				other[i][k] = (float)i;

		SECTION("test full copy")
		{
			buffer.set(other, 1.0f);

			REQUIRE(buffer[0][0] == 0.0f);
			REQUIRE(buffer[16][0] == 16.0f);
			REQUIRE(buffer[128][0] == 128.0f);
			REQUIRE(buffer[1024][0] == 1024.0f);
			REQUIRE(buffer[BUFFER_SIZE - 1][0] == (float)BUFFER_SIZE - 1);
		}
	}
}
