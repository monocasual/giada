#include "../src/core/rendering/sampleRendering.h"
#include "../src/core/channels/channel.h"
#include <catch2/catch.hpp>

TEST_CASE("rendering::sampleRendering")
{
	using namespace giada;

	constexpr int BUFFER_SIZE  = 1024;
	constexpr int NUM_CHANNELS = 2;

	// Wave values: [1..BUFFERSIZE*4]
	m::Wave wave(0);
	wave.getBuffer().alloc(BUFFER_SIZE * 4, NUM_CHANNELS);
	wave.getBuffer().forEachFrame([](float* f, int i) {
		f[0] = static_cast<float>(i + 1);
		f[1] = static_cast<float>(i + 1);
	});

	m::ChannelShared channelShared(0, BUFFER_SIZE);
	m::Channel       channel(ChannelType::SAMPLE, 1, channelShared);

	channelShared.quantizer.emplace();
	channelShared.renderQueue.emplace(/*size=*/16);
	channelShared.resampler.emplace(Resampler::Quality::LINEAR, G_MAX_IO_CHANS);

	SECTION("Test initialization")
	{
		REQUIRE(channel.sampleChannel->hasWave() == false);
	}

	SECTION("Test rendering")
	{
		channel.loadWave(&wave);

		REQUIRE(channel.sampleChannel->hasWave() == true);
		REQUIRE(channel.sampleChannel->range.a == 0);
		REQUIRE(channel.sampleChannel->range.b == wave.getBuffer().countFrames());

		REQUIRE(channelShared.tracker.load() == 0);
		REQUIRE(channelShared.playStatus.load() == ChannelStatus::OFF);

		for (const float pitch : {1.0f, 0.5f})
		{
			channel.sampleChannel->pitch = pitch;

			SECTION("Sub-range [M, N), pitch == " + std::to_string(pitch))
			{
				constexpr int RANGE_BEGIN = 16;
				constexpr int RANGE_END   = 48;

				channel.sampleChannel->range = {RANGE_BEGIN, RANGE_END};

				channelShared.renderQueue->enqueue({m::rendering::RenderInfo::Mode::NORMAL, 0});

				m::rendering::renderSampleChannel(channel, /*seqIsRunning=*/false);

				int numFramesWritten = 0;
				channelShared.audioBuffer.forEachFrame([&numFramesWritten](float* f, int) {
					if (f[0] != 0.0)
						numFramesWritten++;
				});

				REQUIRE(numFramesWritten == (RANGE_END - RANGE_BEGIN) / pitch);
			}

			SECTION("Rewind, pitch == " + std::to_string(pitch))
			{
				// Point in audio buffer where the rewind takes place
				const int OFFSET = 256;

				channelShared.renderQueue->enqueue({m::rendering::RenderInfo::Mode::REWIND, OFFSET});

				m::rendering::renderSampleChannel(channel, /*seqIsRunning=*/false);

				// Rendering should start over again at buffer[OFFSET]
				REQUIRE(channelShared.audioBuffer[OFFSET][0] == 1.0f);
			}

			SECTION("Stop, pitch == " + std::to_string(pitch))
			{
				// Point in audio buffer where the stop takes place
				const int OFFSET = 256;

				channelShared.renderQueue->enqueue({m::rendering::RenderInfo::Mode::STOP, OFFSET});

				m::rendering::renderSampleChannel(channel, /*seqIsRunning=*/false);

				int numFramesWritten = 0;
				channelShared.audioBuffer.forEachFrame([&numFramesWritten](float* f, int) {
					if (f[0] != 0.0)
						numFramesWritten++;
				});

				REQUIRE(numFramesWritten == OFFSET);
			}
		}
	}
}
