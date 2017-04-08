#include "../src/core/recorder.h"
#include "catch/single_include/catch.hpp"


using std::string;
using namespace giada;


TEST_CASE("Test Recorder")
{
  SECTION("test record")
  {
    int chan = 0;
    int action = 1;
    int frame = 50;
    uint32_t iValue = 1;
    float fValue = 0.5;
    recorder::rec(chan, action, frame, iValue, fValue);

    REQUIRE(recorder::frames.size() == 1);
    REQUIRE(recorder::frames.at(0) == frame);
    //REQUIRE(recorder::frames.size() == 1);
  }
}
