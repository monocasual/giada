#ifndef G_MODEL_MODEL_VIEW_H
#define G_MODEL_MODEL_VIEW_H

#include "src/core/model/channelView.h"
#include "src/core/model/types.h"

namespace giada::m::model
{
class ModelView;

/* makeModelView
Builds a view of the Document, Assets and Parameters.

Keep the Snapshot or RealtimeReadLock alive for as long as you use the view or
anything obtained from it. For example:

    const auto snapshot = model.getSnapshot();
    const auto view = makeModelView(snapshot);

The same applies to a RealtimeReadLock in the audio callback. Temporaries such
as makeModelView(model.getSnapshot()) are not allowed. The model must also
remain alive because the view uses its live Parameters. */

ModelView makeModelView(const RealtimeModel::Snapshot&);
ModelView makeModelView(const RealtimeModel::RealtimeReadLock&);
ModelView makeModelView(const RealtimeModel::Snapshot&&)         = delete;
ModelView makeModelView(const RealtimeModel::RealtimeReadLock&&) = delete;

/* ModelView
Gives read-only access to the Document, Assets and Parameters together.
Parameters can change while the view is in use. See makeModelView above for
how long to keep the data alive. */

class ModelView final
{
public:
	const Document&   getDocument() const { return m_document; }
	const Assets&     getAssets() const { return m_assets; }
	const Parameters& getParameters() const { return m_parameters; }

	/* makeChannelView
	Builds a view of a channel and its associated parameters and rendering.
	Pass a channel from this view's Document and keep it alive while using the
	returned view. Asserts if the associated objects are missing. */

	ChannelView makeChannelView(const Channel&) const;

	/* makeSampleChannelView
	Builds a view of a sample (or preview) channel and its wave for one scene.
	Pass a channel from this view's Document and keep it alive while using the
	returned view. Asserts if required objects are missing. */

	SampleChannelView makeSampleChannelView(const Channel&, Scene) const;
	SampleChannelView makeSampleChannelView(const ChannelView&, Scene) const;

	/* findWave
	Returns the wave used by a channel in the given scene, or nullptr if the wave
	is missing. */

	std::shared_ptr<const Wave> findWave(ID channelId, Scene scene) const;

private:
	friend ModelView makeModelView(const RealtimeModel::Snapshot&);
	friend ModelView makeModelView(const RealtimeModel::RealtimeReadLock&);

	ModelView(const Document& document, const Assets& assets, const Parameters& parameters);

	const Document&   m_document;
	const Assets&     m_assets;
	const Parameters& m_parameters;
};
} // namespace giada::m::model

#endif
