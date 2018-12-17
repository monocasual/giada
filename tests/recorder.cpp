#include "../src/core/recorder.h"
#include "../src/core/const.h"
#include "../src/core/types.h"
#include "../src/core/action.h"
#include <catch.hpp>


TEST_CASE("recorder")
{
	using namespace giada;
	using namespace giada::m;

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, nullptr);

	recorder::init(&mutex);
	recorder::enable();

	REQUIRE(recorder::hasActions(/*ch=*/0) == false);

	SECTION("Test record")
	{
		const int       ch = 0;
		const Frame     f1 = 10;
		const Frame     f2 = 70;
		const MidiEvent e1 = MidiEvent(MidiEvent::NOTE_ON, 0x00, 0x00);
		const MidiEvent e2 = MidiEvent(MidiEvent::NOTE_OFF, 0x00, 0x00);

		const Action* a1 = recorder::rec(ch, f1, e1);
		const Action* a2 = recorder::rec(ch, f2, e2);

		REQUIRE(recorder::hasActions(ch) == true);
		REQUIRE(a1->frame == f1);
		REQUIRE(a2->frame == f2);
		REQUIRE(a1->prev == nullptr);
		REQUIRE(a1->next == nullptr);
		REQUIRE(a2->prev == nullptr);
		REQUIRE(a2->next == nullptr);

		SECTION("Test clear actions by channel")
		{
			const int       ch = 1;
			const Frame     f1 = 100;
			const Frame     f2 = 200;
			const MidiEvent e1 = MidiEvent(MidiEvent::NOTE_ON, 0x00, 0x00);
			const MidiEvent e2 = MidiEvent(MidiEvent::NOTE_OFF, 0x00, 0x00);

			recorder::rec(ch, f1, e1);
			recorder::rec(ch, f2, e2);

			recorder::clearChannel(/*channel=*/0);
			
			REQUIRE(recorder::hasActions(/*channel=*/0) == false);
			REQUIRE(recorder::hasActions(/*channel=*/1) == true);
		}

		SECTION("Test clear actions by type")
		{
			recorder::clearActions(/*channel=*/0, MidiEvent::NOTE_ON);
			recorder::clearActions(/*channel=*/0, MidiEvent::NOTE_OFF);
			
			REQUIRE(recorder::hasActions(/*channel=*/0) == false);
		}

/* TODO -> recorderHandler
		SECTION("Test BPM update")
		{
			REQUIRE(a1->frame == f1);
			REQUIRE(a2->frame == f2);

			recorder::updateBpm(60.0f, 120.0f, 44100);  // scaling up

			REQUIRE(a1->frame == f1 / 2);
			REQUIRE(a2->frame == f2 / 2);

			recorder::updateBpm(120.0f, 60.0f, 44100);  // scaling down

			REQUIRE(a1->frame == f1);
			REQUIRE(a2->frame == f2);
		}

		SECTION("Test samplerate update")
		{
			recorder::updateSamplerate(44100, 22050); // scaling down
			
			REQUIRE(a1->frame == f1 * 2);
			REQUIRE(a2->frame == f2 * 2);

			recorder::updateSamplerate(22050, 44100); // scaling up

			REQUIRE(a1->frame == f1);
			REQUIRE(a2->frame == f2);
		}
*/
	}

	SECTION("Test retrieval")
	{
	}

	SECTION("Test retrieval MIDI")
	{
	}

	SECTION("Test deletion, single action")
	{
	}

	SECTION("Test deletion, range of actions")
	{
	}

	SECTION("Test action presence")
	{
	}

	SECTION("Test clear actions by type")
	{
	}

	SECTION("Test clear all")
	{
	}

	SECTION("Test optimization")
	{
	}

	SECTION("Test samplerate update")
	{
	}
}




#if 0

using std::string;
using namespace giada::m;


TEST_CASE("recorder")
{
	/* Each SECTION the TEST_CASE is executed from the start. The following
	code is exectuted before each SECTION. */

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, nullptr);

	recorder_DEPR_::init();
	REQUIRE(recorder_DEPR_::frames.size() == 0);
	REQUIRE(recorder_DEPR_::global.size() == 0);

	SECTION("Test record single action")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS, 50, 1, 0.5f);

		REQUIRE(recorder_DEPR_::frames.size() == 1);
		REQUIRE(recorder_DEPR_::frames.at(0) == 50);
		REQUIRE(recorder_DEPR_::global.at(0).size() == 1);  // 1 action on frame #0
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->chan == 0);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->frame == 50);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->iValue == 1);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->fValue == 0.5f);
	}

	SECTION("Test record, two actions on same frame")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS, 50, 6, 0.3f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,   50, 1, 0.5f);

		REQUIRE(recorder_DEPR_::frames.size() == 1);    // same frame, frames.size must stay 1
		REQUIRE(recorder_DEPR_::frames.at(0) == 50);
		REQUIRE(recorder_DEPR_::global.at(0).size() == 2);  // 2 actions on frame #0

		REQUIRE(recorder_DEPR_::global.at(0).at(0)->chan == 0);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->frame == 50);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->iValue == 6);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->fValue == 0.3f);

		REQUIRE(recorder_DEPR_::global.at(0).at(1)->chan == 0);
		REQUIRE(recorder_DEPR_::global.at(0).at(1)->type == G_ACTION_KEYREL);
		REQUIRE(recorder_DEPR_::global.at(0).at(1)->frame == 50);
		REQUIRE(recorder_DEPR_::global.at(0).at(1)->iValue == 1);
		REQUIRE(recorder_DEPR_::global.at(0).at(1)->fValue == 0.5f);

		SECTION("Test record, another action on a different frame")
		{
			recorder_DEPR_::rec(0, G_ACTION_KEYPRESS, 70, 1, 0.5f);

			REQUIRE(recorder_DEPR_::frames.size() == 2);
			REQUIRE(recorder_DEPR_::frames.at(1) == 70);
			REQUIRE(recorder_DEPR_::global.at(0).size() == 2);  // 2 actions on frame #0
			REQUIRE(recorder_DEPR_::global.at(1).size() == 1);  // 1 actions on frame #1
			REQUIRE(recorder_DEPR_::global.at(1).at(0)->chan == 0);
			REQUIRE(recorder_DEPR_::global.at(1).at(0)->type == G_ACTION_KEYPRESS);
			REQUIRE(recorder_DEPR_::global.at(1).at(0)->frame == 70);
			REQUIRE(recorder_DEPR_::global.at(1).at(0)->iValue == 1);
			REQUIRE(recorder_DEPR_::global.at(1).at(0)->fValue == 0.5f);
		}
	}

	SECTION("Test retrieval")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS, 50, 6, 0.3f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,   70, 1, 0.5f);
		recorder_DEPR_::rec(1, G_ACTION_KEYPRESS, 50, 6, 0.3f);
		recorder_DEPR_::rec(1, G_ACTION_KEYREL,   70, 1, 0.5f);
		recorder_DEPR_::rec(2, G_ACTION_KEYPRESS, 100, 6, 0.3f);
		recorder_DEPR_::rec(2, G_ACTION_KEYREL,   120, 1, 0.5f);

		/* Give me action on chan 1, type G_ACTION_KEYREL, frame 70. */
		recorder_DEPR_::action *action = nullptr;
		REQUIRE(recorder_DEPR_::getAction(1, G_ACTION_KEYREL, 70, &action) == 1);

		REQUIRE(action != nullptr);
		REQUIRE(action->chan == 1);
		REQUIRE(action->type == G_ACTION_KEYREL);
		REQUIRE(action->frame == 70);
		REQUIRE(action->iValue == 1);
		REQUIRE(action->fValue == 0.5f);

		/* Give me *next* action on chan 0, type G_ACTION_KEYREL, starting from frame 20.
		Must be action #2 */

		REQUIRE(recorder_DEPR_::getNextAction(0, G_ACTION_KEYREL, 20, &action) == 1);
		REQUIRE(action != nullptr);
		REQUIRE(action->chan == 0);
		REQUIRE(action->type == G_ACTION_KEYREL);
		REQUIRE(action->frame == 70);

		/* Give me *next* action on chan 2, type G_ACTION_KEYPRESS, starting from
		frame 200. You are requesting frame outside boundaries. */

		REQUIRE(recorder_DEPR_::getNextAction(2, G_ACTION_KEYPRESS, 200, &action) == -1);

		/* Give me *next* action on chan 2, type G_ACTION_KEYPRESS, starting from
		frame 100. That action does not exist. */

		REQUIRE(recorder_DEPR_::getNextAction(2, G_ACTION_KEYPRESS, 100, &action) == -2);
	}

	SECTION("Test retrieval MIDI")
	{
		recorder_DEPR_::rec(0, G_ACTION_MIDI, 0,    0x903C3F00, 0.0f);
		recorder_DEPR_::rec(1, G_ACTION_MIDI, 0,    0x903D3F00, 0.0f);
		recorder_DEPR_::rec(0, G_ACTION_MIDI, 1000, 0x803C2000, 0.0f);		
		recorder_DEPR_::rec(0, G_ACTION_MIDI, 1050, 0x903C3F00, 0.0f);
		recorder_DEPR_::rec(0, G_ACTION_MIDI, 2000, 0x803C3F00, 0.0f);
		recorder_DEPR_::rec(1, G_ACTION_MIDI, 90,   0x803D3F00, 0.0f);		
		recorder_DEPR_::rec(1, G_ACTION_MIDI, 1050, 0x903D3F00, 0.0f);
		recorder_DEPR_::rec(1, G_ACTION_MIDI, 2000, 0x803D3F00, 0.0f);

		recorder_DEPR_::action* result = nullptr;
		recorder_DEPR_::getNextAction(0, G_ACTION_MIDI, 100, &result, 0x803CFF00, 0x0000FF00);

		REQUIRE(result != nullptr);
		REQUIRE(result->frame == 1000);
	}

	SECTION("Test deletion, single action")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS, 50, 6, 0.3f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,   60, 1, 0.5f);
		recorder_DEPR_::rec(1, G_ACTION_KEYPRESS, 70, 6, 0.3f);
		recorder_DEPR_::rec(1, G_ACTION_KEYREL,   80, 1, 0.5f);

		/* Delete action #0, don't check values. */
		recorder_DEPR_::deleteAction(0, 50, G_ACTION_KEYPRESS, false, &mutex);

		REQUIRE(recorder_DEPR_::frames.size() == 3);
		REQUIRE(recorder_DEPR_::global.size() == 3);

		SECTION("Test deletion checked")
		{
			/* Delete action #1, check values. */
			recorder_DEPR_::deleteAction(1, 70, G_ACTION_KEYPRESS, true, &mutex, 6, 0.3f);

			REQUIRE(recorder_DEPR_::frames.size() == 2);
			REQUIRE(recorder_DEPR_::global.size() == 2);
		}
	}

	SECTION("Test deletion, range of actions")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS, 50, 6, 0.3f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,   60, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS, 70, 6, 0.3f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,   80, 1, 0.5f);
		recorder_DEPR_::rec(1, G_ACTION_KEYPRESS, 100, 6, 0.3f);
		recorder_DEPR_::rec(1, G_ACTION_KEYREL,   120, 1, 0.5f);

		/* Delete any action on channel 0 of types KEYPRESS | KEYREL between
		frames 0 and 200. */

		recorder_DEPR_::deleteActions(0, 0, 200, G_ACTION_KEYPRESS | G_ACTION_KEYREL, &mutex);

		REQUIRE(recorder_DEPR_::frames.size() == 2);
		REQUIRE(recorder_DEPR_::global.size() == 2);
		REQUIRE(recorder_DEPR_::global.at(0).size() == 1);
		REQUIRE(recorder_DEPR_::global.at(1).size() == 1);

		REQUIRE(recorder_DEPR_::global.at(0).at(0)->chan == 1);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->frame == 100);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->iValue == 6);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->fValue == 0.3f);

		REQUIRE(recorder_DEPR_::global.at(1).at(0)->chan == 1);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->type == G_ACTION_KEYREL);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->frame == 120);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->iValue == 1);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->fValue == 0.5f);
	}

	SECTION("Test action presence")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,   80, 1, 0.5f);
		recorder_DEPR_::rec(1, G_ACTION_KEYPRESS, 100, 6, 0.3f);
		recorder_DEPR_::rec(1, G_ACTION_KEYREL,   120, 1, 0.5f);

		recorder_DEPR_::deleteAction(0, 80, G_ACTION_KEYREL, false, &mutex);

		REQUIRE(recorder_DEPR_::hasActions(0) == false);
		REQUIRE(recorder_DEPR_::hasActions(1) == true);
	}

	SECTION("Test clear actions by channel")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,   80, 1, 0.5f);
		recorder_DEPR_::rec(1, G_ACTION_KEYPRESS, 100, 6, 0.3f);
		recorder_DEPR_::rec(1, G_ACTION_KEYREL,   120, 1, 0.5f);

		recorder_DEPR_::clearChan(1);

		REQUIRE(recorder_DEPR_::hasActions(0) == true);
		REQUIRE(recorder_DEPR_::hasActions(1) == false);
		REQUIRE(recorder_DEPR_::frames.size() == 1);
		REQUIRE(recorder_DEPR_::global.size() == 1);
		REQUIRE(recorder_DEPR_::global.at(0).size() == 1);
	}

	SECTION("Test clear actions by type")
	{
		recorder_DEPR_::rec(1, G_ACTION_KEYREL,   80, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS, 100, 6, 0.3f);
		recorder_DEPR_::rec(1, G_ACTION_KEYREL,   120, 1, 0.5f);

		/* Clear all actions of type KEYREL from channel 1. */

		recorder_DEPR_::clearAction(1, G_ACTION_KEYREL);

		REQUIRE(recorder_DEPR_::hasActions(0) == true);
		REQUIRE(recorder_DEPR_::hasActions(1) == false);
		REQUIRE(recorder_DEPR_::frames.size() == 1);
		REQUIRE(recorder_DEPR_::global.size() == 1);
		REQUIRE(recorder_DEPR_::global.at(0).size() == 1);
	}

	SECTION("Test clear all")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS, 0, 1, 0.5f);
		recorder_DEPR_::rec(1, G_ACTION_KEYPRESS, 0, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,   80, 1, 0.5f);
		recorder_DEPR_::rec(1, G_ACTION_KEYREL,   100, 6, 0.3f);
		recorder_DEPR_::rec(2, G_ACTION_KILL, 120, 1, 0.5f);

		recorder_DEPR_::clearAll();
		REQUIRE(recorder_DEPR_::frames.size() == 0);
		REQUIRE(recorder_DEPR_::global.size() == 0);
	}

	SECTION("Test optimization")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS, 20, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,   80, 1, 0.5f);
		recorder_DEPR_::rec(1, G_ACTION_KEYPRESS, 20, 1, 0.5f);
		recorder_DEPR_::rec(1, G_ACTION_KEYREL,   80, 1, 0.5f);

		/* Fake frame 80 without actions.*/
		recorder_DEPR_::global.at(1).clear();

		recorder_DEPR_::optimize();

		REQUIRE(recorder_DEPR_::frames.size() == 1);
		REQUIRE(recorder_DEPR_::global.size() == 1);
		REQUIRE(recorder_DEPR_::global.at(0).size() == 2);
	}

	SECTION("Test BPM update")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,  0, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,   80, 1, 0.5f);

		recorder_DEPR_::updateBpm(60.0f, 120.0f, 44100);  // scaling up

		REQUIRE(recorder_DEPR_::frames.at(0) == 0);
		REQUIRE(recorder_DEPR_::frames.at(1) == 40);

		recorder_DEPR_::updateBpm(120.0f, 60.0f, 44100);  // scaling down

		REQUIRE(recorder_DEPR_::frames.at(0) == 0);
		REQUIRE(recorder_DEPR_::frames.at(1) == 80);
	}

	SECTION("Test samplerate update")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,   0, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,    80, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS, 120, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,   150, 1, 0.5f);

		recorder_DEPR_::updateSamplerate(44100, 22050); // scaling down

		REQUIRE(recorder_DEPR_::frames.at(0) == 0);
		REQUIRE(recorder_DEPR_::frames.at(1) == 160);
		REQUIRE(recorder_DEPR_::frames.at(2) == 240);
		REQUIRE(recorder_DEPR_::frames.at(3) == 300);

		recorder_DEPR_::updateSamplerate(22050, 44100); // scaling up

		REQUIRE(recorder_DEPR_::frames.at(0) == 0);
		REQUIRE(recorder_DEPR_::frames.at(1) == 80);
		REQUIRE(recorder_DEPR_::frames.at(2) == 120);
		REQUIRE(recorder_DEPR_::frames.at(3) == 150);
	}

	SECTION("Test expand")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,   0, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,    80, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KILL,     200, 1, 0.5f);

		recorder_DEPR_::expand(300, 600);

		REQUIRE(recorder_DEPR_::frames.size() == 6);
		REQUIRE(recorder_DEPR_::global.size() == 6);
		REQUIRE(recorder_DEPR_::frames.at(0) == 0);
		REQUIRE(recorder_DEPR_::frames.at(1) == 80);
		REQUIRE(recorder_DEPR_::frames.at(2) == 200);
		REQUIRE(recorder_DEPR_::frames.at(3) == 300);
		REQUIRE(recorder_DEPR_::frames.at(4) == 380);
		REQUIRE(recorder_DEPR_::frames.at(5) == 500);
	}

	SECTION("Test shrink")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,   0, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,    80, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KILL,     200, 1, 0.5f);

		recorder_DEPR_::shrink(100);

		REQUIRE(recorder_DEPR_::frames.size() == 2);
		REQUIRE(recorder_DEPR_::global.size() == 2);
		REQUIRE(recorder_DEPR_::frames.at(0) == 0);
		REQUIRE(recorder_DEPR_::frames.at(1) == 80);
	}
	
	SECTION("Test overdub, full overwrite")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,    0, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL,  80, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,  200, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL, 400, 1, 0.5f);

		/* Should delete all actions in between and keep the first one, plus a
		new last action on frame 500. */
		recorder_DEPR_::startOverdub(0, G_ACTION_KEYPRESS | G_ACTION_KEYREL, 0, 1024);
		recorder_DEPR_::stopOverdub(500, 500, &mutex);

		REQUIRE(recorder_DEPR_::frames.size() == 2);
		REQUIRE(recorder_DEPR_::global.size() == 2);
		REQUIRE(recorder_DEPR_::frames.at(0) == 0);
		REQUIRE(recorder_DEPR_::frames.at(1) == 500);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->frame == 0);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->frame == 500);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->type == G_ACTION_KEYREL);
	}

	SECTION("Test overdub, left overlap")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,  100, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL, 400, 1, 0.5f);

		/* Overdub part of the leftmost part of a composite action. Expected result:
		a new composite action.
		Original:    ----|########|
		Overdub:     |#######|-----
		Result:      |#######|----- */
		recorder_DEPR_::startOverdub(0, G_ACTION_KEYPRESS | G_ACTION_KEYREL, 0, 16);
		recorder_DEPR_::stopOverdub(300, 500, &mutex);

		REQUIRE(recorder_DEPR_::frames.size() == 2);
		REQUIRE(recorder_DEPR_::global.size() == 2);
		REQUIRE(recorder_DEPR_::frames.at(0) == 0);
		REQUIRE(recorder_DEPR_::frames.at(1) == 300);

		REQUIRE(recorder_DEPR_::global.at(0).at(0)->frame == 0);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->frame == 300);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->type == G_ACTION_KEYREL);
	}

	SECTION("Test overdub, right overlap")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,  000, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL, 400, 1, 0.5f);

		/* Overdub part of the rightmost part of a composite action. Expected result:
		a new composite action.
		Original:    |########|------
		Overdub:     -----|#######|--
		Result:      |###||#######|-- */
		recorder_DEPR_::startOverdub(0, G_ACTION_KEYPRESS | G_ACTION_KEYREL, 100, 16);
		recorder_DEPR_::stopOverdub(500, 500, &mutex);

		REQUIRE(recorder_DEPR_::frames.size() == 4);
		REQUIRE(recorder_DEPR_::global.size() == 4);
		REQUIRE(recorder_DEPR_::frames.at(0) == 0);
		REQUIRE(recorder_DEPR_::frames.at(1) == 84); // 100 - bufferSize (16)
		REQUIRE(recorder_DEPR_::frames.at(2) == 100);
		REQUIRE(recorder_DEPR_::frames.at(3) == 500);

		REQUIRE(recorder_DEPR_::global.at(0).at(0)->frame == 0);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->frame == 84);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->type == G_ACTION_KEYREL);

		REQUIRE(recorder_DEPR_::global.at(2).at(0)->frame == 100);
		REQUIRE(recorder_DEPR_::global.at(2).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder_DEPR_::global.at(3).at(0)->frame == 500);
		REQUIRE(recorder_DEPR_::global.at(3).at(0)->type == G_ACTION_KEYREL);
	}

	SECTION("Test overdub, hole diggin'")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,    0, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL, 400, 1, 0.5f);

		/* Overdub in the middle of a long, composite action. Expected result:
		original action trimmed down plus anther action next to it. Total frames
		should be 4.
		Original:    |#############|
		Overdub:     ---|#######|---
		Result:      |#||#######|--- */
		recorder_DEPR_::startOverdub(0, G_ACTION_KEYPRESS | G_ACTION_KEYREL, 100, 16);
		recorder_DEPR_::stopOverdub(300, 500, &mutex);

		REQUIRE(recorder_DEPR_::frames.size() == 4);
		REQUIRE(recorder_DEPR_::global.size() == 4);
		REQUIRE(recorder_DEPR_::frames.at(0) == 0);
		REQUIRE(recorder_DEPR_::frames.at(1) == 84); // 100 - bufferSize (16)
		REQUIRE(recorder_DEPR_::frames.at(2) == 100);
		REQUIRE(recorder_DEPR_::frames.at(3) == 300);

		REQUIRE(recorder_DEPR_::global.at(0).at(0)->frame == 0);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->frame == 84);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->type == G_ACTION_KEYREL);

		REQUIRE(recorder_DEPR_::global.at(2).at(0)->frame == 100);
		REQUIRE(recorder_DEPR_::global.at(2).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder_DEPR_::global.at(3).at(0)->frame == 300);
		REQUIRE(recorder_DEPR_::global.at(3).at(0)->type == G_ACTION_KEYREL);
	}

	SECTION("Test overdub, cover all")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,    0, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL, 100, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,  120, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL, 200, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,  220, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL, 300, 1, 0.5f);

		/* Overdub all existing actions. Expected result: a single composite one. */
		recorder_DEPR_::startOverdub(0, G_ACTION_KEYPRESS | G_ACTION_KEYREL, 0, 16);
		recorder_DEPR_::stopOverdub(500, 500, &mutex);

		REQUIRE(recorder_DEPR_::frames.size() == 2);
		REQUIRE(recorder_DEPR_::global.size() == 2);
		REQUIRE(recorder_DEPR_::frames.at(0) == 0);
		REQUIRE(recorder_DEPR_::frames.at(1) == 500);

		REQUIRE(recorder_DEPR_::global.at(0).at(0)->frame == 0);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->frame == 500);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->type == G_ACTION_KEYREL);
	}

	SECTION("Test overdub, null loop")
	{
		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,    0, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL, 500, 1, 0.5f);

		/* A null loop is a loop that begins and ends on the very same frame. */
		recorder_DEPR_::startOverdub(0, G_ACTION_KEYPRESS | G_ACTION_KEYREL, 300, 16);
		recorder_DEPR_::stopOverdub(300, 700, &mutex);

		REQUIRE(recorder_DEPR_::frames.size() == 2);
		REQUIRE(recorder_DEPR_::frames.at(0) == 0);
		REQUIRE(recorder_DEPR_::frames.at(1) == 284);  // 300 - bufferSize (16)

		REQUIRE(recorder_DEPR_::global.at(0).at(0)->frame == 0);
		REQUIRE(recorder_DEPR_::global.at(0).at(0)->type == G_ACTION_KEYPRESS);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->frame == 284);
		REQUIRE(recorder_DEPR_::global.at(1).at(0)->type == G_ACTION_KEYREL);
	}

	SECTION("Test overdub, ring loop")
	{
		/* A ring loop occurs when you record the last action beyond the end of
		the sequencer.
		Original:    ---|#######|---
		Overdub:     #####|------|##
		Result:      ---|#######||#| */

		recorder_DEPR_::rec(0, G_ACTION_KEYPRESS,  200, 1, 0.5f);
		recorder_DEPR_::rec(0, G_ACTION_KEYREL, 300, 1, 0.5f);

		recorder_DEPR_::startOverdub(0, G_ACTION_KEYPRESS | G_ACTION_KEYREL, 400, 16);
		recorder_DEPR_::stopOverdub(250, 700, &mutex);

		REQUIRE(recorder_DEPR_::frames.size() == 4);
		REQUIRE(recorder_DEPR_::frames.at(0) == 200);
		REQUIRE(recorder_DEPR_::frames.at(1) == 300);
		REQUIRE(recorder_DEPR_::frames.at(2) == 400);
		REQUIRE(recorder_DEPR_::frames.at(3) == 700);
	}
}


#endif