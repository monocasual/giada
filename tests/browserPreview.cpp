#include "src/core/api/sampleEditorApi.h"
#include "src/core/channels/channelManager.h"
#include "src/core/actions/ActionManager.h"
#include "src/core/kernelAudio.h"
#include "src/core/kernelMidi.h"
#include "src/core/midiSynchronizer.h"
#include "src/core/jackTransport.h"
#include "src/core/sequencer.h"
#include "src/core/rendering/reactor.h"
#include "src/core/rendering/sampleRendering.h"
#include "src/core/waveFactory.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <chrono>

TEST_CASE("Browser audio previews render without adding waves to the project", "[browser-preview]")
{
    using namespace giada;
    namespace fs = std::filesystem;
    m::model::Model model;
    model.registerThread(Thread::MAIN, false);
    model.init();
    m::KernelAudio audio(model);
    m::KernelMidi midi(model);
    m::MidiMapper<m::KernelMidi> mapper(midi);
    m::MidiSynchronizer sync(midi);
    m::JackTransport jack;
    m::Sequencer sequencer(model, sync, jack);
    m::ActionManager actions(model);
    m::ChannelManager channels(model, mapper, midi);
    m::rendering::Reactor reactor(model, mapper, actions, midi);
    m::SampleEditorApi preview(audio, model, channels, reactor, sequencer);
    model.get().kernelAudio.samplerate = 44100;
    channels.reset(44100, 256);

    const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto file = fs::temp_directory_path() / ("giada-preview-test-" + std::to_string(unique) + ".wav");
    struct RemoveTestFile
    {
        fs::path path;
        ~RemoveTestFile() { std::error_code ignored; fs::remove(path, ignored); }
    } cleanup{file};
    auto wave = m::waveFactory::createEmpty(2048, 2, 44100, "Preview test");
    for (int frame = 0; frame < 2048; ++frame)
        for (int channel = 0; channel < 2; ++channel)
            wave->getBuffer().at(frame, channel) = frame % 2 ? 0.2f : -0.2f;
    REQUIRE(m::waveFactory::save(*wave, file.string()) == G_RES_OK);
    const auto originalWaveCount = model.getAllWaves().size();
    REQUIRE(preview.playPreviewFile(file.string()) == G_RES_OK);
    CHECK(preview.getPreviewStatus() == ChannelStatus::PLAY);
    CHECK(model.getAllWaves().size() == originalWaveCount);
    auto& channel = channels.getChannel(PREVIEW_CHANNEL_ID);
    channel.shared->audioBuffer.clear();
    m::rendering::renderSampleChannel(channel, Scene{0}, false);
    bool audible = false;
    for (int frame = 0; frame < channel.shared->audioBuffer.countFrames(); ++frame)
        audible = audible || std::abs(channel.shared->audioBuffer.at(frame, 0)) > 0.01f;
    CHECK(audible);
    // Let the one-shot reach its natural end without pressing Stop. The browser
    // polls this status during UI refresh to restore its Play preview label.
    for (int block = 0; block < 32 && preview.getPreviewStatus() == ChannelStatus::PLAY; ++block)
    {
        channel.shared->audioBuffer.clear();
        m::rendering::renderSampleChannel(channel, Scene{0}, false);
    }
    CHECK(preview.getPreviewStatus() == ChannelStatus::OFF);
    REQUIRE(preview.playPreviewFile(file.string()) == G_RES_OK);
    CHECK(preview.getPreviewStatus() == ChannelStatus::PLAY);
    preview.freePreviewChannel();
    CHECK_FALSE(model.get().mixer.renderPreview);
    CHECK_FALSE(channels.getChannel(PREVIEW_CHANNEL_ID).sampleChannel->hasWave(Scene{0}));
    CHECK(model.getAllWaves().size() == originalWaveCount);
    CHECK(preview.playPreviewFile((file.string() + ".missing")) != G_RES_OK);
    CHECK_FALSE(model.get().mixer.renderPreview);
}
