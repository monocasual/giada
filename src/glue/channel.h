/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * glue
 * Intermediate layer GUI <-> CORE.
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#ifndef GLUE_CHANNEL_H
#define GLUE_CHANNEL_H


/* addChannel
 * add an empty new channel to the stack. Returns the new channel. */

class Channel *glue_addChannel(int column, int type);

/* loadChannel
 * fill an existing channel with a wave. */

int glue_loadChannel(class SampleChannel *ch, const char *fname);

/* deleteChannel
 * Remove a channel from Mixer. */

void glue_deleteChannel(class Channel *ch);

/* freeChannel
 * Unload the sample from a sample channel. */

void glue_freeChannel(class Channel *ch);

/* cloneChannel
 * Make an exact copy of Channel *ch. */

int glue_cloneChannel(class Channel *ch);

/* toggleArm
 * Toggle arm status. If gui == true the signal comes from a manual interaction
 * on the GUI, otherwise it's a MIDI/Jack/external signal. */

void glue_toggleArm(class Channel *ch, bool gui=true);

#endif
