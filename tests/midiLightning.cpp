#include "../src/core/channels/midiLightning.h"
#include "../src/core/rendering/midiOutput.h"
#include "mocks/kernelMidiMock.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>

TEST_CASE("MidiLightning")
{
	using namespace giada;

	m::KernelMidiMock                kernelMidi;
	m::MidiMapper<m::KernelMidiMock> midiMapper(kernelMidi);
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

	m::rendering::registerOnSendMidiCb([](ID) {});

	midiLightning.playing = {0x000010, 0};
	midiLightning.mute    = {0x000011, 0};
	midiLightning.solo    = {0x000012, 0};

	SECTION("Test initialization")
	{
		REQUIRE(midiLightning.enabled == false);
	}

	SECTION("Test send OFF status")
	{
		m::rendering::sendMidiLightningStatus(0, midiLightning, ChannelStatus::OFF, /*audible=*/true, midiMapper);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000008); // Stopped
	}

	SECTION("Test send WAIT status")
	{
		m::rendering::sendMidiLightningStatus(0, midiLightning, ChannelStatus::WAIT, /*audible=*/true, midiMapper);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000005); // Waiting
	}

	SECTION("Test send ENDING status")
	{
		m::rendering::sendMidiLightningStatus(0, midiLightning, ChannelStatus::ENDING, /*audible=*/true, midiMapper);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000007); // Stopping
	}

	SECTION("Test send PLAY status")
	{
		m::rendering::sendMidiLightningStatus(0, midiLightning, ChannelStatus::PLAY, /*audible=*/true, midiMapper);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000006); // Playing

		m::rendering::sendMidiLightningStatus(0, midiLightning, ChannelStatus::PLAY, /*audible=*/false, midiMapper);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000009); // Playing inaudible
	}

	SECTION("Test send mute")
	{
		m::rendering::sendMidiLightningMute(0, midiLightning, /*isMuted=*/true, midiMapper);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000001); // Mute on

		m::rendering::sendMidiLightningMute(0, midiLightning, /*isMuted=*/false, midiMapper);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000002); // Mute off
	}

	SECTION("Test send solo")
	{
		m::rendering::sendMidiLightningSolo(0, midiLightning, /*isSoloed=*/true, midiMapper);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000003); // Solo on

		m::rendering::sendMidiLightningSolo(0, midiLightning, /*isSoloed=*/false, midiMapper);
		REQUIRE(kernelMidi.sent.back().getRaw() == 0x000004); // Solo off
	}
}
