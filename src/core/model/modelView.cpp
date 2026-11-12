#include "src/core/model/modelView.h"

namespace giada::m::model
{
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

const Channel& ModelView::getChannel(ID channelId) const
{
	return m_document.tracks.getChannel(channelId);
}

/* -------------------------------------------------------------------------- */

ModelView makeModelView(const RealtimeModel::Snapshot& snapshot)
{
	return ModelView(snapshot.getDocument(), snapshot.getAssets(), snapshot.getParameters());
}

/* -------------------------------------------------------------------------- */

ModelView makeModelView(const RealtimeModel::RealtimeReadLock& lock)
{
	return ModelView(lock.getDocument(), lock.getAssets(), lock.getParameters());
}
} // namespace giada::m::model
