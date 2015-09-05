#include "../src/utils/utils.h"
#include "catch.hpp"


TEST_CASE("Test gVector template") 
{
  gVector<int> v;
  v.add(0);
  v.add(1);
  v.add(2);
  REQUIRE(v.size == 3);
  
  v.del(0);
  REQUIRE(v.size == 2);
  
  v.swap(0, 1);
  REQUIRE(v.at(0) == 2);
  
  REQUIRE(v.last() == 1);
  
  v.clear();
  REQUIRE(v.size == 0);
  
  /* test copy constructor */
  
  gVector<int> v1;
  v1.add(0);
  v1.add(1);
  v1.add(2);
  gVector<int> v2;
  v2 = v1;
  REQUIRE(v2.size == 3);
  REQUIRE(v2.at(2) == 2); 
}


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

  gVector<std::string> v;
  gSplit("Giada is cool", " ", &v);
  REQUIRE(v.size == 3);
  REQUIRE(v.at(0) == "Giada");
  REQUIRE(v.at(1) == "is");
  REQUIRE(v.at(2) == "cool");
}

