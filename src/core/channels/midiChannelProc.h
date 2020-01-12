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


#ifndef G_MIDI_CHANNEL_PROC_H
#define G_MIDI_CHANNEL_PROC_H


#include "core/mixer.h"
#include "core/audioBuffer.h"


namespace giada {
namespace m 
{
class MidiChannel;
namespace midiChannelProc
{
/* parseEvents
Parses events gathered by Mixer::masterPlay(). */

void parseEvents(MidiChannel* ch, mixer::FrameEvents ev);

/**/
void process(MidiChannel* ch, AudioBuffer& out, const AudioBuffer& in, bool audible);

/* kill
Stops a channel abruptly. */

void kill(MidiChannel* ch, int localFrame);

/* start
Starts a channel. */

void start(MidiChannel* ch);

/* stopBySeq
Stops a channel when the stop button on main transport is pressed. */

void stopBySeq(MidiChannel* ch);

/* rewind
Rewinds channel when rewind button on main transport is pressed. */

void rewindBySeq(MidiChannel* ch);

/* mute|unmute
Mutes/unmutes a channel. */

void setMute(MidiChannel* ch, bool v);
void setSolo(MidiChannel* ch, bool v);
}}};


#endif
