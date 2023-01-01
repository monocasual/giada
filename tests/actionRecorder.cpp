#include "src/core/actions/actionRecorder.h"
#include "src/core/actions/action.h"
#include "src/core/actions/actions.h"
#include "src/core/channels/channelFactory.h"
#include "src/core/const.h"
#include "src/core/model/model.h"
#include "src/core/types.h"
#include <catch2/catch.hpp>

TEST_CASE("ActionRecorder")
{
	using namespace giada;
	using namespace giada::m;

	const ID channelID1 = 1;
	const ID channelID2 = 2;

	model::Model model;

	model.registerThread(Thread::MAIN, /*realtime=*/false);
	model.reset();

	ChannelFactory::Data channel1 = ChannelFactory::create(channelID1, ChannelType::SAMPLE, 0, 0, 1024, Resampler::Quality::LINEAR, false);
	ChannelFactory::Data channel2 = ChannelFactory::create(channelID2, ChannelType::SAMPLE, 0, 0, 1024, Resampler::Quality::LINEAR, false);

	model.get().channels = {channel1.channel, channel2.channel};
	model.addShared(std::move(channel1.shared));
	model.addShared(std::move(channel2.shared));
	model.swap(model::SwapType::NONE);

	ActionRecorder ar(model);

	REQUIRE(ar.hasActions(channelID1) == false);

	SECTION("Test record")
	{
		const Frame     f1 = 10;
		const Frame     f2 = 70;
		const MidiEvent e1 = MidiEvent::makeFrom3Bytes(MidiEvent::CHANNEL_NOTE_ON, 0x00, 0x00, 0);
		const MidiEvent e2 = MidiEvent::makeFrom3Bytes(MidiEvent::CHANNEL_NOTE_OFF, 0x00, 0x00, 0);

		const Action a1 = ar.rec(channelID1, f1, e1);
		const Action a2 = ar.rec(channelID1, f2, e2);

		REQUIRE(ar.hasActions(channelID1) == true);
		REQUIRE(a1.frame == f1);
		REQUIRE(a2.frame == f2);
		REQUIRE(a1.prevId == 0);
		REQUIRE(a1.nextId == 0);
		REQUIRE(a2.prevId == 0);
		REQUIRE(a2.nextId == 0);

		SECTION("Test clear actions by channel")
		{
			const Frame     f1 = 100;
			const Frame     f2 = 200;
			const MidiEvent e1 = MidiEvent::makeFrom3Bytes(MidiEvent::CHANNEL_NOTE_ON, 0x00, 0x00, 0);
			const MidiEvent e2 = MidiEvent::makeFrom3Bytes(MidiEvent::CHANNEL_NOTE_OFF, 0x00, 0x00, 0);

			ar.rec(channelID2, f1, e1);
			ar.rec(channelID2, f2, e2);

			ar.clearChannel(channelID1);

			REQUIRE(ar.hasActions(channelID1) == false);
			REQUIRE(ar.hasActions(channelID2) == true);
		}

		SECTION("Test clear actions by type")
		{
			ar.clearActions(channelID1, MidiEvent::CHANNEL_NOTE_ON);
			ar.clearActions(channelID1, MidiEvent::CHANNEL_NOTE_OFF);

			REQUIRE(ar.hasActions(channelID1) == false);
		}

		SECTION("Test clear all")
		{
			ar.clearAllActions();
			REQUIRE(ar.hasActions(channelID1) == false);
		}
	}
}
