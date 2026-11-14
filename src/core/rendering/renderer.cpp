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
#include "src/core/model/modelView.h"
#include "src/core/model_DEPR_/model.h"
#include "src/core/rendering/midiAdvance.h"
#include "src/core/rendering/midiOutput.h"
#include "src/core/rendering/midiReactions.h"
#include "src/core/rendering/pluginRendering.h"
#include "src/core/rendering/sampleAdvance.h"
#include "src/core/rendering/sampleRendering.h"
#include "src/utils/time.h"
#include <chrono>
#include <cmath>
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

/* -------------------------------------------------------------------------- */

juce::AudioPlayHead::PositionInfo makePluginPositionInfo_(
    const model::Sequencer& sequencer,
    Frame                   blockStart,
    int                     sampleRate,
    bool                    isPlaying)
{
	const TimeSignature signature   = sequencer.getTimeSignature();
	const double        bpm         = sequencer.getBpm();
	const double        ppqPosition = u::time::frameToQuarterNotes(blockStart, sampleRate, bpm);
	const int           beatsPerBar = signature.beats / signature.bars;

	juce::AudioPlayHead::PositionInfo position;
	position.setTimeInSamples(blockStart);
	position.setTimeInSeconds(static_cast<double>(blockStart) / sampleRate);
	position.setBpm(bpm);
	position.setPpqPosition(ppqPosition);
	position.setPpqPositionOfLastBarStart(std::floor(ppqPosition / beatsPerBar) * beatsPerBar);
	position.setTimeSignature(juce::AudioPlayHead::TimeSignature{beatsPerBar, 4});
	position.setIsPlaying(isPlaying);

	return position;
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

void Renderer::render(mcl::AudioBuffer& out, const mcl::AudioBuffer& in, model::RealtimeModel& model) const
{
	const Now startTime = std::chrono::high_resolution_clock::now();

	/* Clean up output buffer before any rendering. Do this even if mixer is
	disabled to avoid audio leftovers during a temporary suspension (e.g. when
	loading a new patch). */

	out.clear();

	const model::RealtimeLock lock   = model.read();
	const model::ModelView    view   = model::makeModelView(lock);
	const model::Document&    doc    = view.getDocument();
	const model::Assets&      assets = view.getAssets();
	const model::Parameters&  params = view.getParameters();

	/* Mixer disabled or Kernel Audio not ready: nothing to do here. */

	if (!params.mixer.isActive())
		return;

	const int bufferSize = out.countFrames();
	const int sampleRate = doc.kernelAudio.samplerate;

#ifdef WITH_AUDIO_JACK
	if (kernelAudio.api == RtAudio::Api::UNIX_JACK)
		m_jackSynchronizer.recvJackSync(m_jackTransport.getState());
#endif

	const Frame currentFrame   = params.sequencer.getCurrentFrame();
	const auto  pluginPosition = makePluginPositionInfo_(doc.sequencer, currentFrame,
	     sampleRate, doc.sequencer.isRunning());

	/* If the sequencer is running, advance it first (i.e. parse it for events).
	Also advance channels (i.e. let them react to sequencer events). */

	if (doc.sequencer.isRunning())
	{
		const int        quantizerStep = m_sequencer.getQuantizerStep();            // TODO pass this to m_sequencer.advance - or better, Advancer class
		const FrameRange renderRange   = {currentFrame, currentFrame + bufferSize}; // TODO pass this to m_sequencer.advance - or better, Advancer class

		const Sequencer::EventBuffer& events = m_sequencer.advance(doc.sequencer, params.sequencer, doc.actions, bufferSize);
		advanceTracks(events, view, renderRange, quantizerStep, m_renderCommands);

		if (doc.sequencer.metronome)
			m_sequencer.render(out);
	}
	else
		m_renderCommands.clear(); // Don't reuse stale commands from the previous block

	/* Then render Mixer, channels and finalize output. */

	const Scene              scene         = params.sequencer.getCurrentScene();
	const model::Channel&    masterOutCh   = doc.tracks.getChannel(MASTER_OUT_CHANNEL_ID);
	const model::ChannelView masterOutView = view.makeChannelView(masterOutCh);
	const model::Channel&    previewCh     = doc.tracks.getChannel(PREVIEW_CHANNEL_ID);

	/* Take care of the input side of audio: prepare it, check for recording
	threshold, record it if required. */

	m_mixer.handleAudioInput(in, view);

	// Render tracks

	renderTracks(view, pluginPosition, masterOutView, out);

	renderMasterOut(masterOutView, assets, pluginPosition, out, doc.kernelAudio.deviceOut.channelsStart);

	if (doc.mixer.renderPreview)
		renderPreview(view.makeSampleChannelView(previewCh, scene), out);

	m_mixer.updateOutputPeak(params.mixer, masterOutView.getRendering().getAudioBuffer());

	/* Post processing. */

	m_mixer.finalizeOutput(assets.mixerRendering->getInBuffer(), out, doc.mixer.inToOut,
	    doc.kernelAudio.limitOutput, masterOutCh.volume);

	/* CPU load computation. */

	params.kernelAudio.setCpuLoad(computeCpuLoad_(startTime, doc.kernelAudio.samplerate, bufferSize));
}

/* -------------------------------------------------------------------------- */

void Renderer::advanceTracks(
    const Sequencer::EventBuffer& events,
    const model::ModelView&       view,
    FrameRange                    block,
    Frame                         quantizerStep,
    RenderCommands&               renderCommands) const
{
	renderCommands.clear(); // Keeps the capacity, no reallocation

	for (const model::Track& track : view.getDocument().tracks.getAll())
	{
		for (const model::Channel& c : track.getChannels().getAll())
		{
			if (c.isInternal())
				continue;
			const model::ChannelView channelView = view.makeChannelView(c);
			renderCommands.push_back({c.id, advanceChannel(channelView, events, block, quantizerStep)});
		}
	}
}

/* -------------------------------------------------------------------------- */

RenderInfo Renderer::advanceChannel(const model::ChannelView& channelView,
    const Sequencer::EventBuffer&                             events,
    FrameRange                                                block,
    Frame                                                     quantizerStep) const
{
	const model::Channel& ch = channelView.getChannel();

	// TODO
	// if (ch.shared->quantizer)
	//	ch.shared->quantizer->advance(block, quantizerStep);

	RenderInfo info = {RenderInfo::Mode::NONE};

	for (const Sequencer::Event& e : events)
	{
		if (ch.type == ChannelType::MIDI)
			advanceMidiChannel(channelView, e, m_kernelMidi);
		else if (ch.type == ChannelType::SAMPLE)
			info = advanceSampleChannel(channelView, e);
	}

	return info;
}

/* -------------------------------------------------------------------------- */

void Renderer::renderTracks(
    const model::ModelView&                  view,
    const juce::AudioPlayHead::PositionInfo& pluginPosition,
    const model::ChannelView&                masterOut,
    mcl::AudioBuffer&                        hardwareOut) const
{
	const model::Document&           doc                = view.getDocument();
	const model::Assets&             assets             = view.getAssets();
	const model::Parameters&         params             = view.getParameters();
	const model::Channel::Rendering& masterOutRendering = masterOut.getRendering();

	const mcl::AudioBuffer& in           = assets.mixerRendering->getInBuffer();
	const bool              seqIsRunning = doc.sequencer.isRunning();
	const bool              hasSolos     = doc.mixer.hasSolos;
	const Scene             scene        = params.sequencer.getCurrentScene();

	masterOutRendering.clearAudioBuffer(0);

	for (const model::Track& track : doc.tracks.getAll())
	{
		if (track.isInternal())
			continue;

		const model::Channel&    group     = track.getGroupChannel();
		const model::ChannelView groupView = view.makeChannelView(group);

		groupView.getRendering().clearAudioBuffer(0);

		for (const model::Channel& c : track.getChannels().getAll())
		{
			const model::ChannelView channelView = view.makeChannelView(c);
			const RenderInfo         renderInfo  = findRenderInfo_(m_renderCommands, c.id);

			renderNormalChannel(channelView, view, renderInfo, pluginPosition, in, scene, seqIsRunning);

			if (!c.isAudible(hasSolos))
				continue;
			if (c.sendToMaster)
				mergeChannel(channelView, groupView);
			for (const int offset : c.extraOutputs)
				mergeChannel(channelView, hardwareOut, offset);
		}

		renderAudioPlugins(groupView, assets, pluginPosition, m_pluginHost);

		if (!group.isAudible(hasSolos))
			continue;
		if (group.sendToMaster)
			mergeChannel(groupView, masterOut);
		for (const int offset : group.extraOutputs)
			mergeChannel(groupView, hardwareOut, offset);
	}
}

/* -------------------------------------------------------------------------- */

void Renderer::renderNormalChannel(
    const model::ChannelView&                channelView,
    const model::ModelView&                  view,
    RenderInfo                               renderInfo,
    const juce::AudioPlayHead::PositionInfo& pluginPosition,
    const mcl::AudioBuffer&                  in,
    Scene                                    scene,
    bool                                     seqIsRunning) const
{
	const model::Assets& assets = view.getAssets();
	channelView.getRendering().clearAudioBuffer(0);

	if (channelView.getChannel().type == ChannelType::SAMPLE)
	{
		renderSampleChannel(view.makeSampleChannelView(channelView, scene), renderInfo, in, seqIsRunning);
		renderAudioPlugins(channelView, assets, pluginPosition, m_pluginHost);
	}
	else if (channelView.getChannel().type == ChannelType::MIDI)
	{
		renderAudioAndMidiPlugins(channelView, assets, pluginPosition, m_pluginHost);
	}
}

/* -------------------------------------------------------------------------- */

void Renderer::renderMasterOut(
    const model::ChannelView&                channelView,
    const model::Assets&                     assets,
    const juce::AudioPlayHead::PositionInfo& pluginPosition,
    mcl::AudioBuffer&                        out,
    int                                      channelOffset) const
{
	renderAudioPlugins(channelView, assets, pluginPosition, m_pluginHost);
	mergeChannel(channelView, out, channelOffset);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderPreview(const model::SampleChannelView& sampleView, mcl::AudioBuffer& out) const
{
	const model::Channel::Rendering& rendering = sampleView.getRendering();

	rendering.clearAudioBuffer(0);

	if (sampleView.getParameters().isPlaying())
		rendering::renderSampleChannel(sampleView, {}, /*seqIsRunning=*/false);

	out.sumAll(rendering.getAudioBuffer(), sampleView.getChannel().volume);
}

/* -------------------------------------------------------------------------- */

void Renderer::renderSampleChannel(
    const model::SampleChannelView& sampleView,
    RenderInfo                      renderInfo,
    const mcl::AudioBuffer&         in,
    bool                            seqIsRunning) const
{
	assert(sampleView.getChannel().type == ChannelType::SAMPLE);

	if (sampleView.getParameters().isPlaying())
		rendering::renderSampleChannel(sampleView, renderInfo, seqIsRunning);

	// TODO
	// if (ch.canReceiveAudio())
	//	renderSampleChannelInput(ch, in); // record "clean" audio first	(i.e. not plugin-processed)
}

/* -------------------------------------------------------------------------- */

void Renderer::mergeChannel(
    const model::ChannelView& src,
    const model::ChannelView& dest) const
{
	const model::Channel& channel = src.getChannel();

	dest.getRendering().sumToAudioBufferFrom(
	    src.getRendering().getAudioBuffer(),
	    channel.pan,
	    channel.volume * src.getParameters().getVolumeInternal());
}

/* -------------------------------------------------------------------------- */

void Renderer::mergeChannel(
    const model::ChannelView& channelView,
    mcl::AudioBuffer&         out,
    int                       destChannelOffset) const
{
	const model::Channel&            channel = channelView.getChannel();
	const model::Channel::Rendering& src     = channelView.getRendering();
	const mcl::AudioBuffer&          buf     = src.getAudioBuffer();

	assert(buf.countChannels() == static_cast<int>(channel.pan.get().size()));

	for (int i = 0; i < buf.countChannels(); i++)
		if (destChannelOffset < out.countChannels())
			out.sum(buf, i, i + destChannelOffset, channel.volume * channel.pan.get()[i]);
}
} // namespace giada::m::rendering
