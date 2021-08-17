/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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
#include "core/channels/samplePlayer.h"
#include "core/const.h"
#include "core/eventDispatcher.h"
#include "core/midiEvent.h"
#include "core/mixer.h"
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
}
namespace giada::m::channel
{
struct State
{
	WeakAtomic<Frame>         tracker     = 0;
	WeakAtomic<ChannelStatus> playStatus  = ChannelStatus::OFF;
	WeakAtomic<ChannelStatus> recStatus   = ChannelStatus::OFF;
	WeakAtomic<bool>          readActions = false;
	bool                      rewinding   = false;
	Frame                     offset      = 0;

	/* Optional resampler for sample-based channels. Unfortunately a Resampler
	object (based on libsamplerate) doesn't like to get copied while rendering
	audio, so can't live inside WaveReader object (which is copied on model 
	changes by the Swapper mechanism). Let's put it in the shared state here. */

	std::optional<Resampler> resampler = {};
};

struct Buffer
{
	Buffer(Frame bufferSize);

	mcl::AudioBuffer audio;
#ifdef WITH_VST
	juce::MidiBuffer     midi;
	Queue<MidiEvent, 32> midiQueue;
#endif
};

struct Data
{
	Data(ChannelType t, ID id, ID columnId, State& state, Buffer& buffer);
	Data(const patch::Channel& p, State& state, Buffer& buffer, float samplerateRatio);
	Data(const Data& o) = default;
	Data(Data&& o)      = default;
	Data& operator=(const Data&) = default;
	Data& operator=(Data&&) = default;

	bool operator==(const Data&);

	bool isPlaying() const;
	bool isReadingActions() const;
	bool isInternal() const;
	bool isMuted() const;
	bool canInputRec() const;
	bool canActionRec() const;
	bool hasWave() const;

	State*      state;
	Buffer*     buffer;
	ID          id;
	ChannelType type;
	ID          columnId;
	float       volume;
	float       volume_i; // Internal volume used for velocity-drives-volume mode on Sample Channels
	float       pan;
	bool        mute;
	bool        solo;
	bool        armed;
	int         key;
	bool        hasActions;
	std::string name;
	Pixel       height;
#ifdef WITH_VST
	std::vector<Plugin*> plugins;
#endif

	midiLearner::Data midiLearner;
	midiLighter::Data midiLighter;

	std::optional<samplePlayer::Data>   samplePlayer;
	std::optional<sampleReactor::Data>  sampleReactor;
	std::optional<audioReceiver::Data>  audioReceiver;
	std::optional<midiController::Data> midiController;
#ifdef WITH_VST
	std::optional<midiReceiver::Data> midiReceiver;
#endif
	std::optional<midiSender::Data>           midiSender;
	std::optional<sampleActionRecorder::Data> sampleActionRecorder;
	std::optional<midiActionRecorder::Data>   midiActionRecorder;
};

/* advance
Advances internal state by processing static events (e.g. pre-recorded 
actions or sequencer events) in the current block. */

void advance(const Data& d, const sequencer::EventBuffer& e);

/* react
Reacts to live events coming from the EventDispatcher (human events) and
updates itself accordingly. */

void react(Data& d, const eventDispatcher::EventBuffer& e, bool audible);

/* render
Renders audio data to I/O buffers. */

void render(const Data& d, mcl::AudioBuffer* out, mcl::AudioBuffer* in, bool audible);
} // namespace giada::m::channel

#endif
