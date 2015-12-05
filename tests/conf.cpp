#include "../src/core/const.h"
#include "../src/core/conf.h"
#include "catch.hpp"


using std::string;


TEST_CASE("Test Conf class")
{
  Conf conf;
  conf.createConfigFolder();

  SECTION("test write")
  {
    conf.header = "GIADACONFTEST";
    conf.logMode = 1;
    conf.soundSystem = 2;
    conf.soundDeviceOut = 3;
    conf.soundDeviceIn = 4;
    conf.channelsOut = 5;
    conf.channelsIn = 6;
    conf.samplerate = 7;
    conf.buffersize = 8;
    conf.delayComp = 9;
    conf.limitOutput = true;
    conf.rsmpQuality = 10;
    conf.midiSystem = 11;
    conf.midiPortOut = 12;
    conf.midiPortIn = 13;
    conf.noNoteOff = false;
    conf.midiMapPath = "path/to/midi/map";
    conf.lastFileMap = "path/to/last/midi/map";
    conf.midiSync = 14;
    conf.midiTCfps = 15.1f;
    conf.midiInRewind = 16;
    conf.midiInStartStop = 17;
    conf.midiInActionRec = 18;
    conf.midiInInputRec = 19;
    conf.midiInMetronome = 20;
    conf.midiInVolumeIn = 21;
    conf.midiInVolumeOut = 22;
    conf.midiInBeatDouble = 23;
    conf.midiInBeatHalf = 24;
    conf.recsStopOnChanHalt = true;
    conf.chansStopOnSeqHalt = false;
    conf.treatRecsAsLoops = true;
    conf.resizeRecordings = false;
    conf.pluginPath = "path/to/plugins";
    conf.patchPath = "path/to/patches";
    conf.samplePath = "path/to/samples";
    conf.mainWindowX = 0;
    conf.mainWindowY = 0;
    conf.mainWindowW = 800;
    conf.mainWindowH = 600;
    conf.browserX = 0;
    conf.browserY = 0;
    conf.browserW = 800;
    conf.browserH = 600;
    conf.actionEditorX = 0;
    conf.actionEditorY = 0;
    conf.actionEditorW = 800;
    conf.actionEditorH = 600;
    conf.actionEditorZoom = 1;
    conf.actionEditorGridVal = 10;
    conf.actionEditorGridOn = 1;
    conf.sampleEditorX = 0;
    conf.sampleEditorY = 0;
    conf.sampleEditorW = 800;
    conf.sampleEditorH = 600;
    conf.sampleEditorGridVal = 4;
    conf.sampleEditorGridOn = 0;
    conf.pianoRollY = 0;
    conf.pianoRollH = 900;
    conf.pluginListX = 0;
    conf.pluginListY = 50;
    conf.configX = 20;
    conf.configY = 20;
    conf.bpmX = 30;
    conf.bpmY = 36;
    conf.beatsX = 1;
    conf.beatsY = 1;
    conf.aboutX = 2;
    conf.aboutY = 2;

    REQUIRE(conf.write() == 1);
  }

  SECTION("test read")
  {
    REQUIRE(conf.read() == 1);
    //REQUIRE(conf.header == "GIADACONFTEST");
  }
}
