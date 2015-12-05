#include "../src/core/conf_DEPR_.h"
#include "catch.hpp"


Conf_DEPR_ c;
  
TEST_CASE("Write conf file") 
{
  c.setDefault();
  REQUIRE(c.write() == 1);
  REQUIRE(c.read() == 1);
}

