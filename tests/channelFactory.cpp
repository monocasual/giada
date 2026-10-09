
#include "../src/core/channels/channelFactory.h"
#include "../src/core/types.h"
#include <catch2/catch_test_macros.hpp>

using std::string;
using namespace giada;
using namespace giada::m;

TEST_CASE("channelFactory")
{
	SECTION("test creation")
	{
		channelFactory::Data_DEPR_ data = channelFactory::create_DEPR_(
		    /*id=*/{},
		    ChannelType::SAMPLE,
		    /*sampleRate=*/44100,
		    /*bufferSize=*/1024,
		    Resampler::Quality::LINEAR,
		    /*overdubProtection=*/false);

		REQUIRE(data.channel.id.isValid()); // Id must be auto-generated if passed {} (invalid) in channelFactory::create
		REQUIRE(data.channel.type == ChannelType::SAMPLE);

		SECTION("test clone")
		{
			channelFactory::Data_DEPR_ clone = channelFactory::create(data.channel, /*sampleRate=*/44100,
			    /*bufferSize=*/1024, Resampler::Quality::LINEAR);

			REQUIRE(clone.channel.id != data.channel.id); // Clone must have new ID
			REQUIRE(clone.channel.type == data.channel.type);
			REQUIRE(clone.channel.volume == data.channel.volume);
			REQUIRE(clone.channel.pan == data.channel.pan);
			REQUIRE(clone.channel.armed == data.channel.armed);
			REQUIRE(clone.channel.key == data.channel.key);
			REQUIRE(clone.channel.getName(Scene{0}) == data.channel.getName(Scene{0}));
			REQUIRE(clone.channel.height == data.channel.height);
		}
	}
}
