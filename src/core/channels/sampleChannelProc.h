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


#ifndef G_SAMPLE_CHANNEL_PROC_H
#define G_SAMPLE_CHANNEL_PROC_H


#include "core/mixer.h"
#include "core/audioBuffer.h"
#include "core/types.h"


namespace giada {
namespace m 
{
class SampleChannel;

namespace sampleChannelProc
{
/**/
void render(SampleChannel* ch, AudioBuffer& out, const AudioBuffer& in, 
    AudioBuffer& inToOut, bool audible, bool running);

/* parseEvents
Parses events gathered by Mixer::masterPlay(). */

void parseEvents(SampleChannel* ch, mixer::FrameEvents ev);

/* kill
Stops a channel abruptly. */

void kill(SampleChannel* ch, int localFrame);

/* stop
Stops a channel normally (via key or MIDI). */

void stop(SampleChannel* ch);

/* stopInputRec
Prepare a channel for playing when the input recording is done. */

void stopInputRec(SampleChannel* ch, int globalFrame);

/* stopBySeq
Stops a channel when the stop button on main transport is pressed. */

void stopBySeq(SampleChannel* ch, bool chansStopOnSeqHalt);

/* rewind
Rewinds channel when rewind button on main transport is pressed. */

void rewindBySeq(SampleChannel* ch);

/* start
Starts a channel. doQuantize = false (don't quantize) when Mixer is reading 
actions from Recorder. */

void start(SampleChannel* ch, int localFrame, bool doQuantize, int velocity);

void setMute(SampleChannel* ch, bool value);
void setSolo(SampleChannel* ch, bool value);
}}};


#endif
