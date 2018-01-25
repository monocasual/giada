#include "../src/core/recorder.h"
#include "../src/core/const.h"
#include <catch.hpp>


using std::string;
using namespace giada::m;


TEST_CASE("Test Recorder")
{
	/* Each SECTION the TEST_CASE is executed from the start. The following
	code is exectuted before each SECTION. */

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, nullptr);

	recorder::init();
	REQUIRE(recorder::frames.size() == 0);
	REQUIRE(recorder::global.size() == 0);

	SECTION("Test record single action")
	{
		recorder::rec(0, G_ACTION_KEYPRESS, 50, 1, 0.5f);

		REQUIRE(recorder::frames.size() == 1);
		REQUIRE(recorder::frames.at(0) == 50);
		REQUIRE(recorder::global.at(0).size() == 1);  // 1 action on frame #0
		REQUIRE(recorder::global.at(0).at(0)->chan == 0);
		REQUIRE(recorder::global.at(0).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder::global.at(0).at(0)->frame == 50);
		REQUIRE(recorder::global.at(0).at(0)->iValue == 1);
		REQUIRE(recorder::global.at(0).at(0)->fValue == 0.5f);
	}

	SECTION("Test record, two actions on same frame")
	{
		recorder::rec(0, G_ACTION_KEYPRESS, 50, 6, 0.3f);
		recorder::rec(0, G_ACTION_KEYREL,   50, 1, 0.5f);

		REQUIRE(recorder::frames.size() == 1);    // same frame, frames.size must stay 1
		REQUIRE(recorder::frames.at(0) == 50);
		REQUIRE(recorder::global.at(0).size() == 2);  // 2 actions on frame #0

		REQUIRE(recorder::global.at(0).at(0)->chan == 0);
		REQUIRE(recorder::global.at(0).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder::global.at(0).at(0)->frame == 50);
		REQUIRE(recorder::global.at(0).at(0)->iValue == 6);
		REQUIRE(recorder::global.at(0).at(0)->fValue == 0.3f);

		REQUIRE(recorder::global.at(0).at(1)->chan == 0);
		REQUIRE(recorder::global.at(0).at(1)->type == G_ACTION_KEYREL);
		REQUIRE(recorder::global.at(0).at(1)->frame == 50);
		REQUIRE(recorder::global.at(0).at(1)->iValue == 1);
		REQUIRE(recorder::global.at(0).at(1)->fValue == 0.5f);

		SECTION("Test record, another action on a different frame")
		{
			recorder::rec(0, G_ACTION_KEYPRESS, 70, 1, 0.5f);

			REQUIRE(recorder::frames.size() == 2);
			REQUIRE(recorder::frames.at(1) == 70);
			REQUIRE(recorder::global.at(0).size() == 2);  // 2 actions on frame #0
			REQUIRE(recorder::global.at(1).size() == 1);  // 1 actions on frame #1
			REQUIRE(recorder::global.at(1).at(0)->chan == 0);
			REQUIRE(recorder::global.at(1).at(0)->type == G_ACTION_KEYPRESS);
			REQUIRE(recorder::global.at(1).at(0)->frame == 70);
			REQUIRE(recorder::global.at(1).at(0)->iValue == 1);
			REQUIRE(recorder::global.at(1).at(0)->fValue == 0.5f);
		}
	}

	SECTION("Test retrieval")
	{
		recorder::rec(0, G_ACTION_KEYPRESS, 50, 6, 0.3f);
		recorder::rec(0, G_ACTION_KEYREL,   70, 1, 0.5f);
		recorder::rec(1, G_ACTION_KEYPRESS, 50, 6, 0.3f);
		recorder::rec(1, G_ACTION_KEYREL,   70, 1, 0.5f);
		recorder::rec(2, G_ACTION_KEYPRESS, 100, 6, 0.3f);
		recorder::rec(2, G_ACTION_KEYREL,   120, 1, 0.5f);

		/* Give me action on chan 1, type G_ACTION_KEYREL, frame 70. */
		recorder::action *action = nullptr;
		REQUIRE(recorder::getAction(1, G_ACTION_KEYREL, 70, &action) == 1);

		REQUIRE(action != nullptr);
		REQUIRE(action->chan == 1);
		REQUIRE(action->type == G_ACTION_KEYREL);
		REQUIRE(action->frame == 70);
		REQUIRE(action->iValue == 1);
		REQUIRE(action->fValue == 0.5f);

		/* Give me *next* action on chan 0, type G_ACTION_KEYREL, starting from frame 20.
		Must be action #2 */

		REQUIRE(recorder::getNextAction(0, G_ACTION_KEYREL, 20, &action) == 1);
		REQUIRE(action != nullptr);
		REQUIRE(action->chan == 0);
		REQUIRE(action->type == G_ACTION_KEYREL);
		REQUIRE(action->frame == 70);

		/* Give me *next* action on chan 2, type G_ACTION_KEYPRESS, starting from
		frame 200. You are requesting frame outside boundaries. */

		REQUIRE(recorder::getNextAction(2, G_ACTION_KEYPRESS, 200, &action) == -1);

		/* Give me *next* action on chan 2, type G_ACTION_KEYPRESS, starting from
		frame 100. That action does not exist. */

		REQUIRE(recorder::getNextAction(2, G_ACTION_KEYPRESS, 100, &action) == -2);
	}

	SECTION("Test retrieval MIDI")
	{
		recorder::rec(0, G_ACTION_MIDI, 0,    0x903C3F00, 0.0f);
		recorder::rec(1, G_ACTION_MIDI, 0,    0x903D3F00, 0.0f);
		recorder::rec(0, G_ACTION_MIDI, 1000, 0x803C2000, 0.0f);		
		recorder::rec(0, G_ACTION_MIDI, 1050, 0x903C3F00, 0.0f);
		recorder::rec(0, G_ACTION_MIDI, 2000, 0x803C3F00, 0.0f);
		recorder::rec(1, G_ACTION_MIDI, 90,   0x803D3F00, 0.0f);		
		recorder::rec(1, G_ACTION_MIDI, 1050, 0x903D3F00, 0.0f);
		recorder::rec(1, G_ACTION_MIDI, 2000, 0x803D3F00, 0.0f);

		recorder::action* result = nullptr;
		recorder::getNextAction(0, G_ACTION_MIDI, 100, &result, 0x803CFF00, 0x0000FF00);

		REQUIRE(result != nullptr);
		REQUIRE(result->frame == 1000);
	}

	SECTION("Test deletion, single action")
	{
		recorder::rec(0, G_ACTION_KEYPRESS, 50, 6, 0.3f);
		recorder::rec(0, G_ACTION_KEYREL,   60, 1, 0.5f);
		recorder::rec(1, G_ACTION_KEYPRESS, 70, 6, 0.3f);
		recorder::rec(1, G_ACTION_KEYREL,   80, 1, 0.5f);

		/* Delete action #0, don't check values. */
		recorder::deleteAction(0, 50, G_ACTION_KEYPRESS, false, &mutex);

		REQUIRE(recorder::frames.size() == 3);
		REQUIRE(recorder::global.size() == 3);

		SECTION("Test deletion checked")
		{
			/* Delete action #1, check values. */
			recorder::deleteAction(1, 70, G_ACTION_KEYPRESS, true, &mutex, 6, 0.3f);

			REQUIRE(recorder::frames.size() == 2);
			REQUIRE(recorder::global.size() == 2);
		}
	}

	SECTION("Test deletion, range of actions")
	{
		recorder::rec(0, G_ACTION_KEYPRESS, 50, 6, 0.3f);
		recorder::rec(0, G_ACTION_KEYREL,   60, 1, 0.5f);
		recorder::rec(0, G_ACTION_KEYPRESS, 70, 6, 0.3f);
		recorder::rec(0, G_ACTION_KEYREL,   80, 1, 0.5f);
		recorder::rec(1, G_ACTION_KEYPRESS, 100, 6, 0.3f);
		recorder::rec(1, G_ACTION_KEYREL,   120, 1, 0.5f);

		/* Delete any action on channel 0 of types KEYPRESS | KEYREL between
		frames 0 and 200. */

		recorder::deleteActions(0, 0, 200, G_ACTION_KEYPRESS | G_ACTION_KEYREL, &mutex);

		REQUIRE(recorder::frames.size() == 2);
		REQUIRE(recorder::global.size() == 2);
		REQUIRE(recorder::global.at(0).size() == 1);
		REQUIRE(recorder::global.at(1).size() == 1);

		REQUIRE(recorder::global.at(0).at(0)->chan == 1);
		REQUIRE(recorder::global.at(0).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder::global.at(0).at(0)->frame == 100);
		REQUIRE(recorder::global.at(0).at(0)->iValue == 6);
		REQUIRE(recorder::global.at(0).at(0)->fValue == 0.3f);

		REQUIRE(recorder::global.at(1).at(0)->chan == 1);
		REQUIRE(recorder::global.at(1).at(0)->type == G_ACTION_KEYREL);
		REQUIRE(recorder::global.at(1).at(0)->frame == 120);
		REQUIRE(recorder::global.at(1).at(0)->iValue == 1);
		REQUIRE(recorder::global.at(1).at(0)->fValue == 0.5f);
	}

	SECTION("Test action presence")
	{
		recorder::rec(0, G_ACTION_KEYREL,   80, 1, 0.5f);
		recorder::rec(1, G_ACTION_KEYPRESS, 100, 6, 0.3f);
		recorder::rec(1, G_ACTION_KEYREL,   120, 1, 0.5f);

		recorder::deleteAction(0, 80, G_ACTION_KEYREL, false, &mutex);

		REQUIRE(recorder::hasActions(0) == false);
		REQUIRE(recorder::hasActions(1) == true);
	}

	SECTION("Test clear actions by channel")
	{
		recorder::rec(0, G_ACTION_KEYREL,   80, 1, 0.5f);
		recorder::rec(1, G_ACTION_KEYPRESS, 100, 6, 0.3f);
		recorder::rec(1, G_ACTION_KEYREL,   120, 1, 0.5f);

		recorder::clearChan(1);

		REQUIRE(recorder::hasActions(0) == true);
		REQUIRE(recorder::hasActions(1) == false);
		REQUIRE(recorder::frames.size() == 1);
		REQUIRE(recorder::global.size() == 1);
		REQUIRE(recorder::global.at(0).size() == 1);
	}

	SECTION("Test clear actions by type")
	{
		recorder::rec(1, G_ACTION_KEYREL,   80, 1, 0.5f);
		recorder::rec(0, G_ACTION_KEYPRESS, 100, 6, 0.3f);
		recorder::rec(1, G_ACTION_KEYREL,   120, 1, 0.5f);

		/* Clear all actions of type KEYREL from channel 1. */

		recorder::clearAction(1, G_ACTION_KEYREL);

		REQUIRE(recorder::hasActions(0) == true);
		REQUIRE(recorder::hasActions(1) == false);
		REQUIRE(recorder::frames.size() == 1);
		REQUIRE(recorder::global.size() == 1);
		REQUIRE(recorder::global.at(0).size() == 1);
	}

	SECTION("Test clear all")
	{
		recorder::rec(0, G_ACTION_KEYPRESS, 0, 1, 0.5f);
		recorder::rec(1, G_ACTION_KEYPRESS, 0, 1, 0.5f);
		recorder::rec(0, G_ACTION_KEYREL,   80, 1, 0.5f);
		recorder::rec(1, G_ACTION_KEYREL,   100, 6, 0.3f);
		recorder::rec(2, G_ACTION_KILL, 120, 1, 0.5f);

		recorder::clearAll();
		REQUIRE(recorder::frames.size() == 0);
		REQUIRE(recorder::global.size() == 0);
	}

	SECTION("Test optimization")
	{
		recorder::rec(0, G_ACTION_KEYPRESS, 20, 1, 0.5f);
		recorder::rec(0, G_ACTION_KEYREL,   80, 1, 0.5f);
		recorder::rec(1, G_ACTION_KEYPRESS, 20, 1, 0.5f);
		recorder::rec(1, G_ACTION_KEYREL,   80, 1, 0.5f);

		/* Fake frame 80 without actions.*/
		recorder::global.at(1).clear();

		recorder::optimize();

		REQUIRE(recorder::frames.size() == 1);
		REQUIRE(recorder::global.size() == 1);
		REQUIRE(recorder::global.at(0).size() == 2);
	}

	SECTION("Test BPM update")
	{
		recorder::rec(0, G_ACTION_KEYPRESS,  0, 1, 0.5f);
		recorder::rec(0, G_ACTION_KEYREL,   80, 1, 0.5f);

		recorder::updateBpm(60.0f, 120.0f, 44100);  // scaling up

		REQUIRE(recorder::frames.at(0) == 0);
		REQUIRE(recorder::frames.at(1) == 40);

		recorder::updateBpm(120.0f, 60.0f, 44100);  // scaling down

		REQUIRE(recorder::frames.at(0) == 0);
		REQUIRE(recorder::frames.at(1) == 80);
	}

	SECTION("Test samplerate update")
	{
		recorder::rec(0, G_ACTION_KEYPRESS,   0, 1, 0.5f);
		recorder::rec(0, G_ACTION_KEYREL,    80, 1, 0.5f);
		recorder::rec(0, G_ACTION_KEYPRESS, 120, 1, 0.5f);
		recorder::rec(0, G_ACTION_KEYREL,   150, 1, 0.5f);

		recorder::updateSamplerate(44100, 22050); // scaling down

		REQUIRE(recorder::frames.at(0) == 0);
		REQUIRE(recorder::frames.at(1) == 160);
		REQUIRE(recorder::frames.at(2) == 240);
		REQUIRE(recorder::frames.at(3) == 300);

		recorder::updateSamplerate(22050, 44100); // scaling up

		REQUIRE(recorder::frames.at(0) == 0);
		REQUIRE(recorder::frames.at(1) == 80);
		REQUIRE(recorder::frames.at(2) == 120);
		REQUIRE(recorder::frames.at(3) == 150);
	}

	SECTION("Test expand")
	{
		recorder::rec(0, G_ACTION_KEYPRESS,   0, 1, 0.5f);
		recorder::rec(0, G_ACTION_KEYREL,    80, 1, 0.5f);
		recorder::rec(0, G_ACTION_KILL,     200, 1, 0.5f);

		recorder::expand(300, 600);

		REQUIRE(recorder::frames.size() == 6);
		REQUIRE(recorder::global.size() == 6);
		REQUIRE(recorder::frames.at(0) == 0);
		REQUIRE(recorder::frames.at(1) == 80);
		REQUIRE(recorder::frames.at(2) == 200);
		REQUIRE(recorder::frames.at(3) == 300);
		REQUIRE(recorder::frames.at(4) == 380);
		REQUIRE(recorder::frames.at(5) == 500);
	}

	SECTION("Test shrink")
	{
		recorder::rec(0, G_ACTION_KEYPRESS,   0, 1, 0.5f);
		recorder::rec(0, G_ACTION_KEYREL,    80, 1, 0.5f);
		recorder::rec(0, G_ACTION_KILL,     200, 1, 0.5f);

		recorder::shrink(100);

		REQUIRE(recorder::frames.size() == 2);
		REQUIRE(recorder::global.size() == 2);
		REQUIRE(recorder::frames.at(0) == 0);
		REQUIRE(recorder::frames.at(1) == 80);
	}

	SECTION("Test overdub, full overwrite")
	{
		recorder::rec(0, G_ACTION_MUTEON,    0, 1, 0.5f);
		recorder::rec(0, G_ACTION_MUTEOFF,  80, 1, 0.5f);
		recorder::rec(0, G_ACTION_MUTEON,  200, 1, 0.5f);
		recorder::rec(0, G_ACTION_MUTEOFF, 400, 1, 0.5f);

		/* Should delete all actions in between and keep the first one, plus a
		new last action on frame 500. */
		recorder::startOverdub(0, G_ACTION_MUTEON | G_ACTION_MUTEOFF, 0, 1024);
		recorder::stopOverdub(500, 500, &mutex);

		REQUIRE(recorder::frames.size() == 2);
		REQUIRE(recorder::global.size() == 2);
		REQUIRE(recorder::frames.at(0) == 0);
		REQUIRE(recorder::frames.at(1) == 500);
		REQUIRE(recorder::global.at(0).at(0)->frame == 0);
		REQUIRE(recorder::global.at(0).at(0)->type == G_ACTION_MUTEON);
		REQUIRE(recorder::global.at(1).at(0)->frame == 500);
		REQUIRE(recorder::global.at(1).at(0)->type == G_ACTION_MUTEOFF);
	}

	SECTION("Test overdub, left overlap")
	{
		recorder::rec(0, G_ACTION_MUTEON,  100, 1, 0.5f);
		recorder::rec(0, G_ACTION_MUTEOFF, 400, 1, 0.5f);

		/* Overdub part of the leftmost part of a composite action. Expected result:
		a new composite action.
		Original:    ----|########|
		Overdub:     |#######|-----
		Result:      |#######|----- */
		recorder::startOverdub(0, G_ACTION_MUTEON | G_ACTION_MUTEOFF, 0, 16);
		recorder::stopOverdub(300, 500, &mutex);

		REQUIRE(recorder::frames.size() == 2);
		REQUIRE(recorder::global.size() == 2);
		REQUIRE(recorder::frames.at(0) == 0);
		REQUIRE(recorder::frames.at(1) == 300);

		REQUIRE(recorder::global.at(0).at(0)->frame == 0);
		REQUIRE(recorder::global.at(0).at(0)->type == G_ACTION_MUTEON);
		REQUIRE(recorder::global.at(1).at(0)->frame == 300);
		REQUIRE(recorder::global.at(1).at(0)->type == G_ACTION_MUTEOFF);
	}

	SECTION("Test overdub, right overlap")
	{
		recorder::rec(0, G_ACTION_MUTEON,  000, 1, 0.5f);
		recorder::rec(0, G_ACTION_MUTEOFF, 400, 1, 0.5f);

		/* Overdub part of the rightmost part of a composite action. Expected result:
		a new composite action.
		Original:    |########|------
		Overdub:     -----|#######|--
		Result:      |###||#######|-- */
		recorder::startOverdub(0, G_ACTION_MUTEON | G_ACTION_MUTEOFF, 100, 16);
		recorder::stopOverdub(500, 500, &mutex);

		REQUIRE(recorder::frames.size() == 4);
		REQUIRE(recorder::global.size() == 4);
		REQUIRE(recorder::frames.at(0) == 0);
		REQUIRE(recorder::frames.at(1) == 84); // 100 - bufferSize (16)
		REQUIRE(recorder::frames.at(2) == 100);
		REQUIRE(recorder::frames.at(3) == 500);

		REQUIRE(recorder::global.at(0).at(0)->frame == 0);
		REQUIRE(recorder::global.at(0).at(0)->type == G_ACTION_MUTEON);
		REQUIRE(recorder::global.at(1).at(0)->frame == 84);
		REQUIRE(recorder::global.at(1).at(0)->type == G_ACTION_MUTEOFF);

		REQUIRE(recorder::global.at(2).at(0)->frame == 100);
		REQUIRE(recorder::global.at(2).at(0)->type == G_ACTION_MUTEON);
		REQUIRE(recorder::global.at(3).at(0)->frame == 500);
		REQUIRE(recorder::global.at(3).at(0)->type == G_ACTION_MUTEOFF);
	}

	SECTION("Test overdub, hole diggin'")
	{
		recorder::rec(0, G_ACTION_MUTEON,    0, 1, 0.5f);
		recorder::rec(0, G_ACTION_MUTEOFF, 400, 1, 0.5f);

		/* Overdub in the middle of a long, composite action. Expected result:
		original action trimmed down plus anther action next to it. Total frames
		should be 4.
		Original:    |#############|
		Overdub:     ---|#######|---
		Result:      |#||#######|--- */
		recorder::startOverdub(0, G_ACTION_MUTEON | G_ACTION_MUTEOFF, 100, 16);
		recorder::stopOverdub(300, 500, &mutex);

		REQUIRE(recorder::frames.size() == 4);
		REQUIRE(recorder::global.size() == 4);
		REQUIRE(recorder::frames.at(0) == 0);
		REQUIRE(recorder::frames.at(1) == 84); // 100 - bufferSize (16)
		REQUIRE(recorder::frames.at(2) == 100);
		REQUIRE(recorder::frames.at(3) == 300);

		REQUIRE(recorder::global.at(0).at(0)->frame == 0);
		REQUIRE(recorder::global.at(0).at(0)->type == G_ACTION_MUTEON);
		REQUIRE(recorder::global.at(1).at(0)->frame == 84);
		REQUIRE(recorder::global.at(1).at(0)->type == G_ACTION_MUTEOFF);

		REQUIRE(recorder::global.at(2).at(0)->frame == 100);
		REQUIRE(recorder::global.at(2).at(0)->type == G_ACTION_MUTEON);
		REQUIRE(recorder::global.at(3).at(0)->frame == 300);
		REQUIRE(recorder::global.at(3).at(0)->type == G_ACTION_MUTEOFF);
	}

	SECTION("Test overdub, cover all")
	{
		recorder::rec(0, G_ACTION_MUTEON,    0, 1, 0.5f);
		recorder::rec(0, G_ACTION_MUTEOFF, 100, 1, 0.5f);
		recorder::rec(0, G_ACTION_MUTEON,  120, 1, 0.5f);
		recorder::rec(0, G_ACTION_MUTEOFF, 200, 1, 0.5f);
		recorder::rec(0, G_ACTION_MUTEON,  220, 1, 0.5f);
		recorder::rec(0, G_ACTION_MUTEOFF, 300, 1, 0.5f);

		/* Overdub all existing actions. Expected result: a single composite one. */
		recorder::startOverdub(0, G_ACTION_MUTEON | G_ACTION_MUTEOFF, 0, 16);
		recorder::stopOverdub(500, 500, &mutex);

		REQUIRE(recorder::frames.size() == 2);
		REQUIRE(recorder::global.size() == 2);
		REQUIRE(recorder::frames.at(0) == 0);
		REQUIRE(recorder::frames.at(1) == 500);

		REQUIRE(recorder::global.at(0).at(0)->frame == 0);
		REQUIRE(recorder::global.at(0).at(0)->type == G_ACTION_MUTEON);
		REQUIRE(recorder::global.at(1).at(0)->frame == 500);
		REQUIRE(recorder::global.at(1).at(0)->type == G_ACTION_MUTEOFF);
	}

	SECTION("Test overdub, null loop")
	{
		recorder::rec(0, G_ACTION_MUTEON,    0, 1, 0.5f);
		recorder::rec(0, G_ACTION_MUTEOFF, 500, 1, 0.5f);

		/* A null loop is a loop that begins and ends on the very same frame. */
		recorder::startOverdub(0, G_ACTION_MUTEON | G_ACTION_MUTEOFF, 300, 16);
		recorder::stopOverdub(300, 700, &mutex);

		REQUIRE(recorder::frames.size() == 2);
		REQUIRE(recorder::frames.at(0) == 0);
		REQUIRE(recorder::frames.at(1) == 284);  // 300 - bufferSize (16)

		REQUIRE(recorder::global.at(0).at(0)->frame == 0);
		REQUIRE(recorder::global.at(0).at(0)->type == G_ACTION_MUTEON);
		REQUIRE(recorder::global.at(1).at(0)->frame == 284);
		REQUIRE(recorder::global.at(1).at(0)->type == G_ACTION_MUTEOFF);
	}

	SECTION("Test overdub, ring loop")
	{
		/* A ring loop occurs when you record the last action beyond the end of
		the sequencer.
		Original:    ---|#######|---
		Overdub:     #####|------|##
		Result:      ---|#######||#| */

		recorder::rec(0, G_ACTION_MUTEON,  200, 1, 0.5f);
		recorder::rec(0, G_ACTION_MUTEOFF, 300, 1, 0.5f);

		recorder::startOverdub(0, G_ACTION_MUTEON | G_ACTION_MUTEOFF, 400, 16);
		recorder::stopOverdub(250, 700, &mutex);

		REQUIRE(recorder::frames.size() == 4);
		REQUIRE(recorder::frames.at(0) == 200);
		REQUIRE(recorder::frames.at(1) == 300);
		REQUIRE(recorder::frames.at(2) == 400);
		REQUIRE(recorder::frames.at(3) == 700);
	}
}
