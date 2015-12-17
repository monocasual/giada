#include "../src/utils/utils.h"
#include "catch.hpp"


using std::vector;


TEST_CASE("Test filesystem utils")
{
  REQUIRE(gFileExists("giada_tests") == true);
  REQUIRE(gFileExists("ghost_file") == false);
  REQUIRE(gDirExists("src/") == true);
  REQUIRE(gDirExists("ghost_dir/") == false);
  REQUIRE(gIsDir("src/") == true);
  REQUIRE(gIsDir("giada_tests") == false);
  REQUIRE(gBasename("tests/utils.cpp") == "utils.cpp");
  REQUIRE(gDirname("tests/utils.cpp") == "tests");
  REQUIRE(gGetExt("tests/utils.cpp") == "cpp");
  REQUIRE(gStripExt("tests/utils.cpp") == "tests/utils");
}


TEST_CASE("Test string utils")
{
  REQUIRE(gReplace("Giada is cool", "cool", "hot") == "Giada is hot");
  REQUIRE(gTrim("   Giada is cool       ") == "Giada is cool");
  REQUIRE(gItoa(666) == "666");

  vector<std::string> v;
  gSplit("Giada is cool", " ", &v);
  REQUIRE(v.size() == 3);
  REQUIRE(v.at(0) == "Giada");
  REQUIRE(v.at(1) == "is");
  REQUIRE(v.at(2) == "cool");
}
