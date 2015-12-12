#include "../src/core/wave.h"
#include "catch.hpp"


using std::string;


#define G_SAMPLE_RATE 44100
#define G_BUFFER_SIZE 4096


TEST_CASE("Test Wave class")
{
  Wave w1;

  SECTION("test read & write")
  {
    REQUIRE(w1.open("test.wav") == 1);
    REQUIRE(w1.readData() == 1);
    REQUIRE(w1.rate() == 11025);
    REQUIRE(w1.channels() == 2);
    REQUIRE(w1.basename() == "test");
    REQUIRE(w1.extension() == "wav");
    REQUIRE(w1.writeData("test-write.wav") == true);
  }

  SECTION("test copy constructor")
  {
    Wave w2(w1);
    REQUIRE(w2.size == w1.size);
    REQUIRE(w2.isLogical == true);
    REQUIRE(w2.rate() == 11025);
    REQUIRE(w2.channels() == 2);
    REQUIRE(w2.writeData("test-write.wav") == true);
  }

  SECTION("test rec")
  {
    Wave w3;
    REQUIRE(w3.allocEmpty(G_BUFFER_SIZE, G_SAMPLE_RATE) == 1);
    REQUIRE(w3.rate() == G_SAMPLE_RATE);
    REQUIRE(w3.channels() == 2);
    REQUIRE(w3.writeData("test-write.wav") == true);
  }
}
