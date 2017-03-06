/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * clock
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
#include "../glue/main.h"
#include "conf.h"
#include "const.h"
#include "kernelAudio.h"
#include "kernelMidi.h"
#include "clock.h"


Clock::Clock(KernelAudio *kernelAudio, KernelMidi *kernelMidi, Conf *conf)
  : kernelAudio      (kernelAudio),
    kernelMidi       (kernelMidi),
    conf             (conf),
    running          (false),
    bpm              (G_DEFAULT_BPM),
    bars             (G_DEFAULT_BARS),
    beats            (G_DEFAULT_BEATS),
    quantize         (G_DEFAULT_QUANTIZE),
    quanto           (1),
    framesPerBar     (0),
    framesPerBeat    (0),
    framesInSequencer(0),
    totalFrames      (0),
    currentFrame     (0),
    currentBeat      (0),
    midiTCstep       (0),
  	midiTCrate       ((conf->samplerate / conf->midiTCfps) * 2),  // stereo vals
  	midiTCframes     (0),
  	midiTCseconds    (0),
  	midiTCminutes    (0),
  	midiTChours      (0)
{
  assert(kernelAudio != nullptr);
  assert(kernelMidi != nullptr);
  assert(conf != nullptr);
}


/* -------------------------------------------------------------------------- */


bool Clock::isRunning()
{
  return running;
}


bool Clock::quantoHasPassed()
{
  return currentFrame % (quanto) == 0;
}


bool Clock::isOnBar()
{
  /* A bar cannot occur at frame 0. That's the first beat. */
  return currentFrame % framesPerBar == 0 && currentFrame != 0;
}


bool Clock::isOnBeat()
{
  /* Skip frame 0: it is intended as 'first beat'. */
  /* TODO - this is wrong! */
  return currentFrame % framesPerBeat == 0 && currentFrame > 0;
}


bool Clock::isOnFirstBeat()
{
  return currentFrame == 0;
}


void Clock::start()
{
  running = true;
	if (conf->midiSync == MIDI_SYNC_CLOCK_M) {
		kernelMidi->send(MIDI_START, -1, -1);
		kernelMidi->send(MIDI_POSITION_PTR, 0, 0);
	}
}


void Clock::stop()
{
  running = false;
  if (conf->midiSync == MIDI_SYNC_CLOCK_M)
  	kernelMidi->send(MIDI_STOP, -1, -1);
}


void Clock::setBpm(float b)
{
  if (b < G_MIN_BPM)
    b = G_MIN_BPM;
  bpm = b;
	updateFrameBars();
}


void Clock::setBars(int b)
{
  bars = b;
}


void Clock::setBeats(int b)
{
  beats = b;
}


void Clock::setQuantize(int q)
{
  quantize = q;
  updateQuanto();
}


/* -------------------------------------------------------------------------- */


void Clock::incrCurrentFrame()
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


void Clock::rewind()
{
  currentFrame = 0;
  currentBeat = 0;
  sendMIDIrewind();
}


/* -------------------------------------------------------------------------- */


void Clock::updateFrameBars()
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


void Clock::sendMIDIsync()
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


void Clock::sendMIDIrewind()
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

void Clock::recvJackSync()
{
  KernelAudio::JackState jackState = kernelAudio->jackTransportQuery();

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
    glue_setBpm(jackState.bpm);
  if (jackState.frame == 0)
    glue_rewindSeq(false);     // false: not from UI interaction

  jackStatePrev = jackState;
}

#endif


/* -------------------------------------------------------------------------- */


int Clock::getCurrentFrame()
{
  return currentFrame;
}


int Clock::getTotalFrames()
{
  return totalFrames;
}


int Clock::getCurrentBeat()
{
  return currentBeat;
}


int Clock::getQuantize()
{
  return quantize;
}


float Clock::getBpm()
{
  return bpm;
}


int Clock::getBeats()
{
  return beats;
}


int Clock::getBars()
{
  return bars;
}


int Clock::getQuanto()
{
  return quanto;
}


int Clock::getFramesPerBar()
{
  return framesPerBar;
}


int Clock::getFramesPerBeat()
{
  return framesPerBeat;
}


int Clock::getFramesInSequencer()
{
  return framesInSequencer;
}


/* -------------------------------------------------------------------------- */


void Clock::updateQuanto()
{
  if (quantize != 0)
		quanto = framesPerBeat / quantize;
	if (quanto % 2 != 0)
		quanto++;
}
