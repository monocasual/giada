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

#include "src/core/rendering/renderer.h"
#include "src/core/mixer.h"
#include "src/core/model_DEPR_/model.h"
#include "src/core/rendering/midiAdvance.h"
#include "src/core/rendering/midiOutput.h"
#include "src/core/rendering/midiReactions.h"
#include "src/core/rendering/pluginRendering.h"
#include "src/core/rendering/sampleAdvance.h"
#include "src/core/rendering/sampleRendering.h"
#include <chrono>
#ifdef WITH_AUDIO_JACK
#include "src/core/jackSynchronizer.h"
#include "src/core/jackTransport.h"
#endif

namespace giada::m::rendering
{
namespace
{
using Now      = std::chrono::time_point<std::chrono::high_resolution_clock>;
using Duration = std::chrono::duration<double, std::micro>;

/* -------------------------------------------------------------------------- */

double computeCpuLoad_(const Now& startTime, unsigned int sampleRate, int bufferSize)
{
	const Now      endTime        = std::chrono::high_resolution_clock::now();
	const Duration processingTime = endTime - startTime;

	const double callbackDuration = (static_cast<double>(bufferSize) / sampleRate) * 1e6; // in microseconds
	const double load             = (processingTime.count() / callbackDuration) * 100.0;

	return load;
}

/* -------------------------------------------------------------------------- */

RenderInfo findRenderInfo_(const RenderCommands& commands, ID channelId)
{
	for (const auto& [id, info] : commands)
		if (id == channelId)
			return info;
	return {RenderInfo::Mode::NONE};
}
} // namespace

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
	/* Reserve enough space for the render commands so that we never allocate
	on the real-time thread. We assume here maximum 512 channels per project,
	it feels enough for now. */

	m_renderCommands.reserve(512);
}

/* -------------------------------------------------------------------------- */

void Renderer::render(mcl::AudioBuffer& out, const mcl::AudioBuffer& in, model::Model& model) const
{
	const Now startTime = std::chrono::high_resolution_clock::now();

	/* Clean up output buffer before any rendering. Do this even if mixer is
	disabled to avoid audio leftovers during a temporary suspension (e.g. when
	loading a new patch). */

	out.clear();

	const model::RealtimeLock lock   = model.read();
	const model::Document&    doc    = lock.getDocument();
	const model::Assets&      assets = lock.getAssets();
	const model::Parameters&  params = lock.getParameters();

	/* Mixer disabled or Kernel Audio not ready: nothing to do here. */

	if (!params.mixer.isActive())
		return;

	const int bufferSize = out.countFrames();

#ifdef WITH_AUDIO_JACK
	if (kernelAudio.api == RtAudio::Api::UNIX_JACK)
		m_jackSynchronizer.recvJackSync(m_jackTransport.getState());
#endif

	/* If the sequencer is running, advance it first (i.e. parse it for events).
	Also advance channels (i.e. let them react to sequencer events). */

	if (doc.sequencer.isRunning())
	{
		const Frame      currentFrame  = params.sequencer.getCurrentFrame();
		const int        quantizerStep = m_sequencer.getQuantizerStep();            // TODO pass this to m_sequencer.advance - or better, Advancer class
		const FrameRange renderRange   = {currentFrame, currentFrame + bufferSize}; // TODO pass this to m_sequencer.advance - or better, Advancer class

		const Sequencer::EventBuffer& events = m_sequencer.advance(doc.sequencer, params.sequencer, doc.actions, bufferSize);
		advanceTracks(events, doc.tracks, assets, renderRange, quantizerStep, m_renderCommands);

		if (doc.sequencer.metronome)
			m_sequencer.render(out);
	}
	else
		m_renderCommands.clear(); // Don't reuse stale commands from the previous block

	/* Then render Mixer, channels and finalize output. */

	const int             sampleRate     = doc.kernelAudio.samplerate;
	const int             maxFramesToRec = doc.mixer.inputRecMode == InputRecMode::FREE ? doc.sequencer.getMaxFramesInLoop(sampleRate) : doc.sequencer.getFramesInLoop(sampleRate);
	const Scene           scene          = params.sequencer.getCurrentScene();
	const bool            hasSolos       = doc.mixer.hasSolos;
	const bool            hasInput       = in.isAllocd();
	const model::Channel& masterOutCh    = doc.tracks.getChannel(MASTER_OUT_CHANNEL_ID);
	const model::Channel& masterInCh     = doc.tracks.getChannel(MASTER_IN_CHANNEL_ID);
	const model::Channel& previewCh      = doc.tracks.getChannel(PREVIEW_CHANNEL_ID);

	// TODO
	// m_mixer.render(in, document_RT, maxFramesToRec);

	// TODO
	// if (hasInput)
	//	renderMasterIn(masterInCh, mixer.getInBuffer());

	// Render tracks

	const auto masterOutRendering = assets.channelsRendering.find(MASTER_OUT_CHANNEL_ID);
	const auto mixerRendering     = assets.mixerRendering;
	assert(masterOutRendering != nullptr);

	renderTracks(doc.tracks, assets, *masterOutRendering, out, assets.mixerRendering->getInBuffer(),
	    scene, hasSolos, doc.sequencer.isRunning());

	renderMasterOut(masterOutCh, *masterOutRendering, out, doc.kernelAudio.deviceOut.channelsStart);

	// TODO
	// if (mixer.renderPreview)
	//	renderPreview(previewCh, out);

	m_mixer.updateOutputPeak(params.mixer, masterOutRendering->getAudioBuffer());

	/* Post processing. */

	m_mixer.finalizeOutput(mixerRendering->getInBuffer(), out, doc.mixer.inToOut,
	    doc.kernelAudio.limitOutput, masterOutCh.volume);

	/* CPU load computation. */

	params.kernelAudio.setCpuLoad(computeCpuLoad_(startTime, doc.kernelAudio.samplerate, bufferSize));
}

/* -------------------------------------------------------------------------- */

void Renderer::advanceTracks(
    const Sequencer::EventBuffer& events,
    const model::Tracks&          tracks,
    const model::Assets&          assets,
    FrameRange                    block,
    Frame                         quantizerStep,
    RenderCommands&               renderCommands) const
{
	renderCommands.clear(); // Keeps the capacity, no reallocation

	for (const model::Track& track : tracks.getAll())
	{
		for (const model::Channel& c : track.getChannels().getAll())
		{
			if (c.isInternal())
				continue;
			const auto params = assets.channelsParameters.find(c.id);
			assert(params != nullptr);
			renderCommands.push_back({c.id, advanceChannel(c, *params, events, block, quantizerStep)});
		}
	}
}

/* -------------------------------------------------------------------------- */

RenderInfo Renderer::advanceChannel(const model::Channel& ch,
    const model::Channel::Parameters&                     params,
    const Sequencer::EventBuffer&                         events,
    FrameRange                                            block,
    Frame                                                 quantizerStep) const
{
	// TODO
	// if (ch.shared->quantizer)
	//	ch.shared->quantizer->advance(block, quantizerStep);

	RenderInfo info = {RenderInfo::Mode::NONE};

	for (const Sequencer::Event& e : events)
	{
		if (ch.type == ChannelType::MIDI)
			advanceMidiChannel(ch, params, e, m_kernelMidi);
		else if (ch.type == ChannelType::SAMPLE)
			info = advanceSampleChannel(ch, params, e);
	}

	return info;
}

/* -------------------------------------------------------------------------- */

void Renderer::renderTracks(
    const model::Tracks&             tracks,
    const model::Assets&             assets,
    const model::Channel::Rendering& masterOutRendering,
    mcl::AudioBuffer&                hardwareOut,
    const mcl::AudioBuffer&          in,
    Scene                            scene,
    bool                             hasSolos,
    bool                             seqIsRunning) const
{
	masterOutRendering.clearAudioBuffer(0);

	for (const model::Track& track : tracks.getAll())
	{
		if (track.isInternal())
			continue;

		const model::Channel& group           = track.getGroupChannel();
		const auto            groupParameters = assets.channelsParameters.find(group.id);
		const auto            groupRendering  = assets.channelsRendering.find(group.id);

		assert(groupParameters != nullptr);
		assert(groupRendering != nullptr);

		groupRendering->clearAudioBuffer(0);

		for (const model::Channel& c : track.getChannels().getAll())
		{
			const auto channelParameters = assets.channelsParameters.find(c.id);
			const auto channelRendering  = assets.channelsRendering.find(c.id);
			assert(channelParameters != nullptr);
			assert(channelRendering != nullptr);

			const RenderInfo renderInfo = findRenderInfo_(m_renderCommands, c.id);

			renderNormalChannel(c, *channelParameters, *channelRendering, assets, renderInfo, in, scene, seqIsRunning);
			if (!c.isAudible(hasSolos))
				continue;
			if (c.sendToMaster)
				mergeChannel(c, *channelParameters, *channelRendering, *groupRendering);
			for (const int offset : c.extraOutputs)
				mergeChannel(c, *channelRendering, hardwareOut, offset);
		}

		// TODO
		// renderAudioPlugins(group, m_pluginHost);

		if (!group.isAudible(hasSolos))
			continue;
		if (group.sendToMaster)
			mergeChannel(group, *groupParameters, *groupRendering, masterOutRendering);
		for (const int offset : group.extraOutputs)
			mergeChannel(group, *groupRendering, hardwareOut, offset);
	}
}

/* -------------------------------------------------------------------------- */

void Renderer::renderNormalChannel(
    const model::Channel&             ch,
    const model::Channel::Parameters& params,
    const model::Channel::Rendering&  rendering,
    const model::Assets&              assets,
    RenderInfo                        renderInfo,
    const mcl::AudioBuffer&           in,
    Scene                             scene,
    bool                              seqIsRunning) const
{
	rendering.clearAudioBuffer(0);

	if (ch.type == ChannelType::SAMPLE)
	{
		const auto wave = assets.waves.find(ch.sampleChannel->getSample(scene).waveId);
		renderSampleChannel(ch, params, rendering, wave.get(), renderInfo, in, scene, seqIsRunning);
	}
	// TODO
	// else if (ch.type == ChannelType::MIDI)
	//	renderMidiChannel(ch);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderMasterIn(const Channel& ch, mcl::AudioBuffer& in) const
{
	m_pluginHost.processStack(in, ch.plugins, nullptr);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderMasterOut(
    const model::Channel&            ch,
    const model::Channel::Rendering& src,
    mcl::AudioBuffer&                out,
    int                              channelOffset) const
{
	// TODO
	// m_pluginHost.processStack(ch.shared->audioBuffer, ch.plugins, nullptr);
	mergeChannel(ch, src, out, channelOffset);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderPreview(const Channel& ch, mcl::AudioBuffer& out) const
{
	// TODO
	// ch.shared->audioBuffer.clear();

	// if (ch.isPlaying())
	//	rendering::renderSampleChannel(ch, Scene{0}, /*seqIsRunning=*/false); // Sequencer status and scene are irrelevant here

	// out.sumAll(ch.shared->audioBuffer, ch.volume);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderSampleChannel(
    const model::Channel&             ch,
    const model::Channel::Parameters& params,
    const model::Channel::Rendering&  render,
    const model::Wave*                wave,
    RenderInfo                        renderInfo,
    const mcl::AudioBuffer&           in,
    Scene scene, bool seqIsRunning) const
{
	assert(ch.type == ChannelType::SAMPLE);

	if (params.isPlaying())
		rendering::renderSampleChannel(ch, params, render, wave, renderInfo, scene, seqIsRunning);

	// TODO
	// if (ch.canReceiveAudio())
	//	renderSampleChannelInput(ch, in); // record "clean" audio first	(i.e. not plugin-processed)

	// TODO
	// renderAudioPlugins(ch, m_pluginHost);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderMidiChannel(const Channel& ch) const
{
	assert(ch.type == ChannelType::MIDI);

	renderAudioAndMidiPlugins(ch, m_pluginHost);
}

/* -------------------------------------------------------------------------- */

void Renderer::mergeChannel(
    const model::Channel&             ch,
    const model::Channel::Parameters& params,
    const model::Channel::Rendering&  src,
    const model::Channel::Rendering&  dest) const
{
	dest.sumToAudioBufferFrom(src.getAudioBuffer(), ch.pan, ch.volume * params.getVolumeInternal());
}

/* -------------------------------------------------------------------------- */

void Renderer::mergeChannel(
    const model::Channel&            ch,
    const model::Channel::Rendering& src,
    mcl::AudioBuffer&                out,
    int                              destChannelOffset) const
{
	const mcl::AudioBuffer& buf = src.getAudioBuffer();

	assert(buf.countChannels() == static_cast<int>(ch.pan.get().size()));

	for (int i = 0; i < buf.countChannels(); i++)
		if (destChannelOffset < out.countChannels())
			out.sum(buf, i, i + destChannelOffset, ch.volume * ch.pan.get()[i]);
}
} // namespace giada::m::rendering
