/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


using namespace giada;
namespace gc = giada::clock;


namespace
{
  //KernelAudio *kernelAudio;
  KernelMidi  *kernelMidi;
  Conf        *conf;

  bool  running;
  float bpm;
	int   bars;
	int   beats;
  int   quantize;
	int   quanto;            // quantizer step
  int   framesPerBar;      // frames in one bar
	int   framesPerBeat;     // frames in one beat
	int   framesInSequencer; // frames in the whole sequencer
	int   totalFrames;       // frames in the selected range (e.g. 4/4)
	int   currentFrame;
	int   currentBeat;

	int midiTCrate;      // send MTC data every midiTCrate frames
	int midiTCframes;
	int midiTCseconds;
	int midiTCminutes;
	int midiTChours;

#ifdef __linux__
  kernelAudio::JackState jackStatePrev;
#endif

  void updateQuanto()
  {
    if (quantize != 0)
      quanto = framesPerBeat / quantize;
    if (quanto % 2 != 0)
      quanto++;
  }
} // ::


/* -------------------------------------------------------------------------- */


void gc::init(KernelMidi *kernelMidi, Conf *conf)
{
  assert(kernelMidi != nullptr);
  assert(conf != nullptr);

  kernelMidi        = kernelMidi;
  conf              = conf;
  running           = false;
  bpm               = G_DEFAULT_BPM;
  bars              = G_DEFAULT_BARS;
  beats             = G_DEFAULT_BEATS;
  quantize          = G_DEFAULT_QUANTIZE;
  quanto            = 1;
  framesPerBar      = 0;
  framesPerBeat     = 0;
  framesInSequencer = 0;
  totalFrames       = 0;
  currentFrame      = 0;
  currentBeat       = 0;
  midiTCrate        = (conf->samplerate / conf->midiTCfps) * 2;  // stereo values
  midiTCframes      = 0;
  midiTCseconds     = 0;
  midiTCminutes     = 0;
  midiTChours       = 0;
}


/* -------------------------------------------------------------------------- */


bool gc::isRunning()
{
  return running;
}


bool gc::quantoHasPassed()
{
  return currentFrame % (quanto) == 0;
}


bool gc::isOnBar()
{
  /* A bar cannot occur at frame 0. That's the first beat. */
  return currentFrame % framesPerBar == 0 && currentFrame != 0;
}


bool gc::isOnBeat()
{
  /* Skip frame 0: it is intended as 'first beat'. */
  /* TODO - this is wrong! */
  return currentFrame % framesPerBeat == 0 && currentFrame > 0;
}


bool gc::isOnFirstBeat()
{
  return currentFrame == 0;
}


void gc::start()
{
  running = true;
	if (conf->midiSync == MIDI_SYNC_CLOCK_M) {
		kernelMidi->send(MIDI_START, -1, -1);
		kernelMidi->send(MIDI_POSITION_PTR, 0, 0);
	}
}


void gc::stop()
{
  running = false;
  if (conf->midiSync == MIDI_SYNC_CLOCK_M)
  	kernelMidi->send(MIDI_STOP, -1, -1);
}


void gc::setBpm(float b)
{
  if (b < G_MIN_BPM)
    b = G_MIN_BPM;
  bpm = b;
	updateFrameBars();
}


void gc::setBars(int b)
{
  bars = b;
}


void gc::setBeats(int b)
{
  beats = b;
}


void gc::setQuantize(int q)
{
  quantize = q;
  updateQuanto();
}


/* -------------------------------------------------------------------------- */


void gc::incrCurrentFrame()
{
  currentFrame += 2;
  if (currentFrame > totalFrames) {
		currentFrame = 0;
		currentBeat  = 0;
	}
  else
  if (isOnBeat())
    currentBeat++;
}


void gc::rewind()
{
  currentFrame = 0;
  currentBeat = 0;
  sendMIDIrewind();
}


/* -------------------------------------------------------------------------- */


void gc::updateFrameBars()
{
	/* seconds ....... total time of play (in seconds) of the whole
	 *                 sequencer. 60 / bpm == how many seconds lasts one bpm
	 * totalFrames ... loop length in frames, x2 because it's stereo
	 * framesPerBar .. n. of frames within a bar
	 * framesPerBeat . n. of frames within a beat
   * framesInSeq ... number of frames in the whole sequencer */

	float seconds     = (60.0f / bpm) * beats;
	totalFrames       = conf->samplerate * seconds * 2;
	framesPerBar      = totalFrames / bars;
	framesPerBeat     = totalFrames / beats;
	framesInSequencer = framesPerBeat * G_MAX_BEATS;

	/* big troubles if frames are odd. */

	if (totalFrames % 2 != 0)
		totalFrames--;
	if (framesPerBar % 2 != 0)
		framesPerBar--;
	if (framesPerBeat % 2 != 0)
		framesPerBeat--;

  updateQuanto();
}


/* -------------------------------------------------------------------------- */


void gc::sendMIDIsync()
{
  /* TODO - only Master (_M) is implemented so far. */

	if (conf->midiSync == MIDI_SYNC_CLOCK_M) {
		if (currentFrame % (framesPerBeat/24) == 0)
			kernelMidi->send(MIDI_CLOCK, -1, -1);
    return;
  }

	if (conf->midiSync == MIDI_SYNC_MTC_M) {

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
			kernelMidi->send(MIDI_MTC_QUARTER, (midiTCframes & 0x0F)  | 0x00, -1);
			kernelMidi->send(MIDI_MTC_QUARTER, (midiTCframes >> 4)    | 0x10, -1);
			kernelMidi->send(MIDI_MTC_QUARTER, (midiTCseconds & 0x0F) | 0x20, -1);
			kernelMidi->send(MIDI_MTC_QUARTER, (midiTCseconds >> 4)   | 0x30, -1);
		}

		/* minutes low nibble
		 * minutes high nibble
		 * hours low nibble
		 * hours high nibble SMPTE frame rate */

		else {
			kernelMidi->send(MIDI_MTC_QUARTER, (midiTCminutes & 0x0F) | 0x40, -1);
			kernelMidi->send(MIDI_MTC_QUARTER, (midiTCminutes >> 4)   | 0x50, -1);
			kernelMidi->send(MIDI_MTC_QUARTER, (midiTChours & 0x0F)   | 0x60, -1);
			kernelMidi->send(MIDI_MTC_QUARTER, (midiTChours >> 4)     | 0x70, -1);
		}

		midiTCframes++;

		/* check if total timecode frames are greater than timecode fps:
		 * if so, a second has passed */

		if (midiTCframes > conf->midiTCfps) {
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


void gc::sendMIDIrewind()
{
	midiTCframes  = 0;
	midiTCseconds = 0;
	midiTCminutes = 0;
	midiTChours   = 0;

	/* For cueing the slave to a particular start point, Quarter Frame
	 * messages are not used. Instead, an MTC Full Frame message should
	 * be sent. The Full Frame is a SysEx message that encodes the entire
	 * SMPTE time in one message */

	if (conf->midiSync == MIDI_SYNC_MTC_M) {
		kernelMidi->send(MIDI_SYSEX, 0x7F, 0x00);  // send msg on channel 0
		kernelMidi->send(0x01, 0x01, 0x00);        // hours 0
		kernelMidi->send(0x00, 0x00, 0x00);        // mins, secs, frames 0
		kernelMidi->send(MIDI_EOX, -1, -1);        // end of sysex
	}
}


/* -------------------------------------------------------------------------- */


#ifdef __linux__

void gc::recvJackSync()
{
  kernelAudio::JackState jackState = kernelAudio::jackTransportQuery();

  if (jackState.running != jackStatePrev.running) {
    if (jackState.running) {
      if (!isRunning())
        glue_startSeq(false); // not from UI
    }
    else {
      if (isRunning())
        glue_stopSeq(false); // not from UI
    }
  }
  if (jackState.bpm != jackStatePrev.bpm)
    if (jackState.bpm > 1.0f)  // 0 bpm if Jack does not send that info
      glue_setBpm(jackState.bpm);

  if (jackState.frame == 0 && jackState.frame != jackStatePrev.frame)
    glue_rewindSeq(false, false);  // not from UI, don't notify jack (avoid loop)

  jackStatePrev = jackState;
}

#endif


/* -------------------------------------------------------------------------- */


int gc::getCurrentFrame()
{
  return currentFrame;
}


int gc::getTotalFrames()
{
  return totalFrames;
}


int gc::getCurrentBeat()
{
  return currentBeat;
}


int gc::getQuantize()
{
  return quantize;
}


float gc::getBpm()
{
  return bpm;
}


int gc::getBeats()
{
  return beats;
}


int gc::getBars()
{
  return bars;
}


int gc::getQuanto()
{
  return quanto;
}


int gc::getFramesPerBar()
{
  return framesPerBar;
}


int gc::getFramesPerBeat()
{
  return framesPerBeat;
}


int gc::getFramesInSequencer()
{
  return framesInSequencer;
}
