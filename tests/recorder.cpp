#include "../src/core/recorder.h"
#include "../src/core/const.h"
#include "../src/core/types.h"
#include "../src/core/action.h"
#include <catch2/catch.hpp>


TEST_CASE("recorder")
{
	using namespace giada;
	using namespace giada::m;

	recorder::init();

	REQUIRE(recorder::hasActions(/*ch=*/0) == false);

	SECTION("Test record")
	{
		const int       ch = 0;
		const Frame     f1 = 10;
		const Frame     f2 = 70;
		const MidiEvent e1 = MidiEvent(MidiEvent::NOTE_ON, 0x00, 0x00);
		const MidiEvent e2 = MidiEvent(MidiEvent::NOTE_OFF, 0x00, 0x00);

		const Action a1 = recorder::rec(ch, f1, e1);
		const Action a2 = recorder::rec(ch, f2, e2);

		REQUIRE(recorder::hasActions(ch) == true);
		REQUIRE(a1.frame == f1);
		REQUIRE(a2.frame == f2);
		REQUIRE(a1.prevId == 0);
		REQUIRE(a1.nextId == 0);
		REQUIRE(a2.prevId == 0);
		REQUIRE(a2.nextId == 0);

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


		SECTION("Test clear all")
		{
			recorder::clearAll();
			REQUIRE(recorder::hasActions(/*channel=*/0) == false);
		}
	}
}
