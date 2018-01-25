#include "../src/core/patch.h"
#include "../src/core/const.h"
#include <catch.hpp>


using std::string;
using std::vector;
using namespace giada::m;


TEST_CASE("Test Patch class")
{
	string filename = "./test-patch.json";

	SECTION("test write")
	{
		patch::action_t  action1;
		patch::action_t  action2;
		patch::channel_t channel1;
		patch::column_t  column;
#ifdef WITH_VST
		patch::plugin_t  plugin1;
		patch::plugin_t  plugin2;
		patch::plugin_t  plugin3;
#endif

		action1.type   = 0;
		action1.frame  = 50000;
		action1.fValue = 0.3f;
		action1.iValue = 1000;
		action2.type   = 2;
		action2.frame  = 589;
		action2.fValue = 1.0f;
		action2.iValue = 130;
		channel1.actions.push_back(action1);
		channel1.actions.push_back(action2);

#ifdef WITH_VST
		plugin1.path   = "/path/to/plugin1";
		plugin1.bypass = false;
		plugin1.params.push_back(0.0f);
		plugin1.params.push_back(0.1f);
		plugin1.params.push_back(0.2f);
		channel1.plugins.push_back(plugin1);

		plugin2.path   = "/another/path/to/plugin2";
		plugin2.bypass = true;
		plugin2.params.push_back(0.6f);
		plugin2.params.push_back(0.6f);
		plugin2.params.push_back(0.6f);
		plugin2.params.push_back(0.0f);
		plugin2.params.push_back(1.0f);
		plugin2.params.push_back(1.0f);
		plugin2.params.push_back(0.333f);
		channel1.plugins.push_back(plugin2);
#endif

		channel1.type              = CHANNEL_SAMPLE;
		channel1.index             = 666;
		channel1.size              = G_GUI_CHANNEL_H_1;
		channel1.column            = 0;
		channel1.mute              = 0;
		channel1.mute_s            = 0;
		channel1.solo              = 0;
		channel1.volume            = 1.0f;
		channel1.pan               = 0.5f;
		channel1.midiIn            = true;
		channel1.midiInKeyPress    = UINT32_MAX;  // check maximum value
		channel1.midiInKeyRel      = 1;
		channel1.midiInKill        = 2;
		channel1.midiInArm         = 11;
		channel1.midiInVolume      = 3;
		channel1.midiInMute        = 4;
		channel1.midiInSolo        = 5;
		channel1.midiOutL          = true;
		channel1.midiOutLplaying   = 7;
		channel1.midiOutLmute      = 8;
		channel1.midiOutLsolo      = 9;
		channel1.samplePath        = "/tmp/test.wav";
		channel1.key               = 666;
		channel1.mode              = 0;
		channel1.begin             = 0;
		channel1.end               = 0;
		channel1.boost             = 0;
		channel1.recActive         = 0;
		channel1.pitch             = 1.2f;
		channel1.midiInReadActions = 0;
		channel1.midiInPitch       = 0;
		channel1.midiOut           = 0;
		channel1.midiOutChan       = 5;
		patch::channels.push_back(channel1);

		column.index = 0;
		column.width = 500;
		patch::columns.push_back(column);

		patch::header       = "GPTCH";
		patch::version      = "1.0";
		patch::versionMajor = 6;
		patch::versionMinor = 6;
		patch::versionPatch = 6;
		patch::name         = "test patch";
		patch::bpm          = 100.0f;
		patch::bars         = 4;
		patch::beats        = 23;
		patch::quantize     = 1;
		patch::masterVolIn  = 1.0f;
		patch::masterVolOut = 0.7f;
		patch::metronome    = 0;
		patch::lastTakeId   = 0;
		patch::samplerate   = 44100;

#ifdef WITH_VST

		patch::masterInPlugins.push_back(plugin1);
		patch::masterOutPlugins.push_back(plugin2);

#endif

		REQUIRE(patch::write(filename) == 1);
	}

	SECTION("test read")
	{
		REQUIRE(patch::read(filename) == PATCH_READ_OK);
		REQUIRE(patch::header == "GPTCH");
		REQUIRE(patch::version == "1.0");
		REQUIRE(patch::versionMajor == 6);
		REQUIRE(patch::versionMinor == 6);
		REQUIRE(patch::versionPatch == 6);
		REQUIRE(patch::name == "test patch");
		REQUIRE(patch::bpm == Approx(100.0f));
		REQUIRE(patch::bars == 4);
		REQUIRE(patch::beats == 23);
		REQUIRE(patch::quantize == 1);
		REQUIRE(patch::masterVolIn == Approx(1.0f));
		REQUIRE(patch::masterVolOut == Approx(0.7f));
		REQUIRE(patch::metronome == 0);
		REQUIRE(patch::lastTakeId == 0);
		REQUIRE(patch::samplerate == 44100);

		patch::column_t column0 = patch::columns.at(0);
		REQUIRE(column0.index == 0);
		REQUIRE(column0.width == 500);

		patch::channel_t channel0 = patch::channels.at(0);
		REQUIRE(channel0.type == CHANNEL_SAMPLE);
		REQUIRE(channel0.index == 666);
		REQUIRE(channel0.size == G_GUI_CHANNEL_H_1);
		REQUIRE(channel0.column == 0);
		REQUIRE(channel0.mute == 0);
		REQUIRE(channel0.mute_s == 0);
		REQUIRE(channel0.solo == 0);
		REQUIRE(channel0.volume == Approx(1.0f));
		REQUIRE(channel0.pan == Approx(0.5f));
		REQUIRE(channel0.midiIn == true);
		REQUIRE(channel0.midiInKeyPress == UINT32_MAX);
		REQUIRE(channel0.midiInKeyRel == 1);
		REQUIRE(channel0.midiInKill == 2);
		REQUIRE(channel0.midiInArm == 11);
		REQUIRE(channel0.midiInVolume == 3);
		REQUIRE(channel0.midiInMute == 4);
		REQUIRE(channel0.midiInSolo == 5);
		REQUIRE(channel0.midiOutL == true);
		REQUIRE(channel0.midiOutLplaying == 7);
		REQUIRE(channel0.midiOutLmute == 8);
		REQUIRE(channel0.midiOutLsolo == 9);
		REQUIRE(channel0.samplePath == "/tmp/test.wav");
		REQUIRE(channel0.key == 666);
		REQUIRE(channel0.mode == 0);
		REQUIRE(channel0.begin == 0);
		REQUIRE(channel0.end == 0);
		REQUIRE(channel0.boost == 1.0f);
		REQUIRE(channel0.recActive == 0);
		REQUIRE(channel0.pitch == Approx(1.2f));
		REQUIRE(channel0.midiInReadActions == 0);
		REQUIRE(channel0.midiInPitch == 0);
		REQUIRE(channel0.midiOut == 0);
		REQUIRE(channel0.midiOutChan == 5);

		patch::action_t action0 = channel0.actions.at(0);
		REQUIRE(action0.type == 0);
		REQUIRE(action0.frame == 50000);
		REQUIRE(action0.fValue == Approx(0.3f));
		REQUIRE(action0.iValue == 1000);

		patch::action_t action1 = channel0.actions.at(1);
		REQUIRE(action1.type == 2);
		REQUIRE(action1.frame == 589);
		REQUIRE(action1.fValue == Approx(1.0f));
		REQUIRE(action1.iValue == 130);

#ifdef WITH_VST
		patch::plugin_t plugin0 = channel0.plugins.at(0);
		REQUIRE(plugin0.path   == "/path/to/plugin1");
		REQUIRE(plugin0.bypass == false);
		REQUIRE(plugin0.params.at(0) == Approx(0.0f));
		REQUIRE(plugin0.params.at(1) == Approx(0.1f));
		REQUIRE(plugin0.params.at(2) == Approx(0.2f));

		patch::plugin_t plugin1 = channel0.plugins.at(1);
		REQUIRE(plugin1.path == "/another/path/to/plugin2");
		REQUIRE(plugin1.bypass == true);
		REQUIRE(plugin1.params.at(0) == Approx(0.6f));
		REQUIRE(plugin1.params.at(1) == Approx(0.6f));
		REQUIRE(plugin1.params.at(2) == Approx(0.6f));
		REQUIRE(plugin1.params.at(3) == Approx(0.0f));
		REQUIRE(plugin1.params.at(4) == Approx(1.0f));
		REQUIRE(plugin1.params.at(5) == Approx(1.0f));
		REQUIRE(plugin1.params.at(6) == Approx(0.333f));

		patch::plugin_t masterPlugin0 = patch::masterInPlugins.at(0);
		REQUIRE(masterPlugin0.path   == "/path/to/plugin1");
		REQUIRE(masterPlugin0.bypass == false);
		REQUIRE(masterPlugin0.params.at(0) == Approx(0.0f));
		REQUIRE(masterPlugin0.params.at(1) == Approx(0.1f));
		REQUIRE(masterPlugin0.params.at(2) == Approx(0.2f));

		patch::plugin_t masterPlugin1 = patch::masterOutPlugins.at(0);
		REQUIRE(masterPlugin1.path == "/another/path/to/plugin2");
		REQUIRE(masterPlugin1.bypass == true);
		REQUIRE(masterPlugin1.params.at(0) == Approx(0.6f));
		REQUIRE(masterPlugin1.params.at(1) == Approx(0.6f));
		REQUIRE(masterPlugin1.params.at(2) == Approx(0.6f));
		REQUIRE(masterPlugin1.params.at(3) == Approx(0.0f));
		REQUIRE(masterPlugin1.params.at(4) == Approx(1.0f));
		REQUIRE(masterPlugin1.params.at(5) == Approx(1.0f));
		REQUIRE(masterPlugin1.params.at(6) == Approx(0.333f));
#endif
	}
}
