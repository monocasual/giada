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
#include "core/model/model.h"
#include "conf.h"
#include "const.h"
#include "kernelAudio.h"
#include "mixerHandler.h"
#include "kernelMidi.h"
#include "clock.h"


namespace giada {
namespace m {
namespace clock
{
namespace
{
int quanto_ = 1; // quantizer step

std::atomic<ClockStatus> status_(ClockStatus::STOPPED);

std::atomic<int> framesInLoop_(0);
std::atomic<int> framesInBar_ (0);
std::atomic<int> framesInBeat_(0);
std::atomic<int> framesInSeq_ (0);
std::atomic<int> currentFrameWait_(0);  // Used only in wait mode
std::atomic<int> currentFrame_(0);
std::atomic<int> currentBeat_(0);

int midiTCrate_    = 0;      // Send MTC data every midiTCrate_ frames
int midiTCframes_  = 0;
int midiTCseconds_ = 0;
int midiTCminutes_ = 0;
int midiTChours_   = 0;

#ifdef G_OS_LINUX
kernelAudio::JackState jackStatePrev_;
#endif


/* -------------------------------------------------------------------------- */

/* updateFrameBars
Updates bpm, frames, beats and so on. */

void updateFrameBars_()
{
	int   bars     = model::getLayout()->bars;
	int   beats    = model::getLayout()->beats;
	float bpm      = model::getLayout()->bpm;
    int   quantize = model::getLayout()->quantize;
	int   fil      = (conf::samplerate * (60.0f / bpm)) * beats;
	int   fib      = fil / (float) beats;

	framesInLoop_.store(fil);
	framesInBar_.store(fil / (float) bars);
	framesInBeat_.store(fib);
	framesInSeq_.store(fib * G_MAX_BEATS);

	if (quantize != 0)
		quanto_ = fib / quantize;
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init(int sampleRate, float midiTCfps)
{
	midiTCrate_ = (sampleRate / midiTCfps) * G_MAX_IO_CHANS;  // stereo values
	updateFrameBars_();
}


/* -------------------------------------------------------------------------- */


bool isRunning()
{
	return status_.load() == ClockStatus::RUNNING;
}


bool isActive()
{
	ClockStatus status = status_.load();
	return status == ClockStatus::RUNNING || status == ClockStatus::WAITING;
}


bool quantoHasPassed()
{
	return currentFrame_.load() % (quanto_) == 0;
}


bool isOnBar()
{
	ClockStatus status      = status_.load();
    int         framesInBar = framesInBar_.load();

	if (status == ClockStatus::WAITING || isOnFirstBeat())
		return false;
	return currentFrame_.load() % framesInBar == 0;
}


bool isOnBeat()
{
	ClockStatus status       = status_.load();
    int         framesInBeat = framesInBeat_.load();
	
	if (status == ClockStatus::WAITING)
		return currentFrameWait_.load() % framesInBeat == 0;
	return currentFrame_.load() % framesInBeat == 0;
}


bool isOnFirstBeat()
{
	return currentFrame_.load() == 0;
}


/* -------------------------------------------------------------------------- */


void setBpm(float b)
{
	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	
	if (b < G_MIN_BPM)
		b = G_MIN_BPM;
	
	layout->bpm = b;
	
	model::swapLayout(layout);
	
	updateFrameBars_();
}


void setBeats(int newBeats, int newBars)
{
	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	
	if (newBeats > G_MAX_BEATS)
		newBeats = G_MAX_BEATS;
	else if (newBeats < 1)
		newBeats = 1;
	
	/* Bars cannot be greater than beats. */
	
	if (newBars > newBeats)
		newBars = newBeats;
	else if (newBars <= 0)
		newBars = 1;

	layout->beats = newBeats;
	layout->bars  = newBars;
	
	model::swapLayout(layout);
	
	updateFrameBars_();
}


void setQuantize(int q)
{
	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	
	layout->quantize = q;	
	
	model::swapLayout(layout);
	
	updateFrameBars_();
}


void setStatus(ClockStatus s)
{
	status_.store(s);
	
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
	ClockStatus status       = status_.load();
	int         framesInLoop = framesInLoop_.load();
	int         framesInBeat = framesInBeat_.load();

	if (status == ClockStatus::WAITING) {
		int f = currentFrameWait_.load() + 1;
		if (f >= framesInLoop)
			f = 0;
		currentFrameWait_.store(f);
		return;
	}

	int f = currentFrame_.load() + 1;
	int b = currentBeat_.load();

	if (f >= framesInLoop) {
		f = 0;
		b = 0;
	}
	else
	if (f % framesInBeat == 0) // If is on beat
		b++;

	currentFrame_.store(f);
	currentBeat_.store(b);
}


void rewind()
{
	currentFrameWait_.store(0);
	currentFrame_.store(0);
	currentBeat_.store(0);
	sendMIDIrewind();
}


/* -------------------------------------------------------------------------- */


void sendMIDIsync()
{
	ClockStatus status       = status_.load();
	int         framesInBeat = framesInBeat_.load();
	int         currentFrame = currentFrame_.load();	
	
	/* Sending MIDI sync while waiting is meaningless. */

	if (status == ClockStatus::WAITING)
		return;

	/* TODO - only Master (_M) is implemented so far. */

	if (conf::midiSync == MIDI_SYNC_CLOCK_M) {
		if (currentFrame % (framesInBeat / 24) == 0)
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
			//gu_log("%d:%d:%d:%d\n", midiTChours_, midiTCminutes_, midiTCseconds_, midiTCframes_);
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


#ifdef G_OS_LINUX

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
	return getQuantize() > 0 && isRunning();
}


/* -------------------------------------------------------------------------- */


ClockStatus getStatus()       { return status_.load(); }
int         getCurrentFrame() { return currentFrame_.load(); }
int         getCurrentBeat()  { return currentBeat_.load(); }
int         getFramesInLoop() { return framesInLoop_.load(); }
int         getFramesInBar()  { return framesInBar_.load(); }
int         getFramesInBeat() { return framesInBeat_.load(); }
int         getFramesInSeq()  { return framesInSeq_.load(); }
int         getQuanto()       { return quanto_; }
int         getQuantize()     { return model::getLayout()->quantize; }
float       getBpm()          { return model::getLayout()->bpm; }
int         getBeats()        { return model::getLayout()->beats; }
int         getBars()         { return model::getLayout()->bars; }

}}}; // giada::m::clock::
