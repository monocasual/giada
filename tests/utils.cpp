#include "../src/utils/fs.h"
#include "../src/utils/string.h"
#include "../src/utils/math.h"
#include <catch.hpp>


TEST_CASE("u::fs")
{
	using namespace giada::u::fs;

	REQUIRE(fileExists("README.md") == true);
	REQUIRE(fileExists("ghost_file") == false);
	REQUIRE(dirExists("src/") == true);
	REQUIRE(dirExists("ghost_dir/") == false);
	REQUIRE(isDir("src/") == true);
	REQUIRE(isDir("giada_tests") == false);
	REQUIRE(basename("tests/utils.cpp") == "utils.cpp");
	REQUIRE(dirname("tests/utils.cpp") == "tests");
	REQUIRE(getExt("tests/utils.cpp") == "cpp");
	REQUIRE(stripExt("tests/utils.cpp") == "tests/utils");
#if defined(_WIN32)
	REQUIRE(isRootDir("C:\\") == true);
	REQUIRE(isRootDir("C:\\path\\to\\something") == false);
	REQUIRE(getUpDir("C:\\path\\to\\something") == "C:\\path\\to\\");
	REQUIRE(getUpDir("C:\\path") == "C:\\");
	REQUIRE(getUpDir("C:\\") == "");
#else
	REQUIRE(isRootDir("/") == true);
	REQUIRE(isRootDir("/path/to/something") == false);
	REQUIRE(getUpDir("/path/to/something") == "/path/to/");
	REQUIRE(getUpDir("/path") == "/");
	REQUIRE(getUpDir("/") == "/");
#endif
}


TEST_CASE("u::string")
{
	using namespace giada::u::string;

	REQUIRE(replace("Giada is cool", "cool", "hot") == "Giada is hot");
	REQUIRE(trim("   Giada is cool       ") == "Giada is cool");
	REQUIRE(iToString(666) == "666");
	REQUIRE(iToString(0x99AABB, true) == "99AABB");
	REQUIRE(fToString(3.14159, 2) == "3.14");
	REQUIRE(format("I see %d men with %s hats", 5, "strange") == "I see 5 men with strange hats");

	std::vector<std::string> v = split("Giada is cool", " ");
	REQUIRE(v.size() == 3);
	REQUIRE(v.at(0) == "Giada");
	REQUIRE(v.at(1) == "is");
	REQUIRE(v.at(2) == "cool");
}


TEST_CASE("::math")
{
	using namespace giada::u::math;

	REQUIRE(map( 0.0f, 0.0f, 30.0f, 0.0f, 1.0f) == 0.0f);
	REQUIRE(map(30.0f, 0.0f, 30.0f, 0.0f, 1.0f) == 1.0f);
	REQUIRE(map(15.0f, 0.0f, 30.0f, 0.0f, 1.0f) == Approx(0.5f));
}
