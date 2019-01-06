/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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
bool  running  = false;
float bpm      = G_DEFAULT_BPM;
int   bars     = G_DEFAULT_BARS;
int   beats    = G_DEFAULT_BEATS;
int   quantize = G_DEFAULT_QUANTIZE;
int   quanto   = 1;            // quantizer step

int framesInLoop = 0;
int framesInBar  = 0;
int framesInBeat = 0;
int framesInSeq  = 0;
int currentFrame = 0;
int currentBeat  = 0;

int midiTCrate    = 0;      // send MTC data every midiTCrate frames
int midiTCframes  = 0;
int midiTCseconds = 0;
int midiTCminutes = 0;
int midiTChours   = 0;

#ifdef G_OS_LINUX
kernelAudio::JackState jackStatePrev;
#endif


void updateQuanto()
{
	if (quantize != 0)
		quanto = framesInBeat / quantize;
}

}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init(int sampleRate, float midiTCfps)
{
	midiTCrate = (sampleRate / midiTCfps) * G_MAX_IO_CHANS;  // stereo values
	running    = false;
	bpm        = G_DEFAULT_BPM;
	bars       = G_DEFAULT_BARS;
	beats      = G_DEFAULT_BEATS;
	quantize   = G_DEFAULT_QUANTIZE;
	updateFrameBars();
}


/* -------------------------------------------------------------------------- */


bool isRunning()
{
	return running;
}


bool quantoHasPassed()
{
	return currentFrame % (quanto) == 0;
}


bool isOnBar()
{
	return currentFrame % framesInBar == 0;
}


bool isOnBeat()
{
	return currentFrame % framesInBeat == 0;
}


bool isOnFirstBeat()
{
	return currentFrame == 0;
}


void start()
{
	running = true;
	if (conf::midiSync == MIDI_SYNC_CLOCK_M) {
		kernelMidi::send(MIDI_START, -1, -1);
		kernelMidi::send(MIDI_POSITION_PTR, 0, 0);
	}
}


void stop()
{
	running = false;
	if (conf::midiSync == MIDI_SYNC_CLOCK_M)
		kernelMidi::send(MIDI_STOP, -1, -1);
}


void setBpm(float b)
{
	if (b < G_MIN_BPM)
		b = G_MIN_BPM;
	bpm = b;
	updateFrameBars();
}


void setBars(int newBars)
{
	/* Bars cannot be greater than beats and must be a sub multiple of beats. If
	not, approximate to the nearest (and greater) value available. */

	if (newBars > beats)
		bars = beats;
	else if (newBars <= 0)
		bars = 1;
	else if (beats % newBars != 0) {
		bars = newBars + (beats % newBars);
		if (beats % bars != 0) // it could be an odd value, let's check it (and avoid it)
			bars = bars - (beats % bars);
	}
	else
		bars = newBars;
}


void setBeats(int b)
{
	if (b > G_MAX_BEATS)
		beats = G_MAX_BEATS;
	else if (b < 1)
		beats = 1;
	else
		beats = b;
}


void setQuantize(int q)
{
	quantize = q;
	updateQuanto();
}


/* -------------------------------------------------------------------------- */


void incrCurrentFrame() {
	currentFrame++;
	if (currentFrame >= framesInLoop) {
		currentFrame = 0;
		currentBeat  = 0;
	}
	else
	if (isOnBeat())
		currentBeat++;
}


void rewind()
{
	currentFrame = 0;
	currentBeat  = 0;
	sendMIDIrewind();
}


/* -------------------------------------------------------------------------- */


void updateFrameBars()
{
	/* framesInLoop ... loop length in frames, or samplerate * # frames per 
	 *                  current bpm * beats;
	 * framesInBar .... n. of frames within a bar;
	 * framesInBeat ... n. of frames within a beat;
	 * framesInSeq .... number of frames in the whole sequencer. */

	framesInLoop = (conf::samplerate * (60.0f / bpm)) * beats;
	framesInBar  = framesInLoop / bars;
	framesInBeat = framesInLoop / beats;
	framesInSeq  = framesInBeat * G_MAX_BEATS;

	updateQuanto();
}


/* -------------------------------------------------------------------------- */


void sendMIDIsync()
{
	/* TODO - only Master (_M) is implemented so far. */

	if (conf::midiSync == MIDI_SYNC_CLOCK_M) {
		if (currentFrame % (framesInBeat/24) == 0)
			kernelMidi::send(MIDI_CLOCK, -1, -1);
		return;
	}

	if (conf::midiSync == MIDI_SYNC_MTC_M) {

		/* check if a new timecode frame has passed. If so, send MIDI TC
		 * quarter frames. 8 quarter frames, divided in two branches:
		 * 1-4 and 5-8. We check timecode frame's parity: if even, send
		 * range 1-4, if odd send 5-8. */

		if (currentFrame % midiTCrate != 0)  // no timecode frame passed
			return;

		/* frame low nibble
		 * frame high nibble
		 * seconds low nibble
		 * seconds high nibble */

		if (midiTCframes % 2 == 0) {
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCframes & 0x0F)  | 0x00, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCframes >> 4)    | 0x10, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCseconds & 0x0F) | 0x20, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCseconds >> 4)   | 0x30, -1);
		}

		/* minutes low nibble
		 * minutes high nibble
		 * hours low nibble
		 * hours high nibble SMPTE frame rate */

		else {
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCminutes & 0x0F) | 0x40, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCminutes >> 4)   | 0x50, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTChours & 0x0F)   | 0x60, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTChours >> 4)     | 0x70, -1);
		}

		midiTCframes++;

		/* check if total timecode frames are greater than timecode fps:
		 * if so, a second has passed */

		if (midiTCframes > conf::midiTCfps) {
			midiTCframes = 0;
			midiTCseconds++;
			if (midiTCseconds >= 60) {
				midiTCminutes++;
				midiTCseconds = 0;
				if (midiTCminutes >= 60) {
					midiTChours++;
					midiTCminutes = 0;
				}
			}
			//gu_log("%d:%d:%d:%d\n", midiTChours, midiTCminutes, midiTCseconds, midiTCframes);
		}
	}
}


/* -------------------------------------------------------------------------- */


void sendMIDIrewind()
{
	midiTCframes  = 0;
	midiTCseconds = 0;
	midiTCminutes = 0;
	midiTChours   = 0;

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

	if (jackState.running != jackStatePrev.running) {
		if (jackState.running) {
			if (!isRunning())
				c::transport::startSeq(false); // not from UI
		}
		else {
			if (isRunning())
				c::transport::stopSeq(false); // not from UI
		}
	}
	if (jackState.bpm != jackStatePrev.bpm)
		if (jackState.bpm > 1.0f)  // 0 bpm if Jack does not send that info
			c::main::setBpm(jackState.bpm);

	if (jackState.frame == 0 && jackState.frame != jackStatePrev.frame)
		c::transport::rewindSeq(false, false);  // not from UI, don't notify jack (avoid loop)

	jackStatePrev = jackState;
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
	return currentFrame;
}


int getFramesInLoop()
{
	return framesInLoop;
}


int getCurrentBeat()
{
	return currentBeat;
}


int getQuantize()
{
	return quantize;
}


float getBpm()
{
	return bpm;
}


int getBeats()
{
	return beats;
}


int getBars()
{
	return bars;
}


int getQuanto()
{
	return quanto;
}


int getFramesInBar()
{
	return framesInBar;
}


int getFramesInBeat()
{
	return framesInBeat;
}


int getFramesInSeq()
{
	return framesInSeq;
}


}}}; // giada::m::clock::
