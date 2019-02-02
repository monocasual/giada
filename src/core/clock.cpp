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
#include "../glue/transport.h"
#include "../glue/main.h"
#include "conf.h"
#include "const.h"
#include "kernelAudio.h"
#include "kernelMidi.h"
#include "clock.h"


namespace giada {
namespace m {
namespace clock
{
namespace
{
float bpm_      = G_DEFAULT_BPM;
int   bars_     = G_DEFAULT_BARS;
int   beats_    = G_DEFAULT_BEATS;
int   quantize_ = G_DEFAULT_QUANTIZE;
int   quanto_   = 1;            // quantizer step
std::atomic<bool> running_(false);

int framesInLoop_ = 0;
int framesInBar_  = 0;
int framesInBeat_ = 0;
int framesInSeq_  = 0;
std::atomic<int> currentFrame_(0);
std::atomic<int> currentBeat_ (0);

int midiTCrate_    = 0;      // send MTC data every midiTCrate_ frames
int midiTCframes_  = 0;
int midiTCseconds_ = 0;
int midiTCminutes_ = 0;
int midiTChours_   = 0;

#ifdef G_OS_LINUX
kernelAudio::JackState jackStatePrev_;
#endif


void updateQuanto_()
{
	if (quantize_ != 0)
		quanto_ = framesInBeat_ / quantize_;
}

}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init(int sampleRate, float midiTCfps)
{
	running_.load(false);
	midiTCrate_ = (sampleRate / midiTCfps) * G_MAX_IO_CHANS;  // stereo values
	bpm_        = G_DEFAULT_BPM;
	bars_       = G_DEFAULT_BARS;
	beats_      = G_DEFAULT_BEATS;
	quantize_   = G_DEFAULT_QUANTIZE;
	updateFrameBars();
}


/* -------------------------------------------------------------------------- */


bool isRunning()
{
	return running_.load();
}


bool quantoHasPassed()
{
	return currentFrame_.load() % (quanto_) == 0;
}


bool isOnBar()
{
	return currentFrame_.load() % framesInBar_ == 0;
}


bool isOnBeat()
{
	return currentFrame_.load() % framesInBeat_ == 0;
}


bool isOnFirstBeat()
{
	return currentFrame_.load() == 0;
}


void start()
{
	running_.store(true);
	if (conf::midiSync == MIDI_SYNC_CLOCK_M) {
		kernelMidi::send(MIDI_START, -1, -1);
		kernelMidi::send(MIDI_POSITION_PTR, 0, 0);
	}
}


void stop()
{
	running_.store(false);
	if (conf::midiSync == MIDI_SYNC_CLOCK_M)
		kernelMidi::send(MIDI_STOP, -1, -1);
}


void setBpm(float b)
{
	if (b < G_MIN_BPM)
		b = G_MIN_BPM;
	bpm_ = b;
	updateFrameBars();
}


void setBars(int newBars)
{
	/* Bars cannot be greater than beats_ and must be a sub multiple of beats_. If
	not, approximate to the nearest (and greater) value available. */

	if (newBars > beats_)
		bars_ = beats_;
	else if (newBars <= 0)
		bars_ = 1;
	else if (beats_ % newBars != 0) {
		bars_ = newBars + (beats_ % newBars);
		if (beats_ % bars_ != 0) // it could be an odd value, let's check it (and avoid it)
			bars_ = bars_ - (beats_ % bars_);
	}
	else
		bars_ = newBars;
}


void setBeats(int b)
{
	if (b > G_MAX_BEATS)
		beats_ = G_MAX_BEATS;
	else if (b < 1)
		beats_ = 1;
	else
		beats_ = b;
}


void setQuantize(int q)
{
	quantize_ = q;
	updateQuanto_();
}


/* -------------------------------------------------------------------------- */


void incrCurrentFrame() {
	currentFrame_++;
	if (currentFrame_.load() >= framesInLoop_) {
		currentFrame_.store(0);
		currentBeat_.store(0);
	}
	else
	if (isOnBeat())
		currentBeat_++;
}


void rewind()
{
	currentFrame_.store(0);
	currentBeat_.store(0);
	sendMIDIrewind();
}


/* -------------------------------------------------------------------------- */


void updateFrameBars()
{
	/* framesInLoop_ ... loop length in frames, or samplerate * # frames per 
	 *                  current bpm_ * beats_;
	 * framesInBar_ .... n. of frames within a bar;
	 * framesInBeat_ ... n. of frames within a beat;
	 * framesInSeq_ .... number of frames in the whole sequencer. */

	framesInLoop_ = (conf::samplerate * (60.0f / bpm_)) * beats_;
	framesInBar_  = framesInLoop_ / bars_;
	framesInBeat_ = framesInLoop_ / beats_;
	framesInSeq_  = framesInBeat_ * G_MAX_BEATS;

	updateQuanto_();
}


/* -------------------------------------------------------------------------- */


void sendMIDIsync()
{
	/* TODO - only Master (_M) is implemented so far. */

	if (conf::midiSync == MIDI_SYNC_CLOCK_M) {
		if (currentFrame_.load() % (framesInBeat_/24) == 0)
			kernelMidi::send(MIDI_CLOCK, -1, -1);
		return;
	}

	if (conf::midiSync == MIDI_SYNC_MTC_M) {

		/* check if a new timecode frame has passed. If so, send MIDI TC
		 * quarter frames. 8 quarter frames, divided in two branches:
		 * 1-4 and 5-8. We check timecode frame's parity: if even, send
		 * range 1-4, if odd send 5-8. */

		if (currentFrame_.load() % midiTCrate_ != 0)  // no timecode frame passed
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
	kernelAudio::JackState jackState = kernelAudio::jackTransportQuery();

	if (jackState.running_ != jackStatePrev_.running_) {
		if (jackState.running_) {
			if (!isRunning())
				c::transport::startSeq(false); // not from UI
		}
		else {
			if (isRunning())
				c::transport::stopSeq(false); // not from UI
		}
	}
	if (jackState.bpm_ != jackStatePrev_.bpm_)
		if (jackState.bpm_ > 1.0f)  // 0 bpm_ if Jack does not send that info
			c::main::setBpm(jackState.bpm_);

	if (jackState.frame == 0 && jackState.frame != jackStatePrev_.frame)
		c::transport::rewindSeq(false, false);  // not from UI, don't notify jack (avoid loop)

	jackStatePrev_ = jackState;
}

#endif


/* -------------------------------------------------------------------------- */


bool canQuantize()
{
	return getQuantize() > 0 && isRunning();
}


/* -------------------------------------------------------------------------- */


int getCurrentFrame()
{
	return currentFrame_.load();
}


int getFramesInLoop()
{
	return framesInLoop_;
}


int getCurrentBeat()
{
	return currentBeat_.load();
}


int getQuantize()
{
	return quantize_;
}


float getBpm()
{
	return bpm_;
}


int getBeats()
{
	return beats_;
}


int getBars()
{
	return bars_;
}


int getQuanto()
{
	return quanto_;
}


int getFramesInBar()
{
	return framesInBar_;
}


int getFramesInBeat()
{
	return framesInBeat_;
}


int getFramesInSeq()
{
	return framesInSeq_;
}


}}}; // giada::m::clock::
