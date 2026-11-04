// Included by audioFileIO.cpp to test its per-call I/O failure seam.
#include <catch2/catch_test_macros.hpp>

TEST_CASE("WAV writes preserve existing audio on short writes and close errors", "[audio-save]")
{
    using namespace giada;
    using namespace giada::m;
    juce::TemporaryFile fixture;
    const auto path = fixture.getFile().getFullPathName().toStdString();
    mcl::AudioBuffer original(8, 2);
    mcl::AudioBuffer replacement(8, 2);
    for (int f = 0; f < 8; ++f)
        for (int c = 0; c < 2; ++c)
        {
            original.at(f, c) = 0.125f * (c + 1);
            replacement.at(f, c) = -0.25f;
        }
    REQUIRE(audioFileIO::write(path, original, 44100) == G_RES_OK);
    juce::MemoryBlock before;
    REQUIRE(fixture.getFile().loadFileAsData(before));
    SECTION("Short write")
    {
        CHECK(audioFileIO::write_(path, replacement, 44100,
            [](SNDFILE* file, const float* data, sf_count_t frames) -> sf_count_t
            { return sf_writef_float(file, data, frames - 1); }) == G_RES_ERR_IO);
    }
    SECTION("Close failure")
    {
        CHECK(audioFileIO::write_(path, replacement, 44100, sf_writef_float,
            [](SNDFILE* file) { sf_close(file); return 1; }) == G_RES_ERR_IO);
    }
    juce::MemoryBlock after;
    REQUIRE(fixture.getFile().loadFileAsData(after));
    CHECK(after == before);
    auto reopened = audioFileIO::read(path);
    REQUIRE(reopened.has_value());
    CHECK(reopened->countFrames() == 8);
    CHECK(reopened->at(0, 0) == original.at(0, 0));
}

TEST_CASE("WAV writer reports invalid destinations and replaces complete audio", "[audio-save]")
{
    using namespace giada;
    using namespace giada::m;
    juce::TemporaryFile fixture;
    const auto target = fixture.getFile();
    const auto path = target.getFullPathName().toStdString();
    mcl::AudioBuffer buffer(16, 2);
    for (int f = 0; f < 16; ++f)
        for (int c = 0; c < 2; ++c)
            buffer.at(f, c) = (f + c) / 32.0f;
    CHECK(audioFileIO::write("", buffer, 44100) == G_RES_ERR_IO);
    CHECK(audioFileIO::write(path + "/missing/asset.wav", buffer, 44100) == G_RES_ERR_IO);
    REQUIRE(audioFileIO::write(path, buffer, 44100) == G_RES_OK);
    buffer.at(0, 0) = -0.5f;
    REQUIRE(audioFileIO::write(path, buffer, 44100) == G_RES_OK);
    SF_INFO info{};
    audioFileIO::SndFileHandle_ file(sf_open(path.c_str(), SFM_READ, &info));
    REQUIRE(file != nullptr);
    CHECK((info.format & SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV);
    CHECK((info.format & SF_FORMAT_SUBMASK) == SF_FORMAT_FLOAT);
    CHECK(info.frames == 16);
    CHECK(info.channels == 2);
    file.reset();
    auto reopened = audioFileIO::read(path);
    REQUIRE(reopened.has_value());
    for (int f = 0; f < 16; ++f)
        for (int c = 0; c < 2; ++c)
            CHECK(reopened->at(f, c) == buffer.at(f, c));
}
