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


#ifndef G_MIXER_HANDLER_H
#define G_MIXER_HANDLER_H


#include <string>


class Channel;
class SampleChannel;


namespace giada {
namespace m {
namespace mh
{
/* addChannel
Adds a new channel of type 'type' into mixer's stack. */

Channel *addChannel(int type);

/* deleteChannel
Completely removes a channel from the stack. */

int deleteChannel(Channel *ch);

/* getChannelByIndex
Returns channel with given index 'i'. */

Channel *getChannelByIndex(int i);

/* hasLogicalSamples
True if 1 or more samples are logical (memory only, such as takes) */

bool hasLogicalSamples();

/* hasEditedSamples
True if 1 or more samples was edited via gEditor */

bool hasEditedSamples();

/* stopSequencer
 * stop the sequencer, with special case if samplesStopOnSeqHalt is
 * true. */

void stopSequencer();

void rewindSequencer();

/* uniqueSolo
 * true if ch is the only solo'd channel in mixer. */

bool uniqueSolo(Channel *ch);

/* loadPatch
 * load a path or a project (if isProject) into Mixer. If isProject, path
 * must contain the address of the project folder. */

void readPatch();

/* startInputRec - record from line in
 * creates a new empty wave in the first available channels and returns
 * the chan number chosen, otherwise -1 if there are no more empty
 * channels available. */

bool startInputRec();

void stopInputRec();

/* uniqueSamplename
 * return true if samplename 'n' is unique. Requires SampleChannel *ch
 * in order to skip check against itself. */

bool uniqueSampleName(SampleChannel *ch, const std::string &s);

/* hasArmedSampleChannels
Tells whether Mixer has one or more sample channels armed for input
recording. */

bool hasArmedSampleChannels();
}}}  // giada::m::mh::


#endif
