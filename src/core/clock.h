/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#ifndef G_CLOCK_H
#define G_CLOCK_H


#include "types.h"


namespace giada {
namespace m {
namespace clock
{
void init(int sampleRate, float midiTCfps);

/* recomputeFrames
Updates bpm, frames, beats and so on. */

void recomputeFrames();

/* sendMIDIsync
Generates MIDI sync output data. */

void sendMIDIsync();

/* sendMIDIrewind
Rewinds timecode to beat 0 and also send a MTC full frame to cue the slave. */

void sendMIDIrewind();

#if defined(__linux__) || defined(__FreeBSD__)
void recvJackSync();
#endif

float getBpm();
int getBeats();
int getBars();
int getCurrentBeat();
int getCurrentFrame();
int getFramesInBar();
int getFramesInBeat();
int getFramesInLoop();
int getFramesInSeq();
int getQuantize();
int getQuanto();
ClockStatus getStatus();

/* incrCurrentFrame
Increases current frame of a single step (+1). */

void incrCurrentFrame();

/* quantoHasPassed
Tells whether a quanto unit has passed yet. */

bool quantoHasPassed();

/* quantoHasPassed
Whether the quantizer value is > 0 and the clock is running. */

bool canQuantize();

void setBpm(float b);
void setBeats(int beats, int bars);
void setQuantize(int q);

/* isRunning
When clock is actually moving forward, i.e. ClockStatus == RUNNING. */

bool isRunning();

/* isActive
Clock is enabled, but might be in wait mode, i.e. ClockStatus == RUNNING or
ClockStatus == WAITING. */

bool isActive();

bool isOnBeat();
bool isOnBar();
bool isOnFirstBeat();

void rewind();
void setStatus(ClockStatus s);
}}}; // giada::m::clock::


#endif
