#include "src/core/model/modelView.h"
#include <utility>

namespace giada::m::model
{
ModelView::ModelView(RealtimeModel::Snapshot snapshot)
: m_snapshot(std::move(snapshot))
, m_document(m_snapshot->getDocument())
, m_assets(m_snapshot->getAssets())
, m_parameters(m_snapshot->getParameters())
{
}

/* -------------------------------------------------------------------------- */

ModelView::ModelView(const Document& document, const Assets& assets, const Parameters& parameters)
: m_document(document)
, m_assets(assets)
, m_parameters(parameters)
{
}

/* -------------------------------------------------------------------------- */

ChannelView ModelView::makeChannelView(const Channel& channel) const
{
	return model::makeChannelView(channel, m_assets);
}

/* -------------------------------------------------------------------------- */

SampleChannelView ModelView::makeSampleChannelView(const Channel& channel, Scene scene) const
{
	return model::makeSampleChannelView(channel, m_assets, scene);
}

/* -------------------------------------------------------------------------- */

SampleChannelView ModelView::makeSampleChannelView(const ChannelView& channelView, Scene scene) const
{
	return model::makeSampleChannelView(channelView, m_assets, scene);
}

/* -------------------------------------------------------------------------- */

std::shared_ptr<const Wave> ModelView::findWave(ID channelId, Scene scene) const
{
	const Channel& channel = m_document.tracks.getChannel(channelId);

	assert(channel.sampleChannel);

	return m_assets.waves.find(channel.sampleChannel->getWaveId(scene));
}

std::shared_ptr<const Wave> ModelView::getWave(ID channelId, Scene scene) const
{
	auto w = findWave(channelId, scene);
	assert(w != nullptr);
	return w;
}

/* -------------------------------------------------------------------------- */

std::shared_ptr<const Plugin> ModelView::getPlugin(ID pluginId) const
{
	return m_assets.plugins.get(pluginId);
}

/* -------------------------------------------------------------------------- */

const Channel& ModelView::getChannel(ID channelId) const
{
	return m_document.tracks.getChannel(channelId);
}
} // namespace giada::m::model
