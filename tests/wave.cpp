#include "../src/core/wave.h"
#include "catch/single_include/catch.hpp"


using std::string;


#define G_SAMPLE_RATE 44100
#define G_BUFFER_SIZE 4096


TEST_CASE("Test Wave class")
{
  /* Each SECTION the TEST_CASE is executed from the start. The following
  code is exectuted before each SECTION. */

  Wave w1;

  SECTION("test read & write")
  {
    REQUIRE(w1.open("tests/resources/test.wav") == 1);
    REQUIRE(w1.readData() == 1);
    REQUIRE(w1.getRate() == 44100);
    REQUIRE(w1.getChannels() == 1);
    REQUIRE(w1.getBasename() == "test");
    REQUIRE(w1.getBasename(true) == "test.wav");
    REQUIRE(w1.writeData("test-write.wav") == true);
  }

  SECTION("test copy constructor")
  {
    REQUIRE(w1.open("tests/resources/test.wav") == 1);

    Wave w2(w1);
    REQUIRE(w2.getSize() == w1.getSize());
    REQUIRE(w2.isLogical() == true);
    REQUIRE(w2.getRate() == 44100);
    REQUIRE(w2.getChannels() == 1);
    REQUIRE(w2.writeData("test-write.wav") == true);
  }

  SECTION("test rec")
  {
    REQUIRE(w1.allocEmpty(G_BUFFER_SIZE, G_SAMPLE_RATE) == 1);
    REQUIRE(w1.getSize() == G_BUFFER_SIZE);
    REQUIRE(w1.getRate() == G_SAMPLE_RATE);
    REQUIRE(w1.getChannels() == 2);
    REQUIRE(w1.writeData("test-write.wav") == true);
  }

  SECTION("test resampling")
  {
    REQUIRE(w1.open("tests/resources/test.wav") == 1);
    REQUIRE(w1.readData() == 1);
    REQUIRE(w1.resample(1, G_SAMPLE_RATE / 2) == 1);
  }
}
