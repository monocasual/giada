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


#ifndef G_GLUE_CHANNEL_H
#define G_GLUE_CHANNEL_H


#include <string>
#include <vector>
#include "core/types.h"


namespace giada {
namespace m
{
class Channel;
}
namespace c {
namespace channel 
{
/* addChannel
Adds an empty new channel to the stack. */

void addChannel(ID columnId, ChannelType type);

/* loadChannel
Fills an existing channel with a wave. */

int loadChannel(ID channelId, const std::string& fname);

/* addAndLoadChannel
Adds a new Sample Channel and fills it with a wave right away. */

void addAndLoadChannel(ID columnId, const std::string& fpath); 

/* addAndLoadChannels
As above, with multiple audio file paths in input. */

void addAndLoadChannels(ID columnId, const std::vector<std::string>& fpaths);

/* deleteChannel
Removes a channel from Mixer. */

void deleteChannel(ID channelId);

/* freeChannel
Unloads the sample from a sample channel. */

void freeChannel(ID channelId);

/* cloneChannel
Makes an exact copy of Channel *ch. */

void cloneChannel(ID channelId);

/* set*
Sets several channel properties. */

void setArm(ID channelId, bool value);
void toggleArm(ID channelId);
void setInputMonitor(ID channelId, bool value);
void setMute(ID channelId, bool value);
void toggleMute(ID channelId);
void setSolo(ID channelId, bool value);
void toggleSolo(ID channelId);
void setVolume(ID channelId, float v, bool gui=true, bool editor=false);
void setName(ID channelId, const std::string& name);
void setPitch(ID channelId, float val, bool gui=true);
void setPan(ID channelId, float val, bool gui=true);
void setSampleMode(ID channelId, ChannelMode m);

void start(ID channelId, int velocity, bool record);
void kill(ID channelId, bool record);
void stop(ID channelId);

/* toggleReadingRecs
Handles the 'R' button. If gui == true the signal comes from an user interaction
on the GUI, otherwise it's a MIDI/Jack/external signal. */

void toggleReadingActions(ID channelId);
void startReadingActions(ID channelId);
void stopReadingActions(ID channelId);

}}}; // giada::c::channel::

#endif
