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


#include <atomic>
#include <cassert>
#include "glue/main.h"
#include "glue/events.h"
#include "core/model/model.h"
#include "core/conf.h"
#include "core/sequencer.h"
#include "core/const.h"
#include "core/kernelAudio.h"
#include "core/mixerHandler.h"
#include "core/kernelMidi.h"
#include "utils/math.h"
#include "clock.h"


namespace giada::m::clock
{
namespace
{
std::atomic<int> currentFrameWait_(0);
std::atomic<int> currentFrame_(0);
std::atomic<int> currentBeat_(0);

/* quantizerStep_
Tells how many frames to wait to perform a quantized action. */

int quantizerStep_ = 1;

/* midiTC*
MIDI timecode variables. */

int midiTCrate_    = 0;      // Send MTC data every midiTCrate_ frames
int midiTCframes_  = 0;
int midiTCseconds_ = 0;
int midiTCminutes_ = 0;
int midiTChours_   = 0;

#ifdef WITH_AUDIO_JACK
kernelAudio::JackState jackStatePrev_;
#endif


/* -------------------------------------------------------------------------- */

/* recomputeFrames_
Updates bpm, frames, beats and so on. Private version. */

void recomputeFrames_(model::Clock& c)
{
	c.framesInLoop = static_cast<int>((conf::conf.samplerate * (60.0f / c.bpm)) * c.beats);
	c.framesInBar  = static_cast<int>(c.framesInLoop / (float) c.bars);
	c.framesInBeat = static_cast<int>(c.framesInLoop / (float) c.beats);
	c.framesInSeq  = c.framesInBeat * G_MAX_BEATS;

	if (c.quantize != 0)
		quantizerStep_ = c.framesInBeat / c.quantize;
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init(int sampleRate, float midiTCfps)
{
	midiTCrate_ = static_cast<int>((sampleRate / midiTCfps) * G_MAX_IO_CHANS);  // stereo values

	model::onSwap(model::clock, [&](model::Clock& c)
	{
		c.bars     = G_DEFAULT_BARS;
		c.beats    = G_DEFAULT_BEATS;
		c.bpm      = G_DEFAULT_BPM;
		c.quantize = G_DEFAULT_QUANTIZE;
		recomputeFrames_(c);
	});
}


/* -------------------------------------------------------------------------- */


void recomputeFrames()
{
	model::onSwap(model::clock, [&](model::Clock& c) { recomputeFrames_(c);	});
}


/* -------------------------------------------------------------------------- */


bool isRunning()
{
	model::ClockLock lock(model::clock);

	return model::clock.get()->status == ClockStatus::RUNNING;
}


bool isActive()
{
	model::ClockLock lock(model::clock);
	
	ClockStatus status = model::clock.get()->status;
	return status == ClockStatus::RUNNING || status == ClockStatus::WAITING;
}


bool quantoHasPassed()
{
	return clock::getQuantizerValue() != 0 && currentFrame_.load() % quantizerStep_ == 0;
}


bool isOnBar()
{
	model::ClockLock lock(model::clock);

	const model::Clock* c = model::clock.get();
	
	int currentFrame = currentFrame_.load();

	if (c->status == ClockStatus::WAITING || currentFrame == 0)
		return false;
	return currentFrame % c->framesInBar == 0;
}


bool isOnBeat()
{
	model::ClockLock lock(model::clock);
	
	const model::Clock* c = model::clock.get();
	
	if (c->status == ClockStatus::WAITING)
		return currentFrameWait_.load() % c->framesInBeat == 0;
	return currentFrame_.load() % c->framesInBeat == 0;
}


bool isOnFirstBeat()
{
	return currentFrame_.load() == 0;
}


/* -------------------------------------------------------------------------- */


void setBpm(float b)
{	
	b = std::clamp(b, G_MIN_BPM, G_MAX_BPM);

	model::onSwap(model::clock, [&](model::Clock& c)
	{
		c.bpm = b;
		recomputeFrames_(c);
	});
}


void setBeats(int newBeats, int newBars)
{
	newBeats = std::clamp(newBeats, 1, G_MAX_BEATS);
	newBars  = std::clamp(newBars, 1, newBeats); // Bars cannot be greater than beats

	model::onSwap(model::clock, [&](model::Clock& c)
	{
		c.beats = newBeats;
		c.bars  = newBars;
		recomputeFrames_(c);
	});
}


void setQuantize(int q)
{
	model::onSwap(model::clock, [&](model::Clock& c)
	{
		c.quantize = q;	
		recomputeFrames_(c);
	});
}


void setStatus(ClockStatus s)
{
	model::onSwap(model::clock, [&](model::Clock& c)
	{
		c.status = s;
	});
	
	if (s == ClockStatus::RUNNING) {
		if (conf::conf.midiSync == MIDI_SYNC_CLOCK_M) {
			kernelMidi::send(MIDI_START, -1, -1);
			kernelMidi::send(MIDI_POSITION_PTR, 0, 0);
		}
	}
	else
	if (s == ClockStatus::STOPPED) {
		if (conf::conf.midiSync == MIDI_SYNC_CLOCK_M)
			kernelMidi::send(MIDI_STOP, -1, -1);
	}
}


/* -------------------------------------------------------------------------- */


void incrCurrentFrame() 
{
	model::ClockLock lock(model::clock);
	
	const model::Clock* c = model::clock.get();

	if (c->status == ClockStatus::WAITING) {
		int f = currentFrameWait_.load() + 1;
		f %= c->framesInLoop;
		currentFrameWait_.store(f);
		return;
	}

	int f = currentFrame_.load() + 1;
	int b = currentBeat_.load();

	f %= c->framesInLoop;
	b = f / c->framesInBeat;
	
	currentFrame_.store(f);
	currentBeat_.store(b);
}


void rewind()
{
	currentFrame_.store(0);
	currentBeat_.store(0);
	currentFrameWait_.store(0);
	
	sendMIDIrewind();
}


/* -------------------------------------------------------------------------- */


void sendMIDIsync()
{
	model::ClockLock lock(model::clock);
	
	const model::Clock* c = model::clock.get();
	
	/* Sending MIDI sync while waiting is meaningless. */

	if (c->status == ClockStatus::WAITING)
		return;

	int currentFrame = currentFrame_.load();

	/* TODO - only Master (_M) is implemented so far. */

	if (conf::conf.midiSync == MIDI_SYNC_CLOCK_M) {
		if (currentFrame % (c->framesInBeat / 24) == 0)
			kernelMidi::send(MIDI_CLOCK, -1, -1);
		return;
	}

	if (conf::conf.midiSync == MIDI_SYNC_MTC_M) {

		/* check if a new timecode frame has passed. If so, send MIDI TC
		 * quarter frames. 8 quarter frames, divided in two branches:
		 * 1-4 and 5-8. We check timecode frame's parity: if even, send
		 * range 1-4, if odd send 5-8. */

		if (currentFrame % midiTCrate_ != 0)  // no timecode frame passed
			return;

		/* frame low nibble
		 * frame high nibble
		 * seconds low nibble
		 * seconds high nibble */

		if (midiTCframes_ % 2 == 0) {
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCframes_ & 0x0F)  | 0x00, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCframes_ >> 4)    | 0x10, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCseconds_ & 0x0F) | 0x20, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCseconds_ >> 4)   | 0x30, -1);
		}

		/* minutes low nibble
		 * minutes high nibble
		 * hours low nibble
		 * hours high nibble SMPTE frame rate */

		else {
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCminutes_ & 0x0F) | 0x40, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCminutes_ >> 4)   | 0x50, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTChours_ & 0x0F)   | 0x60, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTChours_ >> 4)     | 0x70, -1);
		}

		midiTCframes_++;

		/* check if total timecode frames are greater than timecode fps:
		 * if so, a second has passed */

		if (midiTCframes_ > conf::conf.midiTCfps) {
			midiTCframes_ = 0;
			midiTCseconds_++;
			if (midiTCseconds_ >= 60) {
				midiTCminutes_++;
				midiTCseconds_ = 0;
				if (midiTCminutes_ >= 60) {
					midiTChours_++;
					midiTCminutes_ = 0;
				}
			}
			//u::log::print("%d:%d:%d:%d\n", midiTChours_, midiTCminutes_, midiTCseconds_, midiTCframes_);
		}
	}
}


/* -------------------------------------------------------------------------- */


void sendMIDIrewind()
{
	midiTCframes_  = 0;
	midiTCseconds_ = 0;
	midiTCminutes_ = 0;
	midiTChours_   = 0;

	/* For cueing the slave to a particular start point, Quarter Frame
	 * messages are not used. Instead, an MTC Full Frame message should
	 * be sent. The Full Frame is a SysEx message that encodes the entire
	 * SMPTE time in one message */

	if (conf::conf.midiSync == MIDI_SYNC_MTC_M) {
		kernelMidi::send(MIDI_SYSEX, 0x7F, 0x00);  // send msg on channel 0
		kernelMidi::send(0x01, 0x01, 0x00);        // hours 0
		kernelMidi::send(0x00, 0x00, 0x00);        // mins, secs, frames 0
		kernelMidi::send(MIDI_EOX, -1, -1);        // end of sysex
	}
	else
	if (conf::conf.midiSync == MIDI_SYNC_CLOCK_M)
		kernelMidi::send(MIDI_POSITION_PTR, 0, 0);
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_AUDIO_JACK

void recvJackSync()
{
	/* TODO - these things should be processed by a higher level, 
	above clock:: ----> clockManager */

	kernelAudio::JackState jackStateCurr = kernelAudio::jackTransportQuery();

	if (jackStateCurr != jackStatePrev_) {	

		if (jackStateCurr.frame != jackStatePrev_.frame && jackStateCurr.frame == 0) {
G_DEBUG("JackState received - rewind to frame 0");
			sequencer::rewind();
		}

		if (jackStateCurr.bpm != jackStatePrev_.bpm && jackStateCurr.bpm > 1.0f) {  // 0 bpm if Jack does not send that info
G_DEBUG("JackState received - bpm=" << jackStateCurr.bpm);
			c::main::setBpm(jackStateCurr.bpm);
		}

		if (jackStateCurr.running != jackStatePrev_.running) {
G_DEBUG("JackState received - running=" << jackStateCurr.running);			
			jackStateCurr.running ? sequencer::start() : sequencer::stop();
		}
	}

	jackStatePrev_ = jackStateCurr;
}

#endif


/* -------------------------------------------------------------------------- */


bool canQuantize()
{
	model::ClockLock lock(model::clock);
	
	const model::Clock* c = model::clock.get();
	return c->quantize > 0 && c->status == ClockStatus::RUNNING;
}


/* -------------------------------------------------------------------------- */


Frame quantize(Frame f)
{
	if (!canQuantize()) return f;
	return u::math::quantize(f, quantizerStep_) % getFramesInLoop(); // No overflow
}


/* -------------------------------------------------------------------------- */


int         getCurrentFrame()   { return currentFrame_.load(); }
int         getCurrentBeat()    { return currentBeat_.load(); }
int         getQuantizerStep()  { return quantizerStep_; }
ClockStatus getStatus()         { model::ClockLock lock(model::clock); return model::clock.get()->status; }
int         getFramesInLoop()   { model::ClockLock lock(model::clock); return model::clock.get()->framesInLoop; }
int         getFramesInBar()    { model::ClockLock lock(model::clock); return model::clock.get()->framesInBar; }
int         getFramesInBeat()   { model::ClockLock lock(model::clock); return model::clock.get()->framesInBeat; }
int         getFramesInSeq()    { model::ClockLock lock(model::clock); return model::clock.get()->framesInSeq; }
int         getQuantizerValue() { model::ClockLock lock(model::clock); return model::clock.get()->quantize; }
float       getBpm()            { model::ClockLock lock(model::clock); return model::clock.get()->bpm; }
int         getBeats()          { model::ClockLock lock(model::clock); return model::clock.get()->beats; }
int         getBars()           { model::ClockLock lock(model::clock); return model::clock.get()->bars; }
} // giada::m::clock::
