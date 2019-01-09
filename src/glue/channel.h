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


#ifndef G_GLUE_CHANNEL_H
#define G_GLUE_CHANNEL_H


#include <string>
#include "../core/types.h"


class gdSampleEditor;


namespace giada {
namespace m
{
class Channel;
class SampleChannel;
}
namespace c {
namespace channel 
{
/* addChannel
Adds an empty new channel to the stack. Returns the new channel. */

m::Channel* addChannel(int column, ChannelType type, int size);

/* loadChannel
Fills an existing channel with a wave. */

int loadChannel(m::SampleChannel* ch, const std::string& fname);

/* deleteChannel
Removes a channel from Mixer. */

void deleteChannel(m::Channel* ch);

/* freeChannel
Unloads the sample from a sample channel. */

void freeChannel(m::Channel* ch);

/* cloneChannel
Makes an exact copy of Channel *ch. */

int cloneChannel(m::Channel* ch);

/* toggle/set*
Toggles or set several channel properties. If gui == true the signal comes from 
a manual interaction on the GUI, otherwise it's a MIDI/Jack/external signal. */

void toggleArm(m::Channel* ch, bool gui=true);
void toggleInputMonitor(m::Channel* ch);
void kill(m::Channel* ch);
void toggleMute(m::Channel* ch, bool gui=true);
void toggleSolo(m::Channel* ch, bool gui=true);
void setVolume(m::Channel* ch, float v, bool gui=true, bool editor=false);
void setName(m::Channel* ch, const std::string& name);
void setPitch(m::SampleChannel* ch, float val);
void setPanning(m::SampleChannel* ch, float val);
void setBoost(m::SampleChannel* ch, float val);

/* toggleReadingRecs
Handles the 'R' button. If gui == true the signal comes from an user interaction
on the GUI, otherwise it's a MIDI/Jack/external signal. */

void toggleReadingActions(m::Channel* ch, bool gui=true);
void startReadingActions(m::Channel* ch, bool gui=true);
void stopReadingActions(m::Channel* ch, bool gui=true);

}}}; // giada::c::channel::

#endif
