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

#ifndef G_CLOCK_H
#define G_CLOCK_H

#include "types.h"

namespace giada::m::clock
{
void init();

/* recomputeFrames
Updates bpm, frames, beats and so on. */

void recomputeFrames();

float       getBpm();
int         getBeats();
int         getBars();
int         getCurrentBeat();
int         getCurrentFrame();
float       getCurrentSecond();
int         getFramesInBar();
int         getFramesInBeat();
int         getFramesInLoop();
int         getFramesInSeq();
int         getQuantizerValue();
int         getQuantizerStep();
ClockStatus getStatus();

/* getMaxFramesInLoop
Returns how many frames the current loop length might contain at the slowest
speed possible (G_MIN_BPM). Call this whenever you change the number or beats. */

Frame getMaxFramesInLoop();

/* advance
Increases current frame by a specific amount. */

void advance(Frame amount);

/* quantoHasPassed
Tells whether a quantizer unit has passed yet. */

bool quantoHasPassed();

/* canQuantize
Tells whether the quantizer value is > 0 and the clock is running. */

bool canQuantize();

/* quantize
Quantizes the global frame 'f'.  */

Frame quantize(Frame f);

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

/* calcBpmFromRec
Given the amount of recorded frames, returns the speed of the current 
performance. Used while input recording in FREE mode. */

float calcBpmFromRec(Frame recordedFrames);
} // namespace giada::m::clock

#endif
