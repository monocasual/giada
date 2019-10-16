#include "../src/core/channels/sampleChannel.h"
#include "../src/core/channels/sampleChannelProc.h"
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

	SampleChannel ch(false, BUFFER_SIZE, 1, 1);

	AudioBuffer out;
	AudioBuffer in;
	AudioBuffer inToOut;

	REQUIRE(ch.playStatus == ChannelStatus::EMPTY);
	REQUIRE(ch.mode == ChannelMode::SINGLE_BASIC);

	SECTION("buffer")
	{
		SECTION("prepare")
		{
			/* With no wave data in it. */
			sampleChannelProc::render(&ch, out, in, inToOut, /*audible=*/true, 
				/*running=*/false);
			REQUIRE(ch.tracker == 0);

			/* With data, stopped. */
			ch.pushWave(1, 1024);
			sampleChannelProc::render(&ch, out, in, inToOut, /*audible=*/true, 
				/*running=*/false);

			REQUIRE(ch.tracker == 0);

			/* With data, playing. */
			ch.playStatus = ChannelStatus::PLAY;
			sampleChannelProc::render(&ch, out, in, inToOut, /*audible=*/true, 
				/*running=*/false);

			REQUIRE(ch.tracker == BUFFER_SIZE);
		}

		SECTION("fill")
		{
			ch.pushWave(1, 1024);

			/* Zero offset. */
			REQUIRE(ch.fillBuffer(ch.buffer, 0, 0) == BUFFER_SIZE);

			/* Non-zero offset. */
			REQUIRE(ch.fillBuffer(ch.buffer, 0, 666) == BUFFER_SIZE - 666);

			/* At the end of the waveform. */
			REQUIRE(ch.fillBuffer(ch.buffer, ch.end - 666, 0) == (ch.end - (ch.end - 666)) + 1);
		}
	}

	SECTION("statuses")
	{
		ch.pushWave(1, 1024);

		SECTION("start from OFF")
		{
			for (ChannelMode mode : modes) {
				ch.mode       = mode;
				ch.playStatus = ChannelStatus::OFF;
				sampleChannelProc::start(&ch, 0, /*doQuantize=*/false, /*velocity=*/0);

				if (ch.isAnyLoopMode())
					REQUIRE(ch.playStatus == ChannelStatus::WAIT);
				else
					REQUIRE(ch.playStatus == ChannelStatus::PLAY);
			}
		}

		SECTION("start from PLAY")
		{
			for (ChannelMode mode : modes) {
				ch.mode       = mode;
				ch.playStatus = ChannelStatus::PLAY;
				ch.tracker    = 16; // simulate processing
				sampleChannelProc::start(&ch, 0, /*doQuantize=*/false, /*velocity=*/0);
				
				if (ch.mode == ChannelMode::SINGLE_RETRIG) {
					REQUIRE(ch.playStatus == ChannelStatus::PLAY);
					REQUIRE(ch.tracker == 0);
				}
				else
				if (ch.isAnyLoopMode() || ch.mode == ChannelMode::SINGLE_ENDLESS) 
					REQUIRE(ch.playStatus == ChannelStatus::ENDING);
				else
				if (ch.mode == ChannelMode::SINGLE_BASIC) {
					REQUIRE(ch.playStatus == ChannelStatus::OFF);
					REQUIRE(ch.tracker == 0);
				}
			}
		}

		SECTION("start from WAIT")
		{
			for (ChannelMode mode : modes) {
				ch.mode       = mode;
				ch.playStatus = ChannelStatus::WAIT;
				sampleChannelProc::start(&ch, 0, /*doQuantize=*/false, /*velocity=*/0);

				REQUIRE(ch.playStatus == ChannelStatus::OFF);
			}
		}

		SECTION("start from ENDING")
		{
			for (ChannelMode mode : modes) {
				ch.mode       = mode;
				ch.playStatus = ChannelStatus::ENDING;
				sampleChannelProc::start(&ch, 0, /*doQuantize=*/false, /*velocity=*/0);

				REQUIRE(ch.playStatus == ChannelStatus::PLAY);
			}
		}

		SECTION("stop from PLAY")
		{
			for (ChannelMode mode : modes) {
				ch.mode       = mode;
				ch.playStatus = ChannelStatus::PLAY;
				ch.tracker    = 16; // simulate processing
				sampleChannelProc::stop(&ch);

				if (ch.mode == ChannelMode::SINGLE_PRESS) {
					REQUIRE(ch.playStatus == ChannelStatus::OFF);
					REQUIRE(ch.tracker == 0);
				}
				else {
					/* Nothing should change for other modes. */
					REQUIRE(ch.playStatus == ChannelStatus::PLAY);
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
					ch.mode       = mode;
					ch.playStatus = status;
					ch.tracker    = 16; // simulate processing
					sampleChannelProc::kill(&ch, 0);
					
					if (ch.playStatus == ChannelStatus::WAIT || 
					    ch.playStatus == ChannelStatus::PLAY ||
					    ch.playStatus == ChannelStatus::ENDING) {
						REQUIRE(ch.playStatus == ChannelStatus::OFF);
						REQUIRE(ch.tracker == 0);
					}
				}
			}		
		}

		SECTION("quantized start")
		{
			for (ChannelMode mode : modes) {
				ch.mode       = mode;		
				ch.playStatus = ChannelStatus::OFF;	
				sampleChannelProc::start(&ch, 0, /*doQuantize=*/true, /*velocity=*/0);

				if (ch.isAnyLoopMode())
					REQUIRE(ch.playStatus == ChannelStatus::WAIT);	
				else {
					REQUIRE(ch.playStatus == ChannelStatus::OFF);	
					REQUIRE(ch.quantizing == true);	
				}
			}
		}
	} 

	SECTION("stop input recordings")
	{
		/* Start all sample channels in any loop mode that were armed. */
		for (ChannelMode mode : modes) {
			ch.mode       = mode;
			ch.playStatus = ChannelStatus::OFF;	
			ch.armed      = true;
			ch.tracker    = 16;
			
			sampleChannelProc::stopInputRec(&ch, /*globalFrame=*/666);

			if (ch.isAnyLoopMode()) {
				REQUIRE(ch.playStatus == ChannelStatus::PLAY);
				REQUIRE(ch.tracker == 666);
			}
			else {
				REQUIRE(ch.playStatus == ChannelStatus::OFF);
				REQUIRE(ch.tracker == 16);				
			}
		}
	}

	SECTION("rewind by sequencer")
	{
		ch.pushWave(1, 1024);

		/* Test loop modes. */

		for (ChannelMode mode : modes) {
			ch.mode       = mode;
			ch.playStatus = ChannelStatus::PLAY;	
			ch.tracker    = 16; // simulate processing

			sampleChannelProc::rewindBySeq(&ch);

			if (ch.isAnyLoopMode()) {
				REQUIRE(ch.playStatus == ChannelStatus::PLAY);
				REQUIRE(ch.tracker == 0);
			}
			else {
				REQUIRE(ch.playStatus == ChannelStatus::PLAY);
				REQUIRE(ch.tracker == 16);				
			}
		}

		/* Test single modes that are reading actions. */

		for (ChannelMode mode : modes) {
			ch.mode       = mode;
			ch.playStatus = ChannelStatus::PLAY;	
			ch.tracker    = 16; // simulate processing
			ch.recStatus  = ChannelStatus::PLAY;

			sampleChannelProc::rewindBySeq(&ch);

			if (ch.isAnySingleMode()) {
				REQUIRE(ch.playStatus == ChannelStatus::PLAY);
				REQUIRE(ch.tracker == 0);
			}
		}		
	}
}
