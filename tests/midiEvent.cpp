#include "../src/core/midiEvent.h"
#include <catch2/catch.hpp>

TEST_CASE("MidiEvent")
{
	using namespace giada::m;

	SECTION("Test Channel message")
	{
		const uint32_t raw = 0x912C5000; // Note on, channel 1, key 44 (0x2C), velocity 80 (0x50)
		MidiEvent      e   = MidiEvent::makeFromRaw(raw, /*numBytes=*/3, /*timestamp=*/0.0);

		REQUIRE(e.getRaw() == raw);
		REQUIRE(e.getRawNoVelocity() == 0x912C0000);
		REQUIRE(e.getType() == MidiEvent::Type::CHANNEL);
		REQUIRE(e.getNumBytes() == 3);
		REQUIRE(e.getStatus() == 0x90);
		REQUIRE(e.getChannel() == 1);
		REQUIRE(e.getNote() == 44);
		REQUIRE(e.getVelocity() == 80);
		REQUIRE(e.getByte1() == 0x91);
		REQUIRE(e.getByte2() == 0x2C);
		REQUIRE(e.getByte3() == 0x50);
	}
}
