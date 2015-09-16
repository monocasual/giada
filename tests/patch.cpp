#include "../src/core/patch.h"
#include "../src/core/const.h"
#include "catch.hpp"


TEST_CASE("Test Patch class: write patch") 
{
  Patch patch;
  Patch::action_t  action1;
  Patch::action_t  action2;
  //Patch::plugin  plugin; requires WITH_
  Patch::channel_t channel1;
  Patch::channel_t channel2;
  Patch::column_t  column;
  
  action1.type   = 0;
  action1.frame  = 50000;
  action1.fValue = 0.3f;
  action1.iValue = 1000;
  action2.type   = 2;
  action2.frame  = 589;
  action2.fValue = 1.0f;
  action2.iValue = 130;
  
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
  channel1.samplePath        = 0;
  channel1.key               = 0;
  channel1.mode              = 0;
  channel1.begin             = 0;
  channel1.end               = 0;
  channel1.boost             = 0;
  channel1.recActive         = 0;
  channel1.pitch             = 0;
  channel1.midiInReadActions = 0;
  channel1.midiInPitch       = 0;
  channel1.actions.add(action1);
  channel1.actions.add(action2);
  patch.channels.add(channel1);

  column.index = 0;
  column.width = 500;
  column.channels.add(666);
  patch.columns.add(column);
  
  patch.header       = "GPTCH";
  patch.version      = "1.0";
  patch.versionFloat = 1.0f;
  patch.name         = "patch";
  patch.bpm          = 100;
  patch.bars         = 4;
  patch.beats        = 23;
  patch.quantize     = 1;
  patch.masterVolIn  = 1;
  patch.masterVolOut = 1;
  patch.metronome    = 0;
  patch.lastTakeId   = 0;
  patch.samplerate   = 44100;
  
  REQUIRE(1 == 1);
  patch.write("./test-patch", "test-patch", false);
}


