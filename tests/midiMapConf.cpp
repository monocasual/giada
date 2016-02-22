#include "../src/core/const.h"
#include "../src/core/midiMapConf.h"
#include "catch.hpp"


using std::string;


TEST_CASE("Test MidiMapConf class")
{
  MidiMapConf midimap;

  SECTION("test default values")
  {
    midimap.setDefault();
    REQUIRE(midimap.brand  == "");
  	REQUIRE(midimap.device == "");
  	REQUIRE(midimap.muteOn.channel    == 0);
  	REQUIRE(midimap.muteOn.valueStr   == "");
  	REQUIRE(midimap.muteOn.offset     == -1);
  	REQUIRE(midimap.muteOn.value      == 0);
  	REQUIRE(midimap.muteOff.channel   == 0);
  	REQUIRE(midimap.muteOff.valueStr  == "");
  	REQUIRE(midimap.muteOff.offset    == -1);
  	REQUIRE(midimap.muteOff.value     == 0);
  	REQUIRE(midimap.soloOn.channel    == 0);
  	REQUIRE(midimap.soloOn.valueStr   == "");
  	REQUIRE(midimap.soloOn.offset     == -1);
  	REQUIRE(midimap.soloOn.value      == 0);
  	REQUIRE(midimap.soloOff.channel   == 0);
  	REQUIRE(midimap.soloOff.valueStr  == "");
  	REQUIRE(midimap.soloOff.offset    == -1);
  	REQUIRE(midimap.soloOff.value     == 0);
  	REQUIRE(midimap.waiting.channel   == 0);
  	REQUIRE(midimap.waiting.valueStr  == "");
  	REQUIRE(midimap.waiting.offset    == -1);
  	REQUIRE(midimap.waiting.value     == 0);
  	REQUIRE(midimap.playing.channel   == 0);
  	REQUIRE(midimap.playing.valueStr  == "");
  	REQUIRE(midimap.playing.offset    == -1);
  	REQUIRE(midimap.playing.value     == 0);
  	REQUIRE(midimap.stopping.channel  == 0);
  	REQUIRE(midimap.stopping.valueStr == "");
  	REQUIRE(midimap.stopping.offset   == -1);
  	REQUIRE(midimap.stopping.value    == 0);
  	REQUIRE(midimap.stopped.channel   == 0);
  	REQUIRE(midimap.stopped.valueStr  == "");
  	REQUIRE(midimap.stopped.offset    == -1);
  	REQUIRE(midimap.stopped.value     == 0);
  }

#ifdef RUN_TESTS_WITH_LOCAL_FILES

  SECTION("test read")
  {
    midimap.init();
    midimap.setDefault();

    /* expect more than 2 midifiles */

    REQUIRE(midimap.maps.size() >= 2);

    /* try with deprecated mode */

    int res = midimap.read("akai-lpd8.giadamap");
    if (res != MIDIMAP_READ_OK)
      res = midimap.readMap_DEPR_("akai-lpd8.giadamap");

    REQUIRE(res == MIDIMAP_READ_OK);

    REQUIRE(midimap.brand == "AKAI");
    REQUIRE(midimap.device == "LPD8");

    REQUIRE(midimap.initCommands.size() == 2);
    REQUIRE(midimap.initCommands[0].channel == 0);
    REQUIRE(midimap.initCommands[0].value == 0xB0000000);
    REQUIRE(midimap.initCommands[1].channel == 0);
    REQUIRE(midimap.initCommands[1].value == 0xB0002800);

    /* TODO - can't check 'valueStr' until deprecated methods are alive */

    REQUIRE(midimap.muteOn.channel == 0);
    //REQUIRE(midimap.muteOn.valueStr == "90nn3F00");
    REQUIRE(midimap.muteOn.offset == 16);
    REQUIRE(midimap.muteOn.value == 0x90003F00);

    REQUIRE(midimap.muteOff.channel == 0);
    //REQUIRE(midimap.muteOff.valueStr == "90nn0C00");
    REQUIRE(midimap.muteOff.offset == 16);
    REQUIRE(midimap.muteOff.value == 0x90000C00);

    REQUIRE(midimap.soloOn.channel == 0);
    //REQUIRE(midimap.soloOn.valueStr == "90nn0F00");
    REQUIRE(midimap.soloOn.offset == 16);
    REQUIRE(midimap.soloOn.value == 0x90000F00);

    REQUIRE(midimap.soloOff.channel == 0);
    //REQUIRE(midimap.soloOff.valueStr == "90nn0C00");
    REQUIRE(midimap.soloOff.offset == 16);
    REQUIRE(midimap.soloOff.value == 0x90000C00);

    REQUIRE(midimap.waiting.channel == 0);
    //REQUIRE(midimap.waiting.valueStr == "90nn7f00");
    REQUIRE(midimap.waiting.offset == 16);
    REQUIRE(midimap.waiting.value == 0x90007f00);

    REQUIRE(midimap.playing.channel == 0);
    //REQUIRE(midimap.playing.valueStr == "90nn7f00");
    REQUIRE(midimap.playing.offset == 16);
    REQUIRE(midimap.playing.value == 0x90007f00);

    REQUIRE(midimap.stopping.channel == 0);
    //REQUIRE(midimap.stopping.valueStr == "90nn7f00");
    REQUIRE(midimap.stopping.offset == 16);
    REQUIRE(midimap.stopping.value == 0x90007f00);

    REQUIRE(midimap.stopped.channel == 0);
    //REQUIRE(midimap.stopped.valueStr == "80nn7f00");
    REQUIRE(midimap.stopped.offset == 16);
    REQUIRE(midimap.stopped.value == 0x80007f00);
  }

#endif // #ifdef RUN_TESTS_WITH_LOCAL_FILES
}
