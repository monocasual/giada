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

#ifndef G_CHANNELSHARED_H
#define G_CHANNELSHARED_H

#include "core/const.h"
#include "core/midiEvent.h"
#include "core/quantizer.h"
#include "core/queue.h"
#include "core/rendering/sampleRendering.h"
#include "core/resampler.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include <juce_audio_basics/juce_audio_basics.h>
#include <optional>

namespace giada::m
{
struct ChannelShared final
{
	using MidiQueue   = Queue<MidiEvent, 32>; // TODO - must be multi-producer (multiple midi threads)
	using RenderQueue = Queue<rendering::RenderInfo, 2>;

	ChannelShared(Frame bufferSize);

	bool isReadingActions() const;

	/* setBufferSize 
	Sets a new size for the internal audio buffer. */

	void setBufferSize(int);

	mcl::AudioBuffer audioBuffer;
	juce::MidiBuffer midiBuffer;
	MidiQueue        midiQueue;

	WeakAtomic<Frame>         tracker        = 0;
	WeakAtomic<ChannelStatus> playStatus     = ChannelStatus::OFF;
	WeakAtomic<ChannelStatus> recStatus      = ChannelStatus::OFF;
	WeakAtomic<bool>          readActions    = false;
	WeakAtomic<float>         volumeInternal = G_DEFAULT_VOL; // Used for velocity-drives-volume mode on Sample Channels

	std::optional<Quantizer> quantizer;

	/* Optional render queue for sample-based channels. Used by callers on thread
	different than the real-time one to instruct the real-time one how to render 
	audio. */

	std::optional<RenderQueue> renderQueue = {};

	/* Optional resampler for sample-based channels. Unfortunately a Resampler
	object (based on libsamplerate) doesn't like to get copied while rendering
	audio, so can't live inside a Channel object (which is copied on model 
	changes by the Swapper mechanism). Let's put it in the shared state here. */

	std::optional<Resampler> resampler = {};
};
} // namespace giada::m

#endif
