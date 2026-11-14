/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */

#ifndef G_RENDERER_H
#define G_RENDERER_H

#include "src/core/model/channelView.h"
#include "src/core/model/types.h"
#include "src/core/sequencer.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace mcl
{
class AudioBuffer;
}

namespace giada::m
{
class Engine;
class Sequencer;
class Mixer;
class Channel;
class PluginHost;
class KernelMidi;
#ifdef WITH_AUDIO_JACK
class JackSynchronizer;
class JackTransport;
#endif
} // namespace giada::m

namespace giada::m::model
{
class ModelView;
}

namespace giada::m::model_DEPR_
{
class Model;
class Channels;
class Tracks;
} // namespace giada::m::model_DEPR_

namespace giada::m::rendering
{
class Renderer
{
public:
#ifdef WITH_AUDIO_JACK
	Renderer(Sequencer&, Mixer&, PluginHost&, JackSynchronizer&, JackTransport&, KernelMidi&);
#else
	Renderer(Sequencer&, Mixer&, PluginHost&, KernelMidi&);
#endif

	void render(mcl::AudioBuffer& out, const mcl::AudioBuffer& in, model::RealtimeModel&) const;

private:
	/* advanceTracks
	Processes Channels' static events (e.g. pre-recorded actions or sequencer
	events) in the current audio block. Called when the sequencer is running. */

	void advanceTracks(
	    const Sequencer::EventBuffer&,
	    const model::ModelView&,
	    FrameRange range,
	    Frame      quantizerStep,
	    RenderCommands&) const;

	RenderInfo advanceChannel(
	    const model::ChannelView&,
	    const Sequencer::EventBuffer&,
	    FrameRange,
	    Frame quantizerStep) const;

	void renderTracks(
	    const model::ModelView&,
	    const juce::AudioPlayHead::PositionInfo&,
	    const model::ChannelView& masterOut,
	    mcl::AudioBuffer&         hardwareOut) const;

	void renderNormalChannel(
	    const model::ChannelView&,
	    const model::ModelView&,
	    RenderInfo,
	    const juce::AudioPlayHead::PositionInfo&,
	    const mcl::AudioBuffer& in,
	    Scene,
	    bool seqIsRunning) const;

	void renderMasterOut(
	    const model::ChannelView& channelView,
	    const model::Assets&      assets,
	    const juce::AudioPlayHead::PositionInfo&,
	    mcl::AudioBuffer& out,
	    int               channelOffset) const;

	void renderPreview(const model::SampleChannelView&, mcl::AudioBuffer& out) const;

	void renderSampleChannel(
	    const model::SampleChannelView&,
	    RenderInfo,
	    const mcl::AudioBuffer& in,
	    bool                    seqIsRunning) const;

	/* mergeChannel
	Merges the 'dest' audio buffer with 'src'. */

	void mergeChannel(
	    const model::ChannelView& src,
	    const model::ChannelView& dest) const;

	/* mergeChannel (2)
	Same as above, with a channel offset for the destination buffer 'out'. */

	void mergeChannel(
	    const model::ChannelView& channelView,
	    mcl::AudioBuffer&         out,
	    int                       destChannelOffset) const;

	Sequencer&  m_sequencer;
	Mixer&      m_mixer;
	PluginHost& m_pluginHost;
	KernelMidi& m_kernelMidi;
#ifdef WITH_AUDIO_JACK
	JackSynchronizer& m_jackSynchronizer;
	JackTransport&    m_jackTransport;
#endif

	/* m_renderCommands
	Holds what each channel has to do in the current audio block. Filled by
	advanceTracks(), read by renderTracks(), and emptied when the sequencer is
	stopped. Mutable because render() is const. */

	mutable RenderCommands m_renderCommands;
};
} // namespace giada::m::rendering

#endif
