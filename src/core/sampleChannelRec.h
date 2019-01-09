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


#ifndef G_SAMPLE_CHANNEL_REC_H
#define G_SAMPLE_CHANNEL_REC_H




namespace giada {
namespace m 
{
class SampleChannel;

namespace sampleChannelRec
{
void parseEvents(SampleChannel* ch, mixer::FrameEvents fe);

/* recordStart
Records a G_ACTION_KEYPRESS if capable of. Returns true if a start() call can
be performed. */

bool recordStart(SampleChannel* ch, bool doQuantize);

/* recordKill
Records a G_ACTION_KILL if capable of. Returns true if a kill() call can
be performed. */

bool recordKill(SampleChannel* ch);

/* recordStop
Ends overdub mode SINGLE_PRESS channels. */

void recordStop(SampleChannel* ch);

/* setReadActions
If enabled (v == true), Recorder will read actions from channel 'ch'. If 
recsStopOnChanHalt == true and v == false, will also kill the channel. */

void setReadActions(SampleChannel* ch, bool v, bool recsStopOnChanHalt);

void startReadingActions(SampleChannel* ch, bool treatRecsAsLoops, 
	bool recsStopOnChanHalt);
void stopReadingActions(SampleChannel* ch, bool isClockRunning, 
	bool treatRecsAsLoops, bool recsStopOnChanHalt);
}}};


#endif