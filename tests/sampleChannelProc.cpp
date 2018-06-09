#include "../src/core/sampleChannel.h"
#include "../src/core/sampleChannelProc.h"
#include "../src/core/wave.h"
#include "../src/core/waveManager.h"
#include <catch.hpp>


using namespace giada;
using namespace giada::m;


TEST_CASE("sampleChannelProc")
{
	const int BUFFER_SIZE = 1024;

	std::vector<ChannelMode> modes = { ChannelMode::LOOP_BASIC, 
		ChannelMode::LOOP_ONCE, ChannelMode::LOOP_REPEAT, 
		ChannelMode::LOOP_ONCE_BAR, ChannelMode::SINGLE_BASIC, 
		ChannelMode::SINGLE_PRESS, ChannelMode::SINGLE_RETRIG, 
		ChannelMode::SINGLE_ENDLESS };

	Wave* w;
	SampleChannel ch(false, BUFFER_SIZE);
	waveManager::create("tests/resources/test.wav", &w); 

	REQUIRE(ch.status == ChannelStatus::EMPTY);
	REQUIRE(ch.mode == ChannelMode::SINGLE_BASIC);

	SECTION("buffer")
	{
		SECTION("prepare")
		{
			/* With no wave data in it. */
			sampleChannelProc::prepareBuffer(&ch, /*running=*/false);

			REQUIRE(ch.tracker == 0);

			/* With data, stopped. */
			ch.pushWave(w);
			sampleChannelProc::prepareBuffer(&ch, /*running=*/false);

			REQUIRE(ch.tracker == 0);

			/* With data, playing. */
			ch.status = ChannelStatus::PLAY;
			sampleChannelProc::prepareBuffer(&ch, /*running=*/false);

			REQUIRE(ch.tracker == BUFFER_SIZE);
		}

		SECTION("fill")
		{
			ch.pushWave(w);

			/* Zero offset. */
			REQUIRE(ch.fillBuffer(ch.buffer, 0, 0) == BUFFER_SIZE);

			/* Non-zero offset. */
			REQUIRE(ch.fillBuffer(ch.buffer, 0, 666) == BUFFER_SIZE - 666);

			/* At the end of the waveform. */
			REQUIRE(ch.fillBuffer(ch.buffer, ch.end - 666, 0) == ch.end - (ch.end - 666));
		}
	}

	SECTION("statuses")
	{
		ch.pushWave(w);

		SECTION("start from OFF")
		{
			for (ChannelMode mode : modes) {
				ch.mode   = mode;
				ch.status = ChannelStatus::OFF;
				sampleChannelProc::start(&ch, 0, /*doQuantize=*/false, /*velocity=*/0);

				if (ch.isAnyLoopMode())
					REQUIRE(ch.status == ChannelStatus::WAIT);
				else
					REQUIRE(ch.status == ChannelStatus::PLAY);
			}
		}

		SECTION("start from PLAY")
		{
			for (ChannelMode mode : modes) {
				ch.mode    = mode;
				ch.status  = ChannelStatus::PLAY;
				ch.tracker = 16; // simulate processing
				sampleChannelProc::start(&ch, 0, /*doQuantize=*/false, /*velocity=*/0);
				
				if (ch.mode == ChannelMode::SINGLE_RETRIG) {
					REQUIRE(ch.status == ChannelStatus::PLAY);
					REQUIRE(ch.tracker == 0);
				}
				else
				if (ch.isAnyLoopMode() || ch.mode == ChannelMode::SINGLE_ENDLESS) 
					REQUIRE(ch.status == ChannelStatus::ENDING);
				else
				if (ch.mode == ChannelMode::SINGLE_BASIC) {
					REQUIRE(ch.status == ChannelStatus::OFF);
					REQUIRE(ch.tracker == 0);
				}
			}
		}

		SECTION("start from WAIT")
		{
			for (ChannelMode mode : modes) {
				ch.mode   = mode;
				ch.status = ChannelStatus::WAIT;
				sampleChannelProc::start(&ch, 0, /*doQuantize=*/false, /*velocity=*/0);

				REQUIRE(ch.status == ChannelStatus::OFF);
			}
		}

		SECTION("start from ENDING")
		{
			for (ChannelMode mode : modes) {
				ch.mode   = mode;
				ch.status = ChannelStatus::ENDING;
				sampleChannelProc::start(&ch, 0, /*doQuantize=*/false, /*velocity=*/0);

				REQUIRE(ch.status == ChannelStatus::PLAY);
			}
		}

		SECTION("stop from PLAY")
		{
			for (ChannelMode mode : modes) {
				ch.mode   = mode;
				ch.status = ChannelStatus::PLAY;
				ch.tracker = 16; // simulate processing
				sampleChannelProc::stop(&ch);

				if (ch.mode == ChannelMode::SINGLE_PRESS) {
					REQUIRE(ch.status == ChannelStatus::OFF);
					REQUIRE(ch.tracker == 0);
				}
				else {
					/* Nothing should change for other modes. */
					REQUIRE(ch.status == ChannelStatus::PLAY);
					REQUIRE(ch.tracker == 16);					
				}
			}
		}

		SECTION("kill")
		{
			std::vector<ChannelStatus> statuses = { ChannelStatus::ENDING, 
				ChannelStatus::WAIT, ChannelStatus::PLAY, ChannelStatus::OFF, 
				ChannelStatus::EMPTY, ChannelStatus::MISSING, ChannelStatus::WRONG };  

			for (ChannelMode mode : modes) {
				for (ChannelStatus status : statuses) {
					ch.mode    = mode;
					ch.status  = status;
					ch.tracker = 16; // simulate processing
					sampleChannelProc::kill(&ch, 0);
					
					if (ch.status == ChannelStatus::WAIT || 
					    ch.status == ChannelStatus::PLAY ||
					    ch.status == ChannelStatus::ENDING) {
						REQUIRE(ch.status == ChannelStatus::OFF);
						REQUIRE(ch.tracker == 0);
					}
				}
			}		
		}

		SECTION("quantized start")
		{
			for (ChannelMode mode : modes) {
				ch.mode   = mode;		
				ch.status = ChannelStatus::OFF;	
				sampleChannelProc::start(&ch, 0, /*doQuantize=*/true, /*velocity=*/0);

				if (ch.isAnyLoopMode())
					REQUIRE(ch.status == ChannelStatus::WAIT);	
				else {
					REQUIRE(ch.status == ChannelStatus::OFF);	
					REQUIRE(ch.qWait == true);	
				}
			}
		}
	} 

	SECTION("stop input recordings")
	{
		/* Start all sample channels in any loop mode that were armed. */
		for (ChannelMode mode : modes) {
			ch.mode    = mode;
			ch.status  = ChannelStatus::OFF;	
			ch.armed   = true;
			ch.tracker = 16;
			
			sampleChannelProc::stopInputRec(&ch, /*globalFrame=*/666);

			if (ch.isAnyLoopMode()) {
				REQUIRE(ch.status == ChannelStatus::PLAY);
				REQUIRE(ch.tracker == 666);
			}
			else {
				REQUIRE(ch.status == ChannelStatus::OFF);
				REQUIRE(ch.tracker == 16);				
			}
		}
	}

	SECTION("rewind by sequencer")
	{
		ch.pushWave(w);

		/* Test loop modes. */

		for (ChannelMode mode : modes) {
			ch.mode    = mode;
			ch.status  = ChannelStatus::PLAY;	
			ch.tracker = 16; // simulate processing

			sampleChannelProc::rewindBySeq(&ch);

			if (ch.isAnyLoopMode()) {
				REQUIRE(ch.status == ChannelStatus::PLAY);
				REQUIRE(ch.tracker == 0);
			}
			else {
				REQUIRE(ch.status == ChannelStatus::PLAY);
				REQUIRE(ch.tracker == 16);				
			}
		}

		/* Test single modes that are reading actions. */

		for (ChannelMode mode : modes) {
			ch.mode      = mode;
			ch.status    = ChannelStatus::PLAY;	
			ch.tracker   = 16; // simulate processing
			ch.recStatus = ChannelStatus::PLAY;

			sampleChannelProc::rewindBySeq(&ch);

			if (ch.isAnySingleMode()) {
				REQUIRE(ch.status == ChannelStatus::PLAY);
				REQUIRE(ch.tracker == 0);
			}
		}		
	}
}
