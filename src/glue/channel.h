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


#ifndef G_GLUE_CHANNEL_H
#define G_GLUE_CHANNEL_H


#include <string>


class Channel;
class SampleChannel;
class gdSampleEditor;


/* addChannel
 * add an empty new channel to the stack. Returns the new channel. */

Channel *glue_addChannel(int column, int type);

/* loadChannel
 * fill an existing channel with a wave. */

int glue_loadChannel(SampleChannel *ch, const std::string &fname);

/* deleteChannel
 * Remove a channel from Mixer. */

void glue_deleteChannel(Channel *ch);

/* freeChannel
 * Unload the sample from a sample channel. */

void glue_freeChannel(Channel *ch);

/* cloneChannel
 * Make an exact copy of Channel *ch. */

int glue_cloneChannel(Channel *ch);

/* toggle/set*
 * Toggle or set several channel properties. If gui == true the signal comes
 * from a manual interaction on the GUI, otherwise it's a MIDI/Jack/external
 * signal. */

void glue_toggleArm(Channel *ch, bool gui=true);
void glue_toggleInputMonitor(Channel *ch);
void glue_setMute(Channel *ch, bool gui=true);
void glue_setSoloOn (Channel *ch, bool gui=true);
void glue_setSoloOff(Channel *ch, bool gui=true);
void glue_setVolume(Channel *ch, float v, bool gui=true, bool editor=false);

/* TODO move to glue_sampleEditor */
void glue_setPitch(SampleChannel *ch, float val);

/* TODO move to glue_sampleEditor */
void glue_setPanning(SampleChannel *ch, float val);

/* setBeginEndChannel
 * sets start/end points in the sample editor. Recalc=false: don't recalc
 * internal position. check=true: check the points' consistency */

/* TODO move to glue_sampleEditor */
void glue_setBeginEndChannel(SampleChannel *ch, int b, int e);

/* TODO move to glue_sampleEditor */
void glue_setBoost(SampleChannel *ch, float val);

/* start/stopReadingRecs
Handles the 'R' button. If gui == true the signal comes from an user interaction
on the GUI, otherwise it's a MIDI/Jack/external signal. */

void glue_startStopReadingRecs(SampleChannel *ch, bool gui=true);
void glue_startReadingRecs    (SampleChannel *ch, bool gui=true);
void glue_stopReadingRecs     (SampleChannel *ch, bool gui=true);

#endif
