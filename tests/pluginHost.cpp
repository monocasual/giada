#ifdef WITH_VST

#include "../src/core/pluginHost.h"
#include "catch.hpp"


TEST_CASE("Test PluginHost class")
{
  PluginHost ph;
  SECTION("test read & write")
  {
    REQUIRE(ph.scanDir() > 0);
  }
}

#endif
