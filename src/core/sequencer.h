/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_SEQUENCER_H
#define G_SEQUENCER_H

#include "src/core/actions/action.h"
#include "src/core/const.h"
#include "src/core/eventDispatcher.h"
#include "src/core/metronome.h"
#include "src/core/quantizer.h"
#include "src/core/ringBuffer.h"
#include <vector>

namespace mcl
{
class AudioBuffer;
}

namespace giada::m::model
{
class Model;
class Sequencer;
class Actions;
struct Document;
} // namespace giada::m::model

namespace giada::m
{
class JackTransport;
class ActionRecorder;
class MidiSynchronizer;
class Sequencer final
{
public:
	enum class EventType
	{
		NONE,
		FIRST_BEAT,
		BAR,
		REWIND,
		ACTIONS
	};

	struct Event
	{
		EventType                  type    = EventType::NONE;
		Frame                      global  = 0;
		Frame                      delta   = 0;
		const std::vector<Action>* actions = nullptr;
	};

	using EventBuffer = RingBuffer<Event, G_MAX_SEQUENCER_EVENTS>;

	Sequencer(model::Model&, MidiSynchronizer&, JackTransport&);

	/* canQuantize
	Tells whether the quantizer value is > 0 and the sequencer is running. */

	bool canQuantize() const;

	/* isRunning
	When sequencer is actually moving forward, i.e. SeqStatus == RUNNING. */

	bool isRunning() const;

	/* isActive
	Sequencer is enabled, but might be in wait mode, i.e. SeqStatus == RUNNING or
	SeqStatus == WAITING. */

	bool isActive() const;

	bool isOnBeat() const;
	bool isOnBar() const;
	bool isOnFirstBeat() const;
	bool isMetronomeOn() const;

	float     getBpm() const;
	int       getBeats() const;
	int       getBars() const;
	int       getCurrentBeat() const;
	Frame     getCurrentFrame() const;
	Frame     getCurrentFrameQuantized() const;
	float     getCurrentSecond(int sampleRate) const;
	Frame     getFramesInBar() const;
	Frame     getFramesInBeat() const;
	Frame     getFramesInLoop() const;
	Frame     getFramesInSeq() const;
	int       getQuantizerValue() const;
	int       getQuantizerStep() const;
	SeqStatus getStatus() const;
	int       getMaxFramesInLoop(int sampleRate) const;

	/* calcBpmFromRec
	Given the amount of recorded frames, returns the speed of the current
	performance. Used while input recording in FREE mode. */

	float calcBpmFromRec(Frame recordedFrames, int sampleRate) const;

	/* quantize
	Quantizes the frame 'f'.  */

	Frame quantize(Frame f) const;

	/* reset
	Brings everything back to the initial state. */

	void reset(int sampleRate);

	/* setSampleRate
	Sets a new sample rate value for the internal beat computation. Must be
	called only when mixer is disabled.*/

	void setSampleRate(int);

	/* advance
	Parses sequencer events that might occur in a block and advances the internal
	quantizer. Returns a reference to the internal EventBuffer filled with events
	(if any). Call this on each new audio block. */

	const EventBuffer& advance(const model::Sequencer&, Frame bufferSize, int sampleRate,
	    const model::Actions&) const;

	/* render
	Renders audio coming out from the sequencer: that is, the metronome! */

	void render(mcl::AudioBuffer& outBuf, const model::Document&) const;

	void rewindForced();
	void rewind();
	void start();
	void stop();
	void toggleMetronome();
	void setMetronome(bool v);
	void setBpm(float b, int sampleRate);
	void setBeats(int beats, int bars, int sampleRate);
	void setQuantize(int q, int sampleRate);
	void setStatus(SeqStatus);
	void goToBeat(int beat, int sampleRate);

#ifdef WITH_AUDIO_JACK
	void jack_start();
	void jack_stop();
	void jack_rewind();
	void jack_setBpm(float b, int sampleRate);
#endif

	/* recomputeFrames
	Updates bpm, frames, beats and so on. */

	void recomputeFrames(int sampleRate);

	std::function<void(SeqStatus)>         onAboutStart;
	std::function<void()>                  onAboutStop;
	std::function<void(float, float, int)> onBpmChange;

private:
	/* raw[*]
	Raw functions to start, stop and rewind the sequencer or change other
	properties. These functions must be called only internally. Other modules
	should send a SEQUENCER_* event to the Event Dispatcher. */

	void rawStart();
	void rawStop();
	void rawRewind(Frame delta);
	void rawSetBpm(float v, int sampleRate);
	void rawGoToBeat(int beat, int sampleRate);

	model::Model&     m_model;
	MidiSynchronizer& m_midiSynchronizer;
	JackTransport&    m_jackTransport;

	/* m_eventBuffer
	Buffer of events found in each block sent to channels for event parsing.
	This is filled during react(). */

	mutable EventBuffer m_eventBuffer;

	Metronome m_metronome;
	Quantizer m_quantizer;

	/* m_quantizerStep
	Tells how many frames to wait to perform a quantized action. */

	int m_quantizerStep;
};
} // namespace giada::m

#endif
