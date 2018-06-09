#include "../src/core/sampleChannel.h"
#include "../src/core/sampleChannelRec.h"
#include <catch.hpp>


using namespace giada;
using namespace giada::m;


TEST_CASE("sampleChannelRec")
{
	const int BUFFER_SIZE = 1024;

	SampleChannel ch(false, BUFFER_SIZE);

	SECTION("start reading actions, don't treat recs as loop")
	{
		sampleChannelRec::startReadingActions(&ch, /*treatRecsAsLoops=*/false, 
			/*recsStopOnChanHalt=*/false);	

		REQUIRE(ch.recStatus == ChannelStatus::OFF);
		REQUIRE(ch.readActions == true);
	}	

	SECTION("start reading actions, do treat recs as loop")
	{
		sampleChannelRec::startReadingActions(&ch, /*treatRecsAsLoops=*/true, 
			/*recsStopOnChanHalt=*/false);	

		REQUIRE(ch.recStatus == ChannelStatus::WAIT);
		REQUIRE(ch.readActions == false);
	}	

	SECTION("stop reading actions")
	{
		/* First state: PLAY */
		ch.recStatus = ChannelStatus::PLAY;

		sampleChannelRec::stopReadingActions(&ch, /*clockRunning=*/true, 
			/*treatRecsAsLoops=*/false, /*recsStopOnChanHalt=*/false);

		REQUIRE(ch.readActions == false);		
		REQUIRE(ch.recStatus == ChannelStatus::PLAY);		

		/* Second state: WAIT */
		ch.recStatus = ChannelStatus::WAIT;

		sampleChannelRec::stopReadingActions(&ch, /*clockRunning=*/true, 
			/*treatRecsAsLoops=*/false, /*recsStopOnChanHalt=*/false);

		REQUIRE(ch.readActions == false);		
		REQUIRE(ch.recStatus == ChannelStatus::OFF);		

		/* Third state: WAIT */
		ch.recStatus = ChannelStatus::ENDING;

		sampleChannelRec::stopReadingActions(&ch, /*clockRunning=*/true, 
			/*treatRecsAsLoops=*/false, /*recsStopOnChanHalt=*/false);

		REQUIRE(ch.readActions == false);		
		REQUIRE(ch.recStatus == ChannelStatus::PLAY);

		/* Fourth state: any, but with clockRunning == false. */

		sampleChannelRec::stopReadingActions(&ch, /*clockRunning=*/false, 
			/*treatRecsAsLoops=*/false, /*recsStopOnChanHalt=*/false);

		REQUIRE(ch.readActions == false);		
		REQUIRE(ch.recStatus == ChannelStatus::OFF);		
	}


	SECTION("set read actions status to false with recsStopOnChanHalt")
	{
		ch.status  = ChannelStatus::PLAY;
		ch.tracker = 1024;

		sampleChannelRec::setReadActions(&ch, false, /*recsStopOnChanHalt=*/true);

		REQUIRE(ch.readActions == false);
		REQUIRE(ch.status == ChannelStatus::OFF);
		REQUIRE(ch.tracker == 0);

	}
}
