#include "../src/core/midiEvent.h"
#include <catch2/catch.hpp>

TEST_CASE("MidiEvent")
{
	using namespace giada::m;

	SECTION("Test Channel message")
	{
		const uint32_t raw = 0x912C5000; // Note on, channel 1, key 44 (0x2C), velocity 80 (0x50)
		MidiEvent      e(raw);

		REQUIRE(e.getRaw() == raw);
		REQUIRE(e.getRawNoVelocity() == 0x912C0000);
		REQUIRE(e.getType() == MidiEvent::Type::CHANNEL);
		REQUIRE(e.getStatus() == 0x90);
		REQUIRE(e.getChannel() == 1);
		REQUIRE(e.getNote() == 44);
		REQUIRE(e.getVelocity() == 80);
	}

	SECTION("Test Channel message with float velocity")
	{
		MidiEvent e(0.5f, 0);

		REQUIRE(e.getRaw() == 0xB0000000);
		REQUIRE(e.getRawNoVelocity() == 0xB0000000);
		REQUIRE(e.getType() == MidiEvent::Type::CHANNEL);
		REQUIRE(e.getStatus() == 0xB0);
		REQUIRE(e.getChannel() == 0);
		REQUIRE(e.getNote() == 0);
		REQUIRE(e.getVelocity() == 0);
		REQUIRE(e.getVelocityFloat() == 0.5f);
	}
}
