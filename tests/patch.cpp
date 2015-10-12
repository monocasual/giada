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
    channel1.midiIn            = 0;
    channel1.midiInKeyPress    = 0;
    channel1.midiInKeyRel      = 0;
    channel1.midiInKill        = 0;
    channel1.midiInVolume      = 0;
    channel1.midiInMute        = 0;
    channel1.midiInSolo        = 0;
    channel1.midiOutL          = 0;
    channel1.midiOutLplaying   = 0;
    channel1.midiOutLmute      = 0;
    channel1.midiOutLsolo      = 0;
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
    patch.channels.add(channel1);

    column.index = 0;
    column.width = 500;
    column.channels.add(666);
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
  }
}
