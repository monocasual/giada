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


#ifndef __CLOCK_H__
#define __CLOCK_H__


#include "kernelAudio.h"


class Conf;
class KernelMidi;


class Clock
{
public:

  Clock(KernelAudio *ka, KernelMidi *km, Conf *conf);

  /* sendMIDIsync
  Generates MIDI sync output data. */

  void sendMIDIsync();

  /* sendMIDIrewind
	Rewinds timecode to beat 0 and also send a MTC full frame to cue the slave. */

	void sendMIDIrewind();

#ifdef __linux__
  void recvJackSync();
#endif

  float getBpm();
  int getBeats();
  int getBars();
  int getCurrentFrame();
  int getCurrentBeat();
  int getFramesPerBar();
  int getFramesPerBeat();
  int getTotalFrames();
  int getFramesInSequencer();
  int getQuantize();
  int getQuanto();

  /* incrCurrentFrame
  Increases current frame of a stereo step (+2). */

  void incrCurrentFrame();

  /* quantoHasPassed
  Tells whether a quanto unit has passed yet. */

  bool quantoHasPassed();

  /* updateFrameBars
	Updates bpm, frames, beats and so on. */

	void updateFrameBars();

  void setBpm(float b);
  void setBars(int b);
  void setBeats(int b);
  void setQuantize(int q);

  bool isRunning();
  bool isOnBeat();
  bool isOnBar();
  bool isOnFirstBeat();

  void rewind();
  void start();
  void stop();

private:

  KernelAudio *kernelAudio;
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

  int midiTCstep;      // part of MTC to send (0 to 7)
	int midiTCrate;      // send MTC data every midiTCrate frames
	int midiTCframes;
	int midiTCseconds;
	int midiTCminutes;
	int midiTChours;

#ifdef __linux__
  bool   jackRunningPrev;
  double jackBpmPrev;
  KernelAudio::JackState jackStatePrev;
#endif

  void updateQuanto();
};


#endif
