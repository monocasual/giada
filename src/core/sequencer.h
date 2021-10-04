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

#ifndef G_SEQUENCER_H
#define G_SEQUENCER_H

#include "core/eventDispatcher.h"
#include "core/metronome.h"
#include "core/quantizer.h"
#include <vector>

namespace mcl
{
class AudioBuffer;
}

namespace giada::m::model
{
class Model;
}

namespace giada::m
{
class JackTransport;
class ActionRecorder;
class Synchronizer;
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

	Sequencer(model::Model&, Synchronizer&, JackTransport&);

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

	/* getMaxFramesInLoop
    Returns how many frames the current loop length might contain at the slowest
    speed possible (G_MIN_BPM). Call this whenever you change the number or 
    beats. */

	Frame getMaxFramesInLoop(int sampleRate) const;

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

	/* react
	Reacts to live events coming from the EventDispatcher (human events). */

	void react(const EventDispatcher::EventBuffer&);

	/* advance
	Parses sequencer events that might occur in a block and advances the internal 
	quantizer. Returns a reference to the internal EventBuffer filled with events
	(if any). Call this on each new audio block. */

	const EventBuffer& advance(Frame bufferSize, const ActionRecorder&);

	/* render
	Renders audio coming out from the sequencer: that is, the metronome! */

	void render(mcl::AudioBuffer& outBuf);

	/* raw[*]
	Raw functions to start, stop and rewind the sequencer. These functions must 
	be called only when the JACK signal is received. Other modules should send
	a SEQUENCER_* event to the Event Dispatcher. */

	void rawStart();
	void rawStop();
	void rawRewind();

	/* rawSetBpm
	Raw function to set the bpm, bypassing any JACK instruction. This function 
	must be called only by the Synchronizer when the JACK signal is received. 
	Other modules should use the non-raw version below. */

	void rawSetBpm(float v, int sampleRate);

	void rewind();
	void toggleMetronome();
	void setMetronome(bool v);
	void setBpm(float b, int sampleRate);
	void setBeats(int beats, int bars, int sampleRate);
	void setQuantize(int q, int sampleRate);
	void setStatus(SeqStatus);

	/* recomputeFrames
    Updates bpm, frames, beats and so on. */

	void recomputeFrames(int sampleRate);

	/* quantizer
	Used by the sequencer itself and each sample channel. */

	Quantizer quantizer;

	std::function<void(SeqStatus)>         onAboutStart;
	std::function<void()>                  onAboutStop;
	std::function<void(float, float, int)> onBpmChange;

private:
	/* rewindQ
	Rewinds sequencer, quantized mode. */

	void rewindQ(Frame delta);

	model::Model&  m_model;
	Synchronizer&  m_synchronizer;
	JackTransport& m_jackTransport;

	/* m_eventBuffer
	Buffer of events found in each block sent to channels for event parsing. 
	This is filled during react(). */

	EventBuffer m_eventBuffer;

	Metronome m_metronome;

	/* m_quantizerStep
    Tells how many frames to wait to perform a quantized action. */

	int m_quantizerStep;
};
} // namespace giada::m

#endif
