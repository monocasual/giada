#include "src/core/model/channelView.h"
#include <cassert>
#include <utility>

namespace giada::m::model
{
ChannelView::ChannelView(
    const Channel&                             channel,
    std::shared_ptr<const Channel::Parameters> parameters,
    std::shared_ptr<const Channel::Rendering>  rendering)
: m_channel(channel)
, m_parameters(std::move(parameters))
, m_rendering(std::move(rendering))
{
}

/* -------------------------------------------------------------------------- */

ChannelView makeChannelView(const Channel& channel, const Assets& assets)
{
	auto parameters = assets.channelsParameters.find(channel.id);
	auto rendering  = assets.channelsRendering.find(channel.id);

	assert(parameters != nullptr);
	assert(rendering != nullptr);

	return ChannelView(channel, std::move(parameters), std::move(rendering));
}

/* -------------------------------------------------------------------------- */

SampleChannelView::SampleChannelView(
    ChannelView                 channelView,
    Scene                       scene,
    std::shared_ptr<const Wave> wave)
: m_channelView(std::move(channelView))
, m_scene(scene)
, m_wave(std::move(wave))
{
}

/* -------------------------------------------------------------------------- */

const Sample& SampleChannelView::getSample() const
{
	assert(getChannel().sampleChannel.has_value());
	return getChannel().sampleChannel->getSample(m_scene);
}

/* -------------------------------------------------------------------------- */

SampleChannelView makeSampleChannelView(const ChannelView& channelView, const Assets& assets, Scene scene)
{
	const Channel& channel = channelView.getChannel();

	assert(channel.type == ChannelType::SAMPLE);
	assert(channel.sampleChannel.has_value());

	std::shared_ptr<const Wave> wave;
	const ID                    waveId = channel.sampleChannel->getWaveId(scene);
	if (waveId.isValid())
	{
		wave = assets.waves.find(waveId);
		assert(wave != nullptr);
	}

	return SampleChannelView(std::move(channelView), scene, std::move(wave));
}

/* -------------------------------------------------------------------------- */

SampleChannelView makeSampleChannelView(const Channel& channel, const Assets& assets, Scene scene)
{
	return makeSampleChannelView(makeChannelView(channel, assets), assets, scene);
}
} // namespace giada::m::model
