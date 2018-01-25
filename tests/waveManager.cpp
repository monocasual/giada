#include <memory>
#include "../src/core/waveManager.h"
#include "../src/core/wave.h"
#include "../src/core/const.h"
#include <catch.hpp>


using std::string;
using namespace giada::m;


#define G_SAMPLE_RATE 44100
#define G_BUFFER_SIZE 4096
#define G_CHANNELS 2


TEST_CASE("Test waveManager")
{
  /* Each SECTION the TEST_CASE is executed from the start. Any code between 
  this comment and the first SECTION macro is exectuted before each SECTION. */

  Wave* w;

  SECTION("test creation")
  {
    int res = waveManager::create("tests/resources/test.wav", &w);
    std::unique_ptr<Wave> wave(w);
    
    REQUIRE(res == G_RES_OK);
    REQUIRE(wave->getRate() == G_SAMPLE_RATE);
    REQUIRE(wave->getChannels() == G_CHANNELS);
    REQUIRE(wave->isLogical() == false);
    REQUIRE(wave->isEdited() == false);
  }

  SECTION("test recording")
  {
    int res = waveManager::createEmpty(G_BUFFER_SIZE, G_SAMPLE_RATE, 
      "test.wav", &w);
    std::unique_ptr<Wave> wave(w);

    REQUIRE(res == G_RES_OK);
    REQUIRE(wave->getRate() == G_SAMPLE_RATE);
    REQUIRE(wave->getSize() == G_BUFFER_SIZE / wave->getChannels());
    REQUIRE(wave->getChannels() == G_CHANNELS);
    REQUIRE(wave->isLogical() == true);
    REQUIRE(wave->isEdited() == false);
  }

  SECTION("test resampling")
  {
    int res = waveManager::create("tests/resources/test.wav", &w);
    std::unique_ptr<Wave> wave(w);
    
    REQUIRE(res == G_RES_OK);

    int oldSize = wave->getSize();
    res = waveManager::resample(wave.get(), 1, G_SAMPLE_RATE * 2);
    
    REQUIRE(res == G_RES_OK);
    REQUIRE(wave->getRate() == G_SAMPLE_RATE * 2);
    REQUIRE(wave->getSize() == oldSize * 2);
    REQUIRE(wave->getChannels() == G_CHANNELS);
    REQUIRE(wave->isLogical() == false);
    REQUIRE(wave->isEdited() == false);
  }
}
