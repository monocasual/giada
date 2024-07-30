/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/rendering/renderer.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/rendering/midiAdvance.h"
#include "core/rendering/midiOutput.h"
#include "core/rendering/midiReactions.h"
#include "core/rendering/pluginRendering.h"
#include "core/rendering/sampleAdvance.h"
#include "core/rendering/sampleRendering.h"
#ifdef WITH_AUDIO_JACK
#include "core/jackSynchronizer.h"
#include "core/jackTransport.h"
#endif

namespace giada::m::rendering
{
mcl::AudioBuffer::Pan calcPanning_(float pan)
{
	/* TODO - precompute the AudioBuffer::Pan when pan value changes instead of
	building it on the fly. */

	/* Center pan (0.5f)? Pass-through. */

	if (pan == 0.5f)
		return {1.0f, 1.0f};
	return {1.0f - pan, pan};
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

#ifdef WITH_AUDIO_JACK
Renderer::Renderer(Sequencer& s, Mixer& m, PluginHost& ph, JackSynchronizer& js, JackTransport& jt, KernelMidi& km)
#else
Renderer::Renderer(Sequencer& s, Mixer& m, PluginHost& ph, KernelMidi& km)
#endif
: m_sequencer(s)
, m_mixer(m)
, m_pluginHost(ph)
, m_kernelMidi(km)
#ifdef WITH_AUDIO_JACK
, m_jackSynchronizer(js)
, m_jackTransport(jt)
#endif
{
}

/* -------------------------------------------------------------------------- */

void Renderer::render(mcl::AudioBuffer& out, const mcl::AudioBuffer& in, const model::Model& model) const
{
	/* Clean up output buffer before any rendering. Do this even if mixer is
	disabled to avoid audio leftovers during a temporary suspension (e.g. when
	loading a new patch). */

	out.clear();

	/* Prepare the DocumentLock. From this point on (until out of scope) the
	Document is locked for realtime rendering by the audio thread. Rendering
	functions must access the realtime Document coming from documentLock.get(). */

	const model::DocumentLock documentLock = model.get_RT();
	const model::Document&    document_RT  = documentLock.get();
	const model::KernelAudio& kernelAudio  = document_RT.kernelAudio;
	const model::Mixer&       mixer        = document_RT.mixer;
	const model::Sequencer&   sequencer    = document_RT.sequencer;
	const model::Channels&    channels     = document_RT.channels;
	const model::Tracks&      tracks       = document_RT.tracks;
	const model::Actions&     actions      = document_RT.actions;

	/* Mixer disabled or Kernel Audio not ready: nothing to do here. */

	if (!mixer.a_isActive())
		return;

#ifdef WITH_AUDIO_JACK
	if (kernelAudio.api == RtAudio::Api::UNIX_JACK)
		m_jackSynchronizer.recvJackSync(m_jackTransport.getState());
#endif

	/* If the m_sequencer is running, advance it first (i.e. parse it for events).
	Also advance channels (i.e. let them react to m_sequencer events), only if the
	document is not locked: another thread might altering channel's data in the
	meantime (e.g. Plugins or Waves). */

	if (sequencer.isRunning())
	{
		const Frame                currentFrame  = sequencer.a_getCurrentFrame();
		const int                  bufferSize    = out.countFrames();
		const int                  quantizerStep = m_sequencer.getQuantizerStep();            // TODO pass this to m_sequencer.advance - or better, Advancer class
		const geompp::Range<Frame> renderRange   = {currentFrame, currentFrame + bufferSize}; // TODO pass this to m_sequencer.advance - or better, Advancer class

		const Sequencer::EventBuffer& events = m_sequencer.advance(sequencer, bufferSize, kernelAudio.samplerate, actions);
		m_sequencer.render(out, document_RT);
		if (!document_RT.locked)
			advanceChannels(events, channels, renderRange, quantizerStep);
	}

	/* Then render Mixer, channels and finalize output. */

	const int      maxFramesToRec = mixer.inputRecMode == InputRecMode::FREE ? sequencer.getMaxFramesInLoop(kernelAudio.samplerate) : sequencer.framesInLoop;
	const bool     hasSolos       = mixer.hasSolos;
	const bool     hasInput       = in.isAllocd();
	const Channel& masterOutCh    = tracks.getChannel(Mixer::MASTER_OUT_CHANNEL_ID);
	const Channel& masterInCh     = tracks.getChannel(Mixer::MASTER_IN_CHANNEL_ID);
	const Channel& previewCh      = tracks.getChannel(Mixer::PREVIEW_CHANNEL_ID);

	m_mixer.render(in, document_RT, maxFramesToRec);

	if (hasInput)
		renderMasterIn(masterInCh, mixer.getInBuffer());

	if (!document_RT.locked)
		renderNormalChannels(channels.getAll(), out, mixer.getInBuffer(), hasSolos, sequencer.isRunning());

	renderMasterOut(masterOutCh, out);
	if (mixer.renderPreview)
		renderPreview(previewCh, out);

	/* Post processing. */

	m_mixer.finalizeOutput(mixer, out, mixer.inToOut, kernelAudio.limitOutput, masterOutCh.volume);
}

/* -------------------------------------------------------------------------- */

void Renderer::advanceChannels(const Sequencer::EventBuffer& events,
    const model::Channels& channels, geompp::Range<Frame> block, int quantizerStep) const
{
	for (const Channel& c : channels.getAll())
		if (!c.isInternal())
			advanceChannel(c, events, block, quantizerStep);
}

/* -------------------------------------------------------------------------- */

void Renderer::advanceChannel(const Channel& ch, const Sequencer::EventBuffer& events,
    geompp::Range<Frame> block, Frame quantizerStep) const
{
	if (ch.shared->quantizer)
		ch.shared->quantizer->advance(block, quantizerStep);

	for (const Sequencer::Event& e : events)
	{
		if (ch.type == ChannelType::MIDI)
			rendering::advanceMidiChannel(ch, e, m_kernelMidi);
		else if (ch.type == ChannelType::SAMPLE)
			rendering::advanceSampleChannel(ch, e);
	}
}

/* -------------------------------------------------------------------------- */

void Renderer::renderNormalChannels(const std::vector<Channel>& channels, mcl::AudioBuffer& out,
    const mcl::AudioBuffer& in, bool hasSolos, bool seqIsRunning) const
{
	for (const Channel& c : channels)
		if (!c.isInternal())
			renderNormalChannel(c, out, in, hasSolos, seqIsRunning);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderNormalChannel(const Channel& ch, mcl::AudioBuffer& out,
    const mcl::AudioBuffer& in, bool mixerHasSolos, bool seqIsRunning) const
{
	ch.shared->audioBuffer.clear();

	if (ch.type == ChannelType::SAMPLE)
	{
		renderSampleChannel(ch, in, seqIsRunning);
	}
	else if (ch.type == ChannelType::MIDI)
	{
		renderMidiChannel(ch);
	}

	if (ch.isAudible(mixerHasSolos))
		out.sum(ch.shared->audioBuffer, ch.volume * ch.shared->volumeInternal.load(), calcPanning_(ch.pan));
}

/* -------------------------------------------------------------------------- */

void Renderer::renderMasterIn(const Channel& ch, mcl::AudioBuffer& in) const
{
	m_pluginHost.processStack(in, ch.plugins, nullptr);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderMasterOut(const Channel& ch, mcl::AudioBuffer& out) const
{
	ch.shared->audioBuffer.set(out, /*gain=*/1.0f);
	m_pluginHost.processStack(ch.shared->audioBuffer, ch.plugins, nullptr);
	out.set(ch.shared->audioBuffer, ch.volume);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderPreview(const Channel& ch, mcl::AudioBuffer& out) const
{
	ch.shared->audioBuffer.clear();

	if (ch.isPlaying())
		rendering::renderSampleChannel(ch, /*seqIsRunning=*/false); // Sequencer status is irrelevant here

	out.sum(ch.shared->audioBuffer, ch.volume, calcPanning_(ch.pan));
}

/* -------------------------------------------------------------------------- */

void Renderer::renderSampleChannel(const Channel& ch, const mcl::AudioBuffer& in, bool seqIsRunning) const
{
	assert(ch.type == ChannelType::SAMPLE);

	if (ch.isPlaying())
		rendering::renderSampleChannel(ch, seqIsRunning);

	if (ch.canReceiveAudio())
		rendering::renderSampleChannelInput(ch, in); // record "clean" audio first	(i.e. not plugin-processed)

	rendering::renderAudioPlugins(ch, m_pluginHost);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderMidiChannel(const Channel& ch) const
{
	assert(ch.type == ChannelType::MIDI);

	rendering::renderAudioAndMidiPlugins(ch, m_pluginHost);
}
} // namespace giada::m::rendering
