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
struct SaveErrorFixture
{
    std::filesystem::path root = std::filesystem::temp_directory_path() /
        ("giada-audio-save-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    SaveErrorFixture() { std::filesystem::create_directory(root); }
    ~SaveErrorFixture() { std::error_code ec; std::filesystem::remove_all(root, ec); }
};
}

TEST_CASE("Failed project audio save restores paths and never publishes a patch", "[audio-save]")
{
    using namespace giada;
    namespace fs = std::filesystem;
    SaveErrorFixture fixture;
    m::model::Model model;
    model.registerThread(Thread::MAIN, false);
    model.init();
    m::KernelAudio audio(model);
    m::KernelMidi midi(model);
    m::MidiMapper<m::KernelMidi> mapper(midi);
    m::ChannelManager channels(model, mapper, midi);
    channels.reset(44100, 256);
    model.get().kernelAudio.samplerate = 44100;
    m::MidiSynchronizer sync(midi);
    m::JackTransport jack;
    m::Sequencer sequencer(model, sync, jack);
    m::Mixer mixer(model);
    m::PluginManager plugins;
    // Constructed only for StorageApi's reference; never initializes devices.
    m::Engine engine;
    m::StorageApi storage(engine, model, plugins, sync, mixer, channels, audio, sequencer);
    model.addWave(m::waveFactory::createEmpty(16, 2, 44100, "first.wav"));
    model.addWave(m::waveFactory::createEmpty(16, 2, 44100, "blocked.wav"));
    // A directory occupying the asset filename is a deterministic I/O failure.
    REQUIRE(fs::create_directory(fixture.root / "blocked.wav"));
    v::Model ui{};
    ui.projectName = "Audio regression";
    const auto patchPath = fixture.root / "Audio regression.gptc";
    SECTION("New project")
    {
        CHECK_FALSE(storage.storeProject(fixture.root.string(), ui, [](float) {}));
        CHECK_FALSE(fs::exists(patchPath));
    }
    SECTION("Existing patch is preserved")
    {
        { std::ofstream existing(patchPath); existing << "previous project"; }
        CHECK_FALSE(storage.storeProject(fixture.root.string(), ui, [](float) {}));
        std::ifstream existing(patchPath);
        std::string text((std::istreambuf_iterator<char>(existing)), {});
        CHECK(text == "previous project");
    }
    CHECK(model.getAllWaves()[0]->getPath() == "first.wav");
    CHECK(model.getAllWaves()[1]->getPath() == "blocked.wav");
    m::Patch unpublished;
    CHECK_FALSE(model.store(unpublished, fixture.root.string()));
    CHECK(unpublished.waves.empty());
    REQUIRE(fs::remove(fixture.root / "blocked.wav"));
    REQUIRE(storage.storeProject(fixture.root.string(), ui, [](float) {}));
    const auto patch = m::patchFactory::deserialize(patchPath.string());
    CHECK(patch.status == G_FILE_OK);
    CHECK(patch.waves.size() == 2);
}
