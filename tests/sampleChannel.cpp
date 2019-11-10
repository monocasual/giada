#include "../src/core/channels/sampleChannel.h"
#include "../src/core/model/model.h"
#include <catch.hpp>


TEST_CASE("sampleChannel")
{
	using namespace giada;
	using namespace giada::m;

	const int BUFFER_SIZE = 1024;
	const int WAVE_SIZE   = 50000;

	std::vector<ChannelMode> modes = { ChannelMode::LOOP_BASIC, 
		ChannelMode::LOOP_ONCE, ChannelMode::LOOP_REPEAT, 
		ChannelMode::LOOP_ONCE_BAR, ChannelMode::SINGLE_BASIC, 
		ChannelMode::SINGLE_PRESS, ChannelMode::SINGLE_RETRIG, 
		ChannelMode::SINGLE_ENDLESS };

	model::channels.clear();
	model::channels.push(std::make_unique<SampleChannel>(false, BUFFER_SIZE, 1, 1));

	model::onSwap(model::channels, 1, [&](Channel& c)
	{
		static_cast<SampleChannel&>(c).pushWave(1, WAVE_SIZE);
	});

	model::channels.lock();
	SampleChannel& ch = static_cast<SampleChannel&>(*model::channels.get(0));
	model::channels.unlock();

	SECTION("push wave")
	{
		REQUIRE(ch.playStatus == ChannelStatus::OFF);
		REQUIRE(ch.begin == 0);
		REQUIRE(ch.end == WAVE_SIZE);
		REQUIRE(ch.name == "");		
	}

	SECTION("begin/end")
	{
		/* TODO - This section requires model::waves interaction. Let's wait for 
		the non-virtual channel refactoring... */
		/*
		ch.setBegin(-100);

		REQUIRE(ch.getBegin() == 0);
		REQUIRE(ch.tracker == 0);
		REQUIRE(ch.trackerPreview == 0);

		ch.setBegin(100000);

		REQUIRE(ch.getBegin() == WAVE_SIZE);
		REQUIRE(ch.tracker == WAVE_SIZE);
		REQUIRE(ch.trackerPreview == WAVE_SIZE);

		ch.setBegin(16);

		REQUIRE(ch.getBegin() == 16);
		REQUIRE(ch.tracker == 16);
		REQUIRE(ch.trackerPreview == 16);

		ch.setEnd(0);

		REQUIRE(ch.getEnd() == 17);

		ch.setEnd(100000);

		REQUIRE(ch.getEnd() == WAVE_SIZE - 1);

		ch.setEnd(32);

		REQUIRE(ch.getEnd() == 32);

		ch.setBegin(64);

		REQUIRE(ch.getBegin() == 31);
		*/
	}

	SECTION("pitch")
	{
		ch.setPitch(40.0f);

		REQUIRE(ch.getPitch() == G_MAX_PITCH);

		ch.setPitch(-2.0f);

		REQUIRE(ch.getPitch() == G_MIN_PITCH);

		ch.setPitch(0.8f);

		REQUIRE(ch.getPitch() == 0.8f);
	}

	SECTION("position")
	{
		REQUIRE(ch.getPosition() == -1);  // Initially OFF

		ch.playStatus = ChannelStatus::PLAY;
		ch.tracker    = 1000;

		REQUIRE(ch.getPosition() == 1000);

		ch.begin = 700;

		REQUIRE(ch.getPosition() == 300);
	}

	SECTION("empty")
	{
		ch.empty();

		REQUIRE(ch.playStatus == ChannelStatus::EMPTY);
		REQUIRE(ch.begin == 0);
		REQUIRE(ch.end == 0);
		REQUIRE(ch.tracker == 0);
		REQUIRE(ch.volume == G_DEFAULT_VOL);
		REQUIRE(ch.hasActions == false);
		REQUIRE(ch.hasWave == false);
		REQUIRE(ch.waveId == 0);
	}

	SECTION("can record audio")
	{	
		REQUIRE(ch.canInputRec() == false); // Can't record if not armed

		ch.armed = true;

		REQUIRE(ch.canInputRec() == false); // Can't record with Wave in it

		ch.empty();

		REQUIRE(ch.canInputRec() == true);
	}

	/* TODO - fillBuffer, isAnyLoopMode, isAnySingleMode, isOnLastFrame */
}
