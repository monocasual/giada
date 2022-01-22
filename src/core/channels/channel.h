/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_CHANNEL_H
#define G_CHANNEL_H

#include <optional>
#ifdef WITH_VST
#include "deps/juce-config.h"
#endif
#include "core/channels/audioReceiver.h"
#include "core/channels/midiActionRecorder.h"
#include "core/channels/midiController.h"
#include "core/channels/midiLearner.h"
#include "core/channels/midiLighter.h"
#include "core/channels/midiSender.h"
#include "core/channels/sampleActionRecorder.h"
#include "core/channels/sampleAdvancer.h"
#include "core/channels/samplePlayer.h"
#include "core/channels/sampleReactor.h"
#include "core/const.h"
#include "core/eventDispatcher.h"
#include "core/midiEvent.h"
#include "core/mixer.h"
#include "core/patch.h"
#include "core/queue.h"
#include "core/resampler.h"
#include "core/sequencer.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#ifdef WITH_VST
#include "core/channels/midiReceiver.h"
#endif

namespace giada::m
{
class Plugin;

struct ChannelShared final
{
	ChannelShared(Frame bufferSize);

	mcl::AudioBuffer audioBuffer;
#ifdef WITH_VST
	juce::MidiBuffer     midiBuffer;
	Queue<MidiEvent, 32> midiQueue;
#endif

	WeakAtomic<Frame>         tracker     = 0;
	WeakAtomic<ChannelStatus> playStatus  = ChannelStatus::OFF;
	WeakAtomic<ChannelStatus> recStatus   = ChannelStatus::OFF;
	WeakAtomic<bool>          readActions = false;

	std::optional<Quantizer> quantizer;

	/* Optional render queue for sample-based channels. Used by SampleReactor
	and SampleAdvancer to instruct SamplePlayer how to render audio. */

	std::optional<Queue<SamplePlayer::Render, 2>> renderQueue = {};

	/* Optional resampler for sample-based channels. Unfortunately a Resampler
	object (based on libsamplerate) doesn't like to get copied while rendering
	audio, so can't live inside WaveReader object (which is copied on model 
	changes by the Swapper mechanism). Let's put it in the shared state here. */

	std::optional<Resampler> resampler = {};
};

/* -------------------------------------------------------------------------- */

class Channel final
{
public:
	Channel(ChannelType t, ID id, ID columnId, ChannelShared&);
	Channel(const Patch::Channel& p, ChannelShared&, float samplerateRatio, Wave* w);
	Channel(const Channel& o);
	Channel(Channel&& o) = default;

	Channel& operator=(const Channel&);
	Channel& operator=(Channel&&) = default;
	bool     operator==(const Channel&);

	/* advance
	Advances internal state by processing static events (e.g. pre-recorded 
	actions or sequencer events) in the current block. */

	void advance(const Sequencer::EventBuffer&, Range<Frame>, Frame quantizerStep) const;

	/* render
	Renders audio data to I/O buffers. */

	void render(mcl::AudioBuffer* out, mcl::AudioBuffer* in, bool audible) const;

	/* react
	Reacts to live events coming from the EventDispatcher (human events) and
	updates itself accordingly. */

	void react(const EventDispatcher::EventBuffer& e);

	bool isPlaying() const;
	bool isReadingActions() const;
	bool isInternal() const;
	bool isMuted() const;
	bool isSoloed() const;
	bool canInputRec() const;
	bool canActionRec() const;
	bool hasWave() const;

	void setMute(bool);
	void setSolo(bool);

	ChannelShared* shared;
	ID             id;
	ChannelType    type;
	ID             columnId;
	float          volume;
	float          volume_i; // Internal volume used for velocity-drives-volume mode on Sample Channels
	float          pan;
	bool           armed;
	int            key;
	bool           hasActions;
	std::string    name;
	Pixel          height;
#ifdef WITH_VST
	std::vector<Plugin*> plugins;
#endif

	MidiLearner             midiLearner;
	MidiLighter<KernelMidi> midiLighter;

	std::optional<SamplePlayer>   samplePlayer;
	std::optional<SampleAdvancer> sampleAdvancer;
	std::optional<SampleReactor>  sampleReactor;
	std::optional<AudioReceiver>  audioReceiver;
	std::optional<MidiController> midiController;
#ifdef WITH_VST
	std::optional<MidiReceiver> midiReceiver;
#endif
	std::optional<MidiSender>           midiSender;
	std::optional<SampleActionRecorder> sampleActionRecorder;
	std::optional<MidiActionRecorder>   midiActionRecorder;

private:
	void renderMasterOut(mcl::AudioBuffer&) const;
	void renderMasterIn(mcl::AudioBuffer&) const;
	void renderChannel(mcl::AudioBuffer& out, mcl::AudioBuffer& in, bool audible) const;

	void initCallbacks();
	void react(const EventDispatcher::Event&);

	bool m_mute;
	bool m_solo;
};
} // namespace giada::m

#endif
