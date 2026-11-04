#include "src/core/channels/channelManager.h"
#include "src/core/model/model.h"
#include "src/core/kernelMidi.h"
#include "src/core/mixer.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Live recording stores audible mono and stereo input", "[audio-recording]")
{
    using namespace giada;
    m::model::Model model;
    model.registerThread(Thread::MAIN, false);
    model.init();
    m::KernelMidi midi(model);
    m::MidiMapper<m::KernelMidi> mapper(midi);
    m::ChannelManager channels(model, mapper, midi);
    channels.reset(44100, 8);
    m::Mixer mixer(model);
    mixer.reset(32, 8);
    mixer.onSignalTresholdReached = [] {};
    mixer.onEndOfRecording = [] {};
    auto& document = model.get();
    document.sequencer.status = SeqStatus::RUNNING;
    document.mixer.isRecordingInput = true;
    channels.setVolume(MASTER_IN_CHANNEL_ID, 0.5f);
    for (int inputChannels : {1, 2})
    {
        CAPTURE(inputChannels);
        mixer.clearRecBuffer();
        document.mixer.a_setInputTracker(0);
        mcl::AudioBuffer input;
        input.alloc(8, inputChannels);
        for (int f = 0; f < 8; ++f)
            for (int c = 0; c < inputChannels; ++c)
                input.at(f, c) = 0.25f * (c + 1);
        mixer.render(input, document, 32);
        CHECK(document.mixer.a_getInputTracker() == 8);
        CHECK(document.mixer.a_getPeakIn().left > 0.0f);
        // A second block must append rather than overwrite the first one.
        mixer.render(input, document, 32);
        CHECK(document.mixer.a_getInputTracker() == 16);
        for (int f = 0; f < 16; ++f)
            for (int c = 0; c < 2; ++c)
                CHECK(mixer.getRecBuffer().at(f, c) == input.at(f % 8, c % inputChannels) * 0.5f);
        CHECK(mixer.getRecBuffer().at(16, 0) == 0.0f);
    }
}

