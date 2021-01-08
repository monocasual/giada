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


#ifndef G_MIXER_HANDLER_H
#define G_MIXER_HANDLER_H


#include <memory>
#include <string>
#include "types.h"


namespace giada::m 
{
class Wave;
class Channel;
class SampleChannel;
}
namespace giada::m::mh
{
/* init
Initializes mixer. */

void init();

/* close
Closes mixer and frees resources. */

void close();

/* addChannel
Adds a new channel of type 'type' into the channels stack. Returns the new
channel ID. */

void addChannel(ChannelType type, ID columnId);

/* loadChannel
Loads a new Wave inside a Sample Channel. */

int loadChannel(ID channelId, const std::string& fname);

/* addAndLoadChannel (1)
Creates a new channels, fills it with a Wave and then add it to the stack. */

int addAndLoadChannel(ID columnId, const std::string& fname); 

/* addAndLoadChannel (2)
Same as (1), but Wave is already provided. */

void addAndLoadChannel(ID columnId, std::unique_ptr<Wave>&& w); 

/* freeChannel
Unloads existing Wave from a Sample Channel. */

void freeChannel(ID channelId);

/* deleteChannel
Completely removes a channel from the stack. */

void deleteChannel(ID channelId);

void cloneChannel(ID channelId);
void renameChannel(ID channelId, const std::string& name);
void freeAllChannels();

void setInToOut(bool v);

/* updateSoloCount
Updates the number of solo-ed channels in mixer. */

void updateSoloCount();

/* finalizeInputRec
Fills armed Sample Channels with audio data coming from an input recording
session. */

void finalizeInputRec();

/* hasLogicalSamples
True if 1 or more samples are logical (memory only, such as takes) */

bool hasLogicalSamples();

/* hasEditedSamples
True if 1 or more samples was edited via gEditor */

bool hasEditedSamples();

/* has(Input|Action)RecordableChannels
Tells whether Mixer has one or more input or action recordable channels. */

bool hasInputRecordableChannels();
bool hasActionRecordableChannels();

/* hasActions
True if at least one Channel has actions recorded in it. */

bool hasActions();

/* hasAudioData
True if at least one Sample Channel has some audio recorded in it. */

bool hasAudioData();

float getInVol();
float getOutVol();
bool getInToOut();
}  // giada::m::mh::


#endif
