/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include "utils/math.h"
#include "core/model/model.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/kernelAudio.h"
#include "core/mixerHandler.h"
#include "core/kernelMidi.h"
#include "clock.h"


namespace giada {
namespace m {
namespace clock
{
namespace
{
std::atomic<int> currentFrameWait_(0);
std::atomic<int> currentFrame_(0);
std::atomic<int> currentBeat_(0);

int quanto_ = 1;             // Quantizer step

int midiTCrate_    = 0;      // Send MTC data every midiTCrate_ frames
int midiTCframes_  = 0;
int midiTCseconds_ = 0;
int midiTCminutes_ = 0;
int midiTChours_   = 0;


#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)
kernelAudio::JackState jackStatePrev_;
#endif


/* -------------------------------------------------------------------------- */

/* updateFrameBars
Updates bpm, frames, beats and so on. */

void updateFrameBars_(model::Clock& c)
{
	c.framesInLoop = (conf::samplerate * (60.0f / c.bpm)) * c.beats;
	c.framesInBar  = c.framesInLoop / (float) c.bars;
	c.framesInBeat = c.framesInLoop / (float) c.beats;
	c.framesInSeq  = c.framesInBeat * G_MAX_BEATS;

	if (c.quantize != 0)
		quanto_ = c.framesInBeat / c.quantize;
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init(int sampleRate, float midiTCfps)
{
	midiTCrate_ = (sampleRate / midiTCfps) * G_MAX_IO_CHANS;  // stereo values

	model::onSwap(model::clock, [&](model::Clock& c)
	{
		c.bars     = G_DEFAULT_BARS;
		c.beats    = G_DEFAULT_BEATS;
		c.bpm      = G_DEFAULT_BPM;
		c.quantize = G_DEFAULT_QUANTIZE;
		updateFrameBars_(c);
	});
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
	return currentFrame_.load() % quanto_ == 0;
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
	b = u::math::bound(b, G_MIN_BPM, G_MAX_BPM);

	model::onSwap(model::clock, [&](model::Clock& c)
	{
		c.bpm = b;
		updateFrameBars_(c);
	});
}


void setBeats(int newBeats, int newBars)
{
	newBeats = u::math::bound(newBeats, 1, G_MAX_BEATS);
	newBars  = u::math::bound(newBars, 1, newBeats); // Bars cannot be greater than beats

	model::onSwap(model::clock, [&](model::Clock& c)
	{
		c.beats = newBeats;
		c.bars  = newBars;
		updateFrameBars_(c);
	});
}


void setQuantize(int q)
{
	model::onSwap(model::clock, [&](model::Clock& c)
	{
		c.quantize = q;	
		updateFrameBars_(c);
	});
}


void setStatus(ClockStatus s)
{
	model::onSwap(model::clock, [&](model::Clock& c)
	{
		c.status = s;
	});
	
	if (s == ClockStatus::RUNNING) {
		if (conf::midiSync == MIDI_SYNC_CLOCK_M) {
			kernelMidi::send(MIDI_START, -1, -1);
			kernelMidi::send(MIDI_POSITION_PTR, 0, 0);
		}
	}
	else
	if (s == ClockStatus::STOPPED) {
		if (conf::midiSync == MIDI_SYNC_CLOCK_M)
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
		if (f >= c->framesInLoop)
				f = 0;
		currentFrameWait_.store(f);
		return;
	}

	int f = currentFrame_.load() + 1;
	int b = currentBeat_.load();

	if (f >= c->framesInLoop) {
		f = 0;
		b = 0;
	}
	else
	if (f % c->framesInBeat == 0) // If is on beat
		b++;
	
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

	if (conf::midiSync == MIDI_SYNC_CLOCK_M) {
		if (currentFrame % (c->framesInBeat / 24) == 0)
			kernelMidi::send(MIDI_CLOCK, -1, -1);
		return;
	}

	if (conf::midiSync == MIDI_SYNC_MTC_M) {

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

		if (midiTCframes_ > conf::midiTCfps) {
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

	if (conf::midiSync == MIDI_SYNC_MTC_M) {
		kernelMidi::send(MIDI_SYSEX, 0x7F, 0x00);  // send msg on channel 0
		kernelMidi::send(0x01, 0x01, 0x00);        // hours 0
		kernelMidi::send(0x00, 0x00, 0x00);        // mins, secs, frames 0
		kernelMidi::send(MIDI_EOX, -1, -1);        // end of sysex
	}
}


/* -------------------------------------------------------------------------- */


#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)

void recvJackSync()
{
	/* TODO - these things should be processed by a higher level, 
	above clock:: ----> clockManager */

	kernelAudio::JackState jackState = kernelAudio::jackTransportQuery();

	if (jackState.running != jackStatePrev_.running) {
		if (jackState.running) {
			if (!isRunning())
				mh::startSequencer();
		}
		else {
			if (isRunning())
				mh::stopSequencer();
		}
	}
	if (jackState.bpm != jackStatePrev_.bpm)
		if (jackState.bpm > 1.0f)  // 0 bpm if Jack does not send that info
			c::main::setBpm(jackState.bpm);

	if (jackState.frame == 0 && jackState.frame != jackStatePrev_.frame)
		mh::rewindSequencer();

	jackStatePrev_ = jackState;
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


int         getCurrentFrame() { return currentFrame_.load(); }
int         getCurrentBeat()  { return currentBeat_.load(); }
int         getQuanto()       { return quanto_; }
ClockStatus getStatus()       { model::ClockLock lock(model::clock); return model::clock.get()->status; }
int         getFramesInLoop() { model::ClockLock lock(model::clock); return model::clock.get()->framesInLoop; }
int         getFramesInBar()  { model::ClockLock lock(model::clock); return model::clock.get()->framesInBar; }
int         getFramesInBeat() { model::ClockLock lock(model::clock); return model::clock.get()->framesInBeat; }
int         getFramesInSeq()  { model::ClockLock lock(model::clock); return model::clock.get()->framesInSeq; }
int         getQuantize()     { model::ClockLock lock(model::clock); return model::clock.get()->quantize; }
float       getBpm()          { model::ClockLock lock(model::clock); return model::clock.get()->bpm; }
int         getBeats()        { model::ClockLock lock(model::clock); return model::clock.get()->beats; }
int         getBars()         { model::ClockLock lock(model::clock); return model::clock.get()->bars; }
}}}; // giada::m::clock::
