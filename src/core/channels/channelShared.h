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

#ifndef G_CHANNELSHARED_H
#define G_CHANNELSHARED_H

#include <optional>
#ifdef WITH_VST
#include "deps/juce-config.h"
#endif
#include "core/channels/samplePlayer.h"
#include "core/const.h"
#include "core/midiEvent.h"
#include "core/queue.h"
#include "core/resampler.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"

namespace giada::m
{
struct ChannelShared final
{
	using MidiQueue   = Queue<MidiEvent, 32>;
	using RenderQueue = Queue<SamplePlayer::Render, 2>;

	ChannelShared(Frame bufferSize);

	bool isReadingActions() const;

	mcl::AudioBuffer audioBuffer;
#ifdef WITH_VST
	juce::MidiBuffer midiBuffer;
	MidiQueue        midiQueue;
#endif

	WeakAtomic<Frame>         tracker     = 0;
	WeakAtomic<ChannelStatus> playStatus  = ChannelStatus::OFF;
	WeakAtomic<ChannelStatus> recStatus   = ChannelStatus::OFF;
	WeakAtomic<bool>          readActions = false;

	std::optional<Quantizer> quantizer;

	/* Optional render queue for sample-based channels. Used by SampleReactor
	and SampleAdvancer to instruct SamplePlayer how to render audio. */

	std::optional<RenderQueue> renderQueue = {};

	/* Optional resampler for sample-based channels. Unfortunately a Resampler
	object (based on libsamplerate) doesn't like to get copied while rendering
	audio, so can't live inside WaveReader object (which is copied on model 
	changes by the Swapper mechanism). Let's put it in the shared state here. */

	std::optional<Resampler> resampler = {};
};
} // namespace giada::m

#endif
