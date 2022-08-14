#include "src/core/actions/actionRecorder.h"
#include "src/core/actions/action.h"
#include "src/core/actions/actions.h"
#include "src/core/const.h"
#include "src/core/model/model.h"
#include "src/core/types.h"
#include <catch2/catch.hpp>

TEST_CASE("ActionRecorder")
{
	using namespace giada;
	using namespace giada::m;

	model::Model   model;
	ActionRecorder ar(model);

	REQUIRE(ar.hasActions(/*ch=*/0) == false);

	SECTION("Test record")
	{
		const int       ch = 0;
		const Frame     f1 = 10;
		const Frame     f2 = 70;
		const MidiEvent e1 = MidiEvent(MidiEvent::CHANNEL_NOTE_ON, 0x00, 0x00);
		const MidiEvent e2 = MidiEvent(MidiEvent::CHANNEL_NOTE_OFF, 0x00, 0x00);

		const Action a1 = ar.rec(ch, f1, e1);
		const Action a2 = ar.rec(ch, f2, e2);

		REQUIRE(ar.hasActions(ch) == true);
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
			const MidiEvent e1 = MidiEvent(MidiEvent::CHANNEL_NOTE_ON, 0x00, 0x00);
			const MidiEvent e2 = MidiEvent(MidiEvent::CHANNEL_NOTE_OFF, 0x00, 0x00);

			ar.rec(ch, f1, e1);
			ar.rec(ch, f2, e2);

			ar.clearChannel(/*channel=*/0);

			REQUIRE(ar.hasActions(/*channel=*/0) == false);
			REQUIRE(ar.hasActions(/*channel=*/1) == true);
		}

		SECTION("Test clear actions by type")
		{
			ar.clearActions(/*channel=*/0, MidiEvent::CHANNEL_NOTE_ON);
			ar.clearActions(/*channel=*/0, MidiEvent::CHANNEL_NOTE_OFF);

			REQUIRE(ar.hasActions(/*channel=*/0) == false);
		}

		SECTION("Test clear all")
		{
			ar.clearAllActions();
			REQUIRE(ar.hasActions(/*channel=*/0) == false);
		}
	}
}
