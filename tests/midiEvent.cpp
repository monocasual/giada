#include "../src/core/midiEvent.h"
#include "../src/utils/math.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("MidiEvent")
{
	using namespace giada::m;

	constexpr uint32_t raw = 0x912C5000; // Note on, channel 1, key 44 (0x2C), velocity 80 (0x50)

	SECTION("Test Channel message")
	{
		MidiEvent e = MidiEvent::makeFromRaw(raw, /*numBytes=*/3, /*timestamp=*/0.0);

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

	SECTION("Test get/set properties")
	{
		MidiEvent e = MidiEvent::makeFromRaw(raw, /*numBytes=*/3, /*timestamp=*/0.0);

		SECTION("Test velocity")
		{
			e.setVelocity(33);
			REQUIRE(e.getChannel() == 1);
			REQUIRE(e.getNote() == 44);
			REQUIRE(e.getVelocity() == 33);
			REQUIRE(e.getVelocityFloat() == u::math::map(33, G_MAX_VELOCITY, G_MAX_VELOCITY_FLOAT));

			e.setVelocityFloat(0.4f);
			REQUIRE(e.getVelocity() == u::math::map(0.4f, G_MAX_VELOCITY_FLOAT, G_MAX_VELOCITY));
			REQUIRE(e.getVelocityFloat() == 0.4f);
		}

		SECTION("Test channel")
		{
			e.setChannel(4);
			REQUIRE(e.getChannel() == 4);
			REQUIRE(e.getNote() == 44);
			REQUIRE(e.getVelocity() == 80);
			REQUIRE(e.getVelocityFloat() == u::math::map(80, G_MAX_VELOCITY, G_MAX_VELOCITY_FLOAT));
		}
	}
}
