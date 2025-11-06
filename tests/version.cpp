#include "../src/version.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Version")
{
	using namespace giada;

	Version version{1, 0, 0};

	SECTION("Test comparison")
	{
		REQUIRE(version == Version{1, 0, 0});
		REQUIRE(version != Version{0, 0, 0});
		REQUIRE(version != Version{0, 1, 0});
		REQUIRE(version != Version{0, 0, 1});
		REQUIRE(version < Version{1, 0, 1});
		REQUIRE(version < Version{1, 1, 0});
		REQUIRE(version < Version{2, 0, 0});
	}

	SECTION("Test to string")
	{
		REQUIRE(version.toString() == "1.0.0");
	}
}
