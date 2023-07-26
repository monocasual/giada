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

#include "core/renderer.h"
#include "core/model/model.h"
#ifdef WITH_AUDIO_JACK
#include "core/jackSynchronizer.h"
#include "core/jackTransport.h"
#endif

namespace giada::m
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
Renderer::Renderer(Sequencer& s, Mixer& m, PluginHost& ph, JackSynchronizer& js, JackTransport& jt)
#else
Renderer::Renderer(Sequencer& s, Mixer& m, PluginHost& ph)
#endif
: m_sequencer(s)
, m_mixer(m)
, m_pluginHost(ph)
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

	/* Prepare the LayoutLock. From this point on (until out of scope) the
	Layout is locked for realtime rendering by the audio thread. Rendering
	functions must access the realtime layout coming from layoutLock.get(). */

	const model::LayoutLock   layoutLock  = model.get_RT();
	const model::Layout&      layout_RT   = layoutLock.get();
	const model::KernelAudio& kernelAudio = layout_RT.kernelAudio;
	const model::Mixer&       mixer       = layout_RT.mixer;
	const model::Sequencer&   sequencer   = layout_RT.sequencer;
	const model::Channels&    channels    = layout_RT.channels;
	const model::Actions&     actions     = layout_RT.actions;

	/* Mixer disabled or Kernel Audio not ready: nothing to do here. */

	if (!mixer.a_isActive())
		return;

#ifdef WITH_AUDIO_JACK
	if (kernelAudio.api == RtAudio::Api::UNIX_JACK)
		m_jackSynchronizer.recvJackSync(m_jackTransport.getState());
#endif

	/* If the m_sequencer is running, advance it first (i.e. parse it for events).
	Also advance channels (i.e. let them react to m_sequencer events), only if the
	layout is not locked: another thread might altering channel's data in the
	meantime (e.g. Plugins or Waves). */

	if (sequencer.isRunning())
	{
		const Frame        currentFrame  = sequencer.a_getCurrentFrame();
		const int          bufferSize    = out.countFrames();
		const int          quantizerStep = m_sequencer.getQuantizerStep();            // TODO pass this to m_sequencer.advance - or better, Advancer class
		const Range<Frame> renderRange   = {currentFrame, currentFrame + bufferSize}; // TODO pass this to m_sequencer.advance - or better, Advancer class

		const Sequencer::EventBuffer& events = m_sequencer.advance(sequencer, bufferSize, kernelAudio.samplerate, actions);
		m_sequencer.render(out, layout_RT);
		if (!layout_RT.locked)
			m_mixer.advanceChannels(events, channels, renderRange, quantizerStep);
	}

	/* Then render Mixer, channels and finalize output. */

	const int      maxFramesToRec = mixer.inputRecMode == InputRecMode::FREE ? sequencer.getMaxFramesInLoop(kernelAudio.samplerate) : sequencer.framesInLoop;
	const bool     hasSolos       = mixer.hasSolos;
	const bool     hasInput       = in.isAllocd();
	const Channel& masterOutCh    = channels.get(Mixer::MASTER_OUT_CHANNEL_ID);
	const Channel& masterInCh     = channels.get(Mixer::MASTER_IN_CHANNEL_ID);
	const Channel& previewCh      = channels.get(Mixer::PREVIEW_CHANNEL_ID);

	m_mixer.render(out, in, layout_RT, maxFramesToRec);

	if (hasInput)
		renderMasterIn(masterInCh, mixer.getInBuffer());

	if (!layout_RT.locked)
		renderNormalChannels(channels.getAll(), out, mixer.getInBuffer(), hasSolos, sequencer.isRunning());

	renderMasterOut(masterOutCh, out);
	renderPreview(previewCh, out);

	/* Post processing. */

	m_mixer.finalizeOutput(mixer, out, mixer.inToOut, kernelAudio.limitOutput, masterOutCh.volume);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderNormalChannels(const std::vector<Channel>& channels, mcl::AudioBuffer& out,
    mcl::AudioBuffer& in, bool hasSolos, bool seqIsRunning) const
{
	for (const Channel& c : channels)
		if (!c.isInternal())
			renderNormalChannel(c, out, in, hasSolos, seqIsRunning);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderNormalChannel(const Channel& ch, mcl::AudioBuffer& out, mcl::AudioBuffer& in, bool mixerHasSolos, bool seqIsRunning) const
{
	ch.shared->audioBuffer.clear();

	if (ch.samplePlayer && ch.isPlaying())
	{
		SamplePlayer::Render render;
		while (ch.shared->renderQueue->pop(render))
			;
		ch.samplePlayer->render(ch, render, seqIsRunning);
	}

	if (ch.type == ChannelType::SAMPLE)
		m_audioReceiver.render(ch, in);

	/* If MidiReceiver exists, let it process the plug-in stack, as it can
	contain plug-ins that take MIDI events (i.e. synths). Otherwise process the
	plug-in stack internally with no MIDI events. */

	if (ch.midiReceiver)
		ch.midiReceiver->render(*ch.shared, ch.plugins, m_pluginHost);
	else if (ch.plugins.size() > 0)
		m_pluginHost.processStack(ch.shared->audioBuffer, ch.plugins, nullptr);

	if (ch.isAudible(mixerHasSolos))
		out.sum(ch.shared->audioBuffer, ch.volume * ch.volume_i, calcPanning_(ch.pan));
}

/* -------------------------------------------------------------------------- */

void Renderer::renderMasterIn(const Channel& ch, mcl::AudioBuffer& in) const
{
	if (ch.plugins.size() > 0)
		m_pluginHost.processStack(in, ch.plugins, nullptr);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderMasterOut(const Channel& ch, mcl::AudioBuffer& out) const
{
	ch.shared->audioBuffer.set(out, /*gain=*/1.0f);
	if (ch.plugins.size() > 0)
		m_pluginHost.processStack(ch.shared->audioBuffer, ch.plugins, nullptr);
	out.set(ch.shared->audioBuffer, ch.volume);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderPreview(const Channel& ch, mcl::AudioBuffer& out) const
{
	assert(ch.samplePlayer);

	ch.shared->audioBuffer.clear();

	if (ch.isPlaying())
	{
		SamplePlayer::Render render;
		while (ch.shared->renderQueue->pop(render))
			;
		ch.samplePlayer->render(ch, render, /*seqIsRunning=*/false); // Sequencer status is irrelevant here
	}

	out.sum(ch.shared->audioBuffer, ch.volume, calcPanning_(ch.pan));
}
} // namespace giada::m
