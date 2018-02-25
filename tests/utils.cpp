#include "../src/utils/fs.h"
#include "../src/utils/string.h"
#include <catch.hpp>


using std::vector;


TEST_CASE("Test filesystem utils")
{
  REQUIRE(gu_fileExists("README.md") == true);
  REQUIRE(gu_fileExists("ghost_file") == false);
  REQUIRE(gu_dirExists("src/") == true);
  REQUIRE(gu_dirExists("ghost_dir/") == false);
  REQUIRE(gu_isDir("src/") == true);
  REQUIRE(gu_isDir("giada_tests") == false);
  REQUIRE(gu_basename("tests/utils.cpp") == "utils.cpp");
  REQUIRE(gu_dirname("tests/utils.cpp") == "tests");
  REQUIRE(gu_getExt("tests/utils.cpp") == "cpp");
  REQUIRE(gu_stripExt("tests/utils.cpp") == "tests/utils");
#if defined(_WIN32)
  REQUIRE(gu_isRootDir("C:\\") == true);
#else
  REQUIRE(gu_isRootDir("/") == true);
#endif
  REQUIRE(gu_isRootDir("/") == true);
  REQUIRE(gu_isRootDir("/path/to/something") == false);
}


TEST_CASE("Test string utils")
{
  REQUIRE(gu_replace("Giada is cool", "cool", "hot") == "Giada is hot");
  REQUIRE(gu_trim("   Giada is cool       ") == "Giada is cool");
  REQUIRE(gu_iToString(666) == "666");
  REQUIRE(gu_iToString(0x99AABB, true) == "99AABB");
  REQUIRE(gu_fToString(3.14159, 2) == "3.14");
  REQUIRE(gu_format("I see %d men with %s hats", 5, "strange") == "I see 5 men with strange hats");

  vector<std::string> v;
  gu_split("Giada is cool", " ", &v);
  REQUIRE(v.size() == 3);
  REQUIRE(v.at(0) == "Giada");
  REQUIRE(v.at(1) == "is");
  REQUIRE(v.at(2) == "cool");
}
