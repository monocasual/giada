#include "../src/core/patch.h"
#include "../src/core/const.h"
#include "catch.hpp"


TEST_CASE("Test Patch class")
{
  Patch patch;

  SECTION("test write")
  {
    Patch::action_t  action1;
    Patch::action_t  action2;
    Patch::channel_t channel1;
    Patch::channel_t channel2;
    Patch::column_t  column;
#ifdef WITH_VST
    Patch::plugin_t  plugin1;
    Patch::plugin_t  plugin2;
    Patch::plugin_t  plugin3;
#endif

    action1.type   = 0;
    action1.frame  = 50000;
    action1.fValue = 0.3f;
    action1.iValue = 1000;
    action2.type   = 2;
    action2.frame  = 589;
    action2.fValue = 1.0f;
    action2.iValue = 130;
    channel1.actions.add(action1);
    channel1.actions.add(action2);

#ifdef WITH_VST
    plugin1.path   = "/path/to/plugin1";
    plugin1.bypass = false;
    plugin1.params.add(0.0f);
    plugin1.params.add(0.1f);
    plugin1.params.add(0.2f);
    channel1.plugins.add(plugin1);

    plugin2.path   = "/another/path/to/plugin2";
    plugin2.bypass = true;
    plugin2.params.add(0.6f);
    plugin2.params.add(0.6f);
    plugin2.params.add(0.6f);
    plugin2.params.add(0.0f);
    plugin2.params.add(1.0f);
    plugin2.params.add(1.0f);
    plugin2.params.add(0.333f);
    channel1.plugins.add(plugin2);
#endif

    channel1.type              = CHANNEL_SAMPLE;
    channel1.index             = 666;
    channel1.column            = 0;
    channel1.mute              = 0;
    channel1.mute_s            = 0;
    channel1.solo              = 0;
    channel1.volume            = 1.0f;
    channel1.panLeft           = 0.5f;
    channel1.panRight          = 0.5f;
    channel1.midiIn            = true;
    channel1.midiInKeyPress    = UINT32_MAX;  // check maximum value
    channel1.midiInKeyRel      = 1;
    channel1.midiInKill        = 2;
    channel1.midiInVolume      = 3;
    channel1.midiInMute        = 4;
    channel1.midiInSolo        = 5;
    channel1.midiOutL          = true;
    channel1.midiOutLplaying   = 7;
    channel1.midiOutLmute      = 8;
    channel1.midiOutLsolo      = 9;
    channel1.samplePath        = "/tmp/test.wav";
    channel1.key               = 0;
    channel1.mode              = 0;
    channel1.begin             = 0;
    channel1.end               = 0;
    channel1.boost             = 0;
    channel1.recActive         = 0;
    channel1.pitch             = 0;
    channel1.midiInReadActions = 0;
    channel1.midiInPitch       = 0;
    channel1.midiOut           = 0;
    channel1.midiOutChan       = 5;
    patch.channels.add(channel1);

    column.index = 0;
    column.width = 500;
    column.channels.add(666);
    column.channels.add(555);
    column.channels.add(123);
    patch.columns.add(column);

    patch.header       = "GPTCH";
    patch.version      = "1.0";
    patch.versionFloat = 1.0f;
    patch.name         = "test patch";
    patch.bpm          = 100;
    patch.bars         = 4;
    patch.beats        = 23;
    patch.quantize     = 1;
    patch.masterVolIn  = 1.0f;
    patch.masterVolOut = 0.2f;
    patch.metronome    = 0;
    patch.lastTakeId   = 0;
    patch.samplerate   = 44100;

    patch.masterInPlugins.add(plugin1);
    patch.masterOutPlugins.add(plugin2);

    REQUIRE(patch.write("./test-patch.json") == 1);
  }

  SECTION("test read")
  {
    REQUIRE(patch.read("./test-patch.json") == 1);
    REQUIRE(patch.header == "GPTCH");
    REQUIRE(patch.version == "1.0");
    REQUIRE(patch.versionFloat == 1.0);
    REQUIRE(patch.name == "test patch");
    REQUIRE(patch.bpm == 100);
    REQUIRE(patch.bars == 4);
    REQUIRE(patch.beats == 23);
    REQUIRE(patch.quantize == 1);
    REQUIRE(patch.masterVolIn == 1.0f);
    REQUIRE(patch.masterVolOut == 0.2f);
    REQUIRE(patch.metronome == 0);
    REQUIRE(patch.lastTakeId == 0);
    REQUIRE(patch.samplerate == 44100);

    Patch::column_t column0 = patch.columns.at(0);
    REQUIRE(column0.index == 0);
    REQUIRE(column0.width == 500);
    REQUIRE(column0.channels.at(0) == 666);
    REQUIRE(column0.channels.at(1) == 555);
    REQUIRE(column0.channels.at(2) == 123);

    Patch::channel_t channel0 = patch.channels.at(0);
    REQUIRE(channel0.type == CHANNEL_SAMPLE);
    REQUIRE(channel0.index == 666);
    REQUIRE(channel0.column == 0);
    REQUIRE(channel0.mute == 0);
    REQUIRE(channel0.mute_s == 0);
    REQUIRE(channel0.solo == 0);
    REQUIRE(channel0.volume == 1.0f);
    REQUIRE(channel0.panLeft == 0.5f);
    REQUIRE(channel0.panRight == 0.5f);
    REQUIRE(channel0.midiIn == true);
    REQUIRE(channel0.midiInKeyPress == UINT32_MAX);
    REQUIRE(channel0.midiInKeyRel == 1);
    REQUIRE(channel0.midiInKill == 2);
    REQUIRE(channel0.midiInVolume == 3);
    REQUIRE(channel0.midiInMute == 4);
    REQUIRE(channel0.midiInSolo == 5);
    REQUIRE(channel0.midiOutL == true);
    REQUIRE(channel0.midiOutLplaying == 7);
    REQUIRE(channel0.midiOutLmute == 8);
    REQUIRE(channel0.midiOutLsolo == 9);
    REQUIRE(channel0.samplePath == "/tmp/test.wav");
    REQUIRE(channel0.key == 0);
    REQUIRE(channel0.mode == 0);
    REQUIRE(channel0.begin == 0);
    REQUIRE(channel0.end == 0);
    REQUIRE(channel0.boost == 0);
    REQUIRE(channel0.recActive == 0);
    REQUIRE(channel0.pitch == 0);
    REQUIRE(channel0.midiInReadActions == 0);
    REQUIRE(channel0.midiInPitch == 0);
    REQUIRE(channel0.midiOut == 0);
    REQUIRE(channel0.midiOutChan == 5);

    Patch::action_t action0 = channel0.actions.at(0);
    REQUIRE(action0.type == 0);
    REQUIRE(action0.frame == 50000);
    REQUIRE(action0.fValue == 0.3f);
    REQUIRE(action0.iValue == 1000);

    Patch::action_t action1 = channel0.actions.at(1);
    REQUIRE(action1.type == 2);
    REQUIRE(action1.frame == 589);
    REQUIRE(action1.fValue == 1.0f);
    REQUIRE(action1.iValue == 130);

#ifdef WITH_VST
    Patch::plugin_t plugin0 = channel0.plugins.at(0);
    REQUIRE(plugin0.path   == "/path/to/plugin1");
    REQUIRE(plugin0.bypass == false);
    REQUIRE(plugin0.params.at(0) == 0.0f);
    REQUIRE(plugin0.params.at(1) == 0.1f);
    REQUIRE(plugin0.params.at(2) == 0.2f);

    Patch::plugin_t plugin1 = channel0.plugins.at(1);
    REQUIRE(plugin1.path == "/another/path/to/plugin2");
    REQUIRE(plugin1.bypass == true);
    REQUIRE(plugin1.params.at(0) == 0.6f);
    REQUIRE(plugin1.params.at(1) == 0.6f);
    REQUIRE(plugin1.params.at(2) == 0.6f);
    REQUIRE(plugin1.params.at(3) == 0.0f);
    REQUIRE(plugin1.params.at(4) == 1.0f);
    REQUIRE(plugin1.params.at(5) == 1.0f);
    REQUIRE(plugin1.params.at(6) == 0.333f);

    Patch::plugin_t masterPlugin0 = patch.masterInPlugins.at(0);
    REQUIRE(masterPlugin0.path   == "/path/to/plugin1");
    REQUIRE(masterPlugin0.bypass == false);
    REQUIRE(masterPlugin0.params.at(0) == 0.0f);
    REQUIRE(masterPlugin0.params.at(1) == 0.1f);
    REQUIRE(masterPlugin0.params.at(2) == 0.2f);

    Patch::plugin_t masterPlugin1 = patch.masterOutPlugins.at(0);
    REQUIRE(masterPlugin1.path == "/another/path/to/plugin2");
    REQUIRE(masterPlugin1.bypass == true);
    REQUIRE(masterPlugin1.params.at(0) == 0.6f);
    REQUIRE(masterPlugin1.params.at(1) == 0.6f);
    REQUIRE(masterPlugin1.params.at(2) == 0.6f);
    REQUIRE(masterPlugin1.params.at(3) == 0.0f);
    REQUIRE(masterPlugin1.params.at(4) == 1.0f);
    REQUIRE(masterPlugin1.params.at(5) == 1.0f);
    REQUIRE(masterPlugin1.params.at(6) == 0.333f);
#endif
  }
}
