#include "../src/utils/fs.h"
#include "../src/utils/string.h"
#include "../src/utils/math.h"
#include <catch.hpp>


TEST_CASE("u::fs")
{
	using namespace giada::u;

	REQUIRE(fs::fileExists(TEST_RESOURCES_DIR "test.wav") == true);
	REQUIRE(fs::fileExists("nonexistent_file") == false);
	REQUIRE(fs::dirExists(TEST_RESOURCES_DIR) == true);
	REQUIRE(fs::dirExists("ghost_dir/") == false);
	REQUIRE(fs::isDir(TEST_RESOURCES_DIR) == true);
	REQUIRE(fs::isDir("nonexistent_dir") == false);
	REQUIRE(fs::basename("tests/utils.cpp") == "utils.cpp");
	REQUIRE(fs::dirname("tests/utils.cpp") == "tests");
	REQUIRE(fs::getExt("tests/utils.cpp") == ".cpp");
	REQUIRE(fs::stripExt("tests/utils.cpp") == "tests/utils");
#if defined(_WIN32)
	REQUIRE(fs::isRootDir("C:\\") == true);
	REQUIRE(fs::isRootDir("C:\\path\\to\\something") == false);
	REQUIRE(fs::getUpDir("C:\\path\\to\\something") == "C:\\path\\to\\");
	REQUIRE(fs::getUpDir("C:\\path") == "C:\\");
	REQUIRE(fs::getUpDir("C:\\") == "");
#else
	REQUIRE(fs::isRootDir("/") == true);
	REQUIRE(fs::isRootDir("/path/to/something") == false);
	REQUIRE(fs::getUpDir("/path/to/something") == "/path/to");
	REQUIRE(fs::getUpDir("/path") == "/");
	REQUIRE(fs::getUpDir("/") == "/");
#endif
}


TEST_CASE("u::string")
{
	using namespace giada::u;

	REQUIRE(string::replace("Giada is cool", "cool", "hot") == "Giada is hot");
	REQUIRE(string::trim("   Giada is cool       ") == "Giada is cool");
	REQUIRE(string::iToString(666) == "666");
	REQUIRE(string::iToString(0x99AABB, true) == "99AABB");
	REQUIRE(string::fToString(3.14159, 2) == "3.14");
	REQUIRE(string::format("I see %d men with %s hats", 5, "strange") == "I see 5 men with strange hats");

	std::vector<std::string> v = string::split("Giada is cool", " ");
	REQUIRE(v.size() == 3);
	REQUIRE(v.at(0) == "Giada");
	REQUIRE(v.at(1) == "is");
	REQUIRE(v.at(2) == "cool");
}


TEST_CASE("::math")
{
	using namespace giada::u;

	REQUIRE(math::map( 0.0f, 0.0f, 30.0f, 0.0f, 1.0f) == 0.0f);
	REQUIRE(math::map(30.0f, 0.0f, 30.0f, 0.0f, 1.0f) == 1.0f);
	REQUIRE(math::map(15.0f, 0.0f, 30.0f, 0.0f, 1.0f) == Approx(0.5f));
}
