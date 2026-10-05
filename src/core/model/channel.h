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

#ifndef G_MODEL_CHANNEL_H
#define G_MODEL_CHANNEL_H

#include "src/core/midiEvent.h"
#include "src/core/model/midiChannel.h"
#include "src/core/model/midiInput.h"
#include "src/core/model/midiLightning.h"
#include "src/core/model/sampleChannel.h"
#include "src/core/pan.h"
#include "src/core/patch.h"
#include "src/core/quantizer.h"
#include "src/core/rendering/sampleRendering.h"
#include "src/core/resampler.h"
#include "src/core/stretcher.h"
#include "src/core/weakAtomic.h"
#include "src/deps/concurrentqueue/concurrentqueue.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include <juce_audio_basics/juce_audio_basics.h>
#include <optional>

namespace giada::m::model
{
class Channel final
{
public:
	class Parameters final
	{
	public:
		bool isPlaying() const;
		bool isReadingActions() const;

		/* isActive
		True if status is PLAY, WAIT or ENDING. */

		bool isActive() const;

		// TODO...

	private:
		mutable WeakAtomic<Frame>         m_tracker        = 0;
		mutable WeakAtomic<ChannelStatus> m_playStatus     = ChannelStatus::OFF;
		mutable WeakAtomic<ChannelStatus> m_recStatus      = ChannelStatus::OFF;
		mutable WeakAtomic<bool>          m_readActions    = false;
		mutable WeakAtomic<float>         m_volumeInternal = G_DEFAULT_VOL; // Used for velocity-drives-volume mode on Sample Channels
		                                                                    // TODO - midi input
	};

	class Rendering final
	{
	public:
		Rendering(ChannelType, int sampleRate, int bufferSize, Resampler::Quality);

		void pushMidiEvent(MidiEvent e) const { m_midiQueue.enqueue(std::move(e)); }
		// TODO...

	private:
		using MidiQueue   = moodycamel::ConcurrentQueue<MidiEvent>;
		using RenderQueue = moodycamel::ConcurrentQueue<rendering::RenderInfo>;

		mutable mcl::AudioBuffer m_audioBuffer;
		mutable juce::MidiBuffer m_midiBuffer;
		mutable MidiQueue        m_midiQueue{/*size=*/32, 0, /*num_threads=*/8}; // TODO - maximum 8 MIDI threads for now

		mutable std::optional<Quantizer> m_quantizer;

		/* m_renderQueue
		Optional render queue for sample-based channels. Used by callers on thread
		different than the real-time one to instruct the real-time one how to render
		audio. */

		mutable std::optional<RenderQueue> m_renderQueue;

		/* Optional resampler for sample-based channels. Unfortunately a Resampler
		object (based on libsamplerate) doesn't like to get copied while rendering
		audio, so can't live inside a Channel object (which is copied on model
		changes by the RealtimeModel mechanism). Let's put it in the shared state
		here. Same applies for the Stretcher, too. */

		mutable std::optional<Resampler> m_resampler;
		mutable std::optional<Stretcher> m_stretcher;
	};

	Channel(ChannelType t, ID id);
	Channel(const Patch::Channel&, float samplerateRatio, const SceneArray<Sample>&, std::vector<ID>);

	bool operator==(const Channel&) const;

	bool isInternal() const;
	bool isMuted() const;
	bool isSoloed() const;
	bool canInputRec(Scene) const;
	bool canActionRec(Scene) const;
	bool hasWave(Scene) const;

	std::string                    getName(Scene) const;
	const SceneArray<std::string>& getNames() const;

	/* canReceiveAudio
	Tells if the sample channel can receive audio as input monitor. */

	bool canReceiveAudio() const;

	/* canSendMidi
	Tells if the MIDI channel can output MIDI messages to the outside world. */

	bool canSendMidi() const;

	/* isAudible
	True if this channel is currently audible: not muted or not included in a
	solo session. */

	bool isAudible(bool mixerHasSolos) const;

#if G_DEBUG_MODE
	std::string debug() const;
#endif

	void setMute(bool);
	void setSolo(bool);
	void setName(const std::string&, Scene);

	/* loadWave
	Loads Wave and sets it up (name, markers, ...). Also updates Channel's shared
	state accordingly. Resets begin/end points shift if not specified (-1). */

	void loadSample(const Sample&, Scene);

	/* setWave
	Just sets the pointer to a Wave object. Used during de-serialization. The
	ratio is used to adjust begin/end points in case of patch vs. conf sample
	rate mismatch. If nullptr, set the wave to invalid. */

	void setWave(Wave* w, Scene, float samplerateRatio);

	/* kickIn
	Starts the player right away at frame 'f'. Used when launching a loop after
	being live recorded. */

	void kickIn(Frame f);

	ID              id;
	ChannelType     type;
	float           volume;
	Pan             pan;
	bool            armed;
	int             key;    // TODO - move this to v::Model
	int             height; // TODO - move this to v::Model
	std::vector<ID> plugins;

	/* sendToMaster
	If false, the audio buffer of this channel won't be rendered to master
	channel. Useful if you want extra outputs only (see below). */

	bool sendToMaster;

	/* extraOutputs
	Defines the channel _offsets_ that will be used to render the audio buffer of
	this channel when extra outputs are used. Each element of the vector is
	intended as an extra ouput. */

	std::vector<int> extraOutputs;

	MidiInput     midiInput;
	MidiLightning midiLightning;

	std::optional<SampleChannel> sampleChannel;
	std::optional<MidiChannel>   midiChannel;

private:
	bool                    m_mute;
	bool                    m_solo;
	SceneArray<std::string> m_names;
};
} // namespace giada::m::model

#endif
