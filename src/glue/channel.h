/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * glue
 * Intermediate layer GUI <-> CORE.
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
class gdEditor;


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
void glue_setChanVol(Channel *ch, float v, bool gui=true);
void glue_setMute(Channel *ch, bool gui=true);
void glue_setSoloOn (Channel *ch, bool gui=true);
void glue_setSoloOff(Channel *ch, bool gui=true);

void glue_setPitch(gdEditor *win, SampleChannel *ch, float val,
  bool numeric);

void glue_setPanning(gdEditor *win, SampleChannel *ch, float val);

/* setBeginEndChannel
 * sets start/end points in the sample editor. Recalc=false: don't recalc
 * internal position. check=true: check the points' consistency */

void glue_setBeginEndChannel(gdEditor *win, SampleChannel *ch,
  int b, int e, bool recalc=false, bool check=true);

void glue_setBoost(gdEditor *win, SampleChannel *ch, float val,
  bool numeric);

/* setVolEditor
 * handles the volume inside the SAMPLE EDITOR (not the main gui). The
 * numeric flag tells if we want to handle the dial or the numeric input
 * field. */

void glue_setVolEditor(gdEditor *win, SampleChannel *ch, float val,
  bool numeric);

/* start/stopReadingRecs
Handles the 'R' button. If gui == true the signal comes from an user interaction
on the GUI, otherwise it's a MIDI/Jack/external signal. */

void glue_startStopReadingRecs(SampleChannel *ch, bool gui=true);
void glue_startReadingRecs    (SampleChannel *ch, bool gui=true);
void glue_stopReadingRecs     (SampleChannel *ch, bool gui=true);

#endif
