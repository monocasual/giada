#ifndef G_MODEL_MODEL_VIEW_H
#define G_MODEL_MODEL_VIEW_H

#include "src/core/model/channelView.h"
#include "src/core/model/types.h"
#include <optional>

namespace giada::m::model
{
/* ModelView
Gives read-only access to the Document, Assets and Parameters together.
Parameters can change while the view is in use. Keep the Model alive while
using the view. For realtime reads and edits, see Model's lifetime rules. */

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

	/* getWave
	Returns the wave used by a channel in the given scene. Asserts if not found. */

	std::shared_ptr<const Wave> getWave(ID channelId, Scene scene) const;

	/* getPlugin
	Returns the requested plug-in by id. Asserts if not found. */

	std::shared_ptr<const Plugin> getPlugin(ID pluginId) const;

	/* getChannel
	Returns the channel object given its ID. Asserts if not found. */

	const Channel& getChannel(ID channelId) const;

private:
	friend class Model;

	/* ModelView
	Creates a view that keeps its snapshot alive. Used by Model::read(). */

	explicit ModelView(RealtimeModel::Snapshot snapshot);

	ModelView(const Document& document, const Assets& assets, const Parameters& parameters);

	std::optional<RealtimeModel::Snapshot> m_snapshot;
	const Document&                        m_document;
	const Assets&                          m_assets;
	const Parameters&                      m_parameters;
};
} // namespace giada::m::model

#endif
