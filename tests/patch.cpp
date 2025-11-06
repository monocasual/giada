#include "../src/core/patch.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Patch")
{
	using namespace giada;

	SECTION("version")
	{
		Patch patch;
		patch.version = {0, 16, 0};

		REQUIRE(patch.version < Version{1, 0, 0});
	}
}
