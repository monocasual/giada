#ifndef G_MODEL_CHANNEL_VIEW_H
#define G_MODEL_CHANNEL_VIEW_H

#include "src/core/model/assets.h"
#include "src/core/model/tracks.h"
#include "src/core/model/wave.h"
#include "src/scene.h"
#include <memory>

namespace giada::m::model
{
class ChannelView;
class SampleChannelView;

/* makeChannelView
Builds a view for an existing channel. */

ChannelView makeChannelView(const Channel&, const Assets&);

/* makeSampleChannelView
Builds a view for an existing sample channel. */

SampleChannelView makeSampleChannelView(const ChannelView&, const Assets&, Scene);
SampleChannelView makeSampleChannelView(const Channel&, const Assets&, Scene);

/* ChannelView
Groups a channel with its Parameters and Rendering objects. They are stored
separately, so this view lets code use them together. */

class ChannelView final
{
public:
	const Channel&             getChannel() const { return m_channel; }
	const Channel::Parameters& getParameters() const { return *m_parameters; }
	const Channel::Rendering&  getRendering() const { return *m_rendering; }

private:
	friend ChannelView makeChannelView(const Channel&, const Assets&);

	ChannelView(
	    const Channel&                             channel,
	    std::shared_ptr<const Channel::Parameters> parameters,
	    std::shared_ptr<const Channel::Rendering>  rendering);

	const Channel&                             m_channel;
	std::shared_ptr<const Channel::Parameters> m_parameters;
	std::shared_ptr<const Channel::Rendering>  m_rendering;
};

/* SampleChannelView
Groups a ChannelView with one scene's Sample and Wave. Sample channels store a
Wave ID, so this view also provides the Wave needed for rendering. */

class SampleChannelView final
{
public:
	const Channel&             getChannel() const { return m_channelView.getChannel(); }
	const Channel::Parameters& getParameters() const { return m_channelView.getParameters(); }
	const Channel::Rendering&  getRendering() const { return m_channelView.getRendering(); }
	Scene                      getScene() const { return m_scene; }
	const Sample&              getSample() const;
	const Wave*                getWave() const { return m_wave.get(); }
	bool                       hasWave() const { return m_wave != nullptr; }

private:
	friend SampleChannelView makeSampleChannelView(const ChannelView&, const Assets&, Scene);
	friend SampleChannelView makeSampleChannelView(const Channel&, const Assets&, Scene);

	SampleChannelView(ChannelView channelView, Scene scene, std::shared_ptr<const Wave> wave);

	ChannelView                 m_channelView;
	Scene                       m_scene;
	std::shared_ptr<const Wave> m_wave;
};
} // namespace giada::m::model

#endif