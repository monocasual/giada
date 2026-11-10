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
