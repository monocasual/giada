#include "src/utils/fs.h"
#include "src/core/patchFactory.h"
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <filesystem>

TEST_CASE("Native project paths preserve folder bundles")
{
	namespace fs = std::filesystem;
	using giada::u::fs::getProjectSavePath;
	const fs::path folder = fs::path("Desktop") / "making music";
	REQUIRE(fs::path(getProjectSavePath(folder.string(), "My melody")) == folder / "My melody.gprj");
	REQUIRE(fs::path(getProjectSavePath(folder.string(), "My melody.v2")) == folder / "My melody.v2.gprj");
	REQUIRE(fs::path(getProjectSavePath(folder.string(), "My melody.gprj")) == folder / "My melody.gprj");
	REQUIRE(fs::path(getProjectSavePath((folder / "Existing.gprj").string(), "Untitled")) == folder / "Existing.gprj");
}

TEST_CASE("Project data saves and reopens in a folder containing spaces")
{
	namespace fs = std::filesystem;
	using namespace giada;
	const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
	const fs::path root = fs::temp_directory_path() / ("giada-native-save-test-" + std::to_string(unique));
	REQUIRE(fs::create_directory(root));
	const fs::path bundle = u::fs::getProjectSavePath(root.string(), "My melody.v2");
	REQUIRE(fs::create_directory(bundle));
	const fs::path file = bundle / "My melody.v2.gptc";
	m::Patch original;
	original.name = "My melody.v2";
	original.bpm = 123.0f;
	REQUIRE(m::patchFactory::serialize(original, file.string()));
	const m::Patch reopened = m::patchFactory::deserialize(file.string());
	CHECK(reopened.status == G_FILE_OK);
	CHECK(reopened.name == original.name);
	CHECK(reopened.bpm == original.bpm);
	CHECK(fs::remove(file));
	CHECK(fs::remove(bundle));
	CHECK(fs::remove(root));
}
