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


#ifndef G_CLOCK_H
#define G_CLOCK_H


namespace giada {
namespace m {
namespace clock
{
void init(int sampleRate, float midiTCfps);

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
int getCurrentBeat();
int getCurrentFrame();
int getFramesInBar();
int getFramesInBeat();
int getFramesInLoop();
int getFramesInSeq();
int getQuantize();
int getQuanto();

/* incrCurrentFrame
Increases current frame of a single step (+1). */

void incrCurrentFrame();

/* quantoHasPassed
Tells whether a quanto unit has passed yet. */

bool quantoHasPassed();

/* quantoHasPassed
Whether the quantizer value is > 0 and the clock is running. */

bool canQuantize();

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
}}}; // giada::m::clock::


#endif
