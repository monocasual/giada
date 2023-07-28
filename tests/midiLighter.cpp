#include "../src/core/channels/midiLighter.h"
#include "../src/core/channels/midiLightning.h"
#include "mocks/kernelMidiMock.h"
#include <catch2/catch.hpp>
#include <memory>

TEST_CASE("MidiMapper")
{
	using namespace giada;

	m::KernelMidiMock                kernelMidi;
	m::MidiMapper<m::KernelMidiMock> midiMapper(kernelMidi);
	m::MidiLighter                   midiLighter(midiMapper);
	m::MidiLightning                 midiLightning;

	midiMapper.currentMap = {
	    "test-brand",
	    "test-device",
	    {{0, "0x000000", 0, 0x000000}}, // init commands
	    {0, "0x000001", 0, 0x000001},   // mute on
	    {0, "0x000002", 0, 0x000002},   // mute off
	    {0, "0x000003", 0, 0x000003},   // solo on
	    {0, "0x000004", 0, 0x000004},   // solo off
	    {0, "0x000005", 0, 0x000005},   // waiting
	    {0, "0x000006", 0, 0x000006},   // playing
	    {0, "0x000007", 0, 0x000007},   // stopping
	    {0, "0x000008", 0, 0x000008},   // stopped
	    {0, "0x000009", 0, 0x000009},   // playingInaudible
	};

	midiLighter.onSend = []() {};

	midiLighter.playing = {0x000010, 0};
	midiLighter.mute    = {0x000011, 0};
	midiLighter.solo    = {0x000012, 0};

	SECTION("Test initialization")
	{
		REQUIRE(midiLightning.enabled == false);
	}

	SECTION("Test send OFF status")
	{
		midiLighter.sendStatus(ChannelStatus::OFF, /*audible=*/true);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000008); // Stopped
	}

	SECTION("Test send WAIT status")
	{
		midiLighter.sendStatus(ChannelStatus::WAIT, /*audible=*/true);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000005); // Waiting
	}

	SECTION("Test send ENDING status")
	{
		midiLighter.sendStatus(ChannelStatus::ENDING, /*audible=*/true);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000007); // Stopping
	}

	SECTION("Test send PLAY status")
	{
		midiLighter.sendStatus(ChannelStatus::PLAY, /*audible=*/true);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000006); // Playing

		midiLighter.sendStatus(ChannelStatus::PLAY, /*audible=*/false);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000009); // Playing inaudible
	}

	SECTION("Test send mute")
	{
		midiLighter.sendMute(/*isMuted=*/true);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000001); // Mute on

		midiLighter.sendMute(/*isMuted=*/false);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000002); // Mute off
	}

	SECTION("Test send solo")
	{
		midiLighter.sendSolo(/*isSoloed=*/true);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000003); // Solo on

		midiLighter.sendSolo(/*isSoloed=*/false);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000004); // Solo off
	}
}
