#include "../src/core/midiEvent.h"
#include <catch2/catch.hpp>

TEST_CASE("MidiEvent")
{
	using namespace giada::m;

	SECTION("Test Channel messages")
	{
		const uint32_t raw = 0x912C5000; // Note on, channel 1, key 44, velocity 80
		MidiEvent      e(raw);

		REQUIRE(e.getRaw() == raw);
		REQUIRE(e.getStatus() == 0x90);
		REQUIRE(e.getChannel() == 1);
		REQUIRE(e.getNote() == 44);
		REQUIRE(e.getVelocity() == 80);
	}
}
