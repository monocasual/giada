#include "../src/utils/fs.h"
#include "../src/utils/math.h"
#include "../src/utils/string.h"
#include <catch2/catch.hpp>

TEST_CASE("u::fs")
{
	using namespace giada;

	REQUIRE(u::fs::fileExists(TEST_RESOURCES_DIR "test.wav") == true);
	REQUIRE(u::fs::fileExists("nonexistent_file") == false);
	REQUIRE(u::fs::dirExists(TEST_RESOURCES_DIR) == true);
	REQUIRE(u::fs::dirExists("ghost_dir/") == false);
	REQUIRE(u::fs::isDir(TEST_RESOURCES_DIR) == true);
	REQUIRE(u::fs::isDir("nonexistent_dir") == false);
	REQUIRE(u::fs::basename("tests/utils.cpp") == "utils.cpp");
	REQUIRE(u::fs::dirname("tests/utils.cpp") == "tests");
	REQUIRE(u::fs::getExt("tests/utils.cpp") == ".cpp");
	REQUIRE(u::fs::stripExt("tests/utils.cpp") == "tests/utils");
#if defined(_WIN32)
	REQUIRE(u::fs::isRootDir("C:\\path\\to\\something") == false);
	REQUIRE(u::fs::getUpDir("C:\\path\\to\\something") == "C:\\path\\to");
	REQUIRE(u::fs::getUpDir("C:\\path") == "C:\\");
#else
	REQUIRE(u::fs::isRootDir("/") == true);
	REQUIRE(u::fs::isRootDir("/path/to/something") == false);
	REQUIRE(u::fs::getUpDir("/path/to/something") == "/path/to");
	REQUIRE(u::fs::getUpDir("/path") == "/");
	REQUIRE(u::fs::getUpDir("/") == "/");
#endif
}

TEST_CASE("u::string")
{
	using namespace giada;

	REQUIRE(u::string::replace("Giada is cool", "cool", "hot") == "Giada is hot");
	REQUIRE(u::string::trim("   Giada is cool       ") == "Giada is cool");

	std::vector<std::string> v = u::string::split("Giada is cool", " ");
	REQUIRE(v.size() == 3);
	REQUIRE(v.at(0) == "Giada");
	REQUIRE(v.at(1) == "is");
	REQUIRE(v.at(2) == "cool");
}

TEST_CASE("::math")
{
	using namespace giada;

	REQUIRE(u::math::map(0.0f, 0.0f, 30.0f, 0.0f, 1.0f) == 0.0f);
	REQUIRE(u::math::map(30.0f, 0.0f, 30.0f, 0.0f, 1.0f) == 1.0f);
	REQUIRE(u::math::map(15.0f, 0.0f, 30.0f, 0.0f, 1.0f) == Approx(0.5f));

	REQUIRE(u::math::map(0.0f, 30.0f, 1.0f) == 0.0f);
	REQUIRE(u::math::map(30.0f, 30.0f, 1.0f) == 1.0f);
	REQUIRE(u::math::map(15.0f, 30.0f, 1.0f) == Approx(0.5f));
}
