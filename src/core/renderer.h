/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/channels/audioReceiver.h"
#include "core/channels/midiController.h"
#include "core/channels/midiReceiver.h"
#include "core/channels/midiSender.h"
#include "core/channels/sampleAdvancer.h"
#include "core/channels/samplePlayer.h"
#include <vector>

namespace mcl
{
class AudioBuffer;
}

namespace giada::m::model
{
class Model;
class Channels;
} // namespace giada::m::model

namespace giada::m
{
class Engine;
class Sequencer;
class Mixer;
class Channel;
class PluginHost;
#ifdef WITH_AUDIO_JACK
class JackSynchronizer;
class JackTransport;
#endif
class Renderer
{
public:
	friend Engine;

#ifdef WITH_AUDIO_JACK
	Renderer(Sequencer&, Mixer&, PluginHost&, JackSynchronizer&, JackTransport&, KernelMidi&);
#else
	Renderer(Sequencer&, Mixer&, PluginHost&, KernelMidi&);
#endif

	void render(mcl::AudioBuffer& out, const mcl::AudioBuffer& in, const model::Model&) const;

private:
	/* advanceChannels
	Processes Channels' static events (e.g. pre-recorded actions or sequencer 
	events) in the current audio block. Called when the sequencer is running. */

	void advanceChannels(const Sequencer::EventBuffer&, const model::Channels&,
	    Range<Frame>, int quantizerStep) const;

	void advanceChannel(const Channel&, const Sequencer::EventBuffer&, Range<Frame>, Frame quantizerStep) const;

	void renderNormalChannels(const std::vector<Channel>& channels, mcl::AudioBuffer& out,
	    mcl::AudioBuffer& in, bool hasSolos, bool seqIsRunning) const;
	void renderNormalChannel(const Channel& ch, mcl::AudioBuffer& out, mcl::AudioBuffer& in, bool mixerHasSolos, bool seqIsRunning) const;
	void renderMasterIn(const Channel&, mcl::AudioBuffer& in) const;
	void renderMasterOut(const Channel&, mcl::AudioBuffer& out) const;
	void renderPreview(const Channel&, mcl::AudioBuffer& out) const;

	Sequencer&  m_sequencer;
	Mixer&      m_mixer;
	PluginHost& m_pluginHost;
#ifdef WITH_AUDIO_JACK
	JackSynchronizer& m_jackSynchronizer;
	JackTransport&    m_jackTransport;
#endif

	AudioReceiver  m_audioReceiver;
	SamplePlayer   m_samplePlayer;
	SampleAdvancer m_sampleAdvancer;
	MidiSender     m_midiSender;
	MidiReceiver   m_midiReceiver;
	MidiController m_midiController;
};
} // namespace giada::m

#endif
