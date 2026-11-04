#include "src/core/audioFileIO.h"
#include "src/core/engine.h"
#include "src/core/patchFactory.h"
#include "src/gui/model.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <chrono>

namespace
{
struct WavNamesFixture
{
    std::filesystem::path root = std::filesystem::temp_directory_path() /
        ("giada-audio-save-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    WavNamesFixture() { std::filesystem::create_directory(root); }
    ~WavNamesFixture() { std::error_code ec; std::filesystem::remove_all(root, ec); }
};
}

TEST_CASE("Project audio gets WAV filenames, unique references and reloadable data", "[audio-save]")
{
    using namespace giada;
    namespace fs = std::filesystem;
    WavNamesFixture fixture;
    m::model::Model model;
    model.registerThread(Thread::MAIN, false);
    model.init();
    // These are decoded in-memory waves, with the original imported filenames.
    for (const auto* name : {"same.mp3", "same.flac", "TAKE", "same.wav"})
    {
        auto wave = m::waveFactory::createEmpty(16, 2, 44100, name);
        wave->getBuffer().at(0, 0) = 0.25f;
        model.addWave(std::move(wave));
    }
    auto copy = m::waveFactory::createFromWave(*model.getAllWaves().front());
    model.addWave(std::move(copy));
    m::Patch patch;
    model.store(patch, fixture.root.string());
    REQUIRE(patch.waves.size() == 5);
    std::vector<std::string> names;
    for (const auto& asset : patch.waves)
    {
        CAPTURE(asset.path);
        CHECK(fs::path(asset.path).extension() == ".wav");
        CHECK(std::find(names.begin(), names.end(), asset.path) == names.end());
        names.push_back(asset.path);
        auto wave = m::waveFactory::deserializeWave(
            {asset.id, (fixture.root / asset.path).string()}, 44100, m::Resampler::Quality::LINEAR);
        REQUIRE(wave != nullptr);
        CHECK(wave->getBuffer().countFrames() == 16);
        CHECK(wave->getBuffer().at(0, 0) == 0.25f);
    }
    m::Patch savedAgain;
    model.store(savedAgain, fixture.root.string());
    for (std::size_t i = 0; i < names.size(); ++i)
        CHECK(savedAgain.waves[i].path == names[i]);
}

