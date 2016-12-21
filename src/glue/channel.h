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


#include <string>


/* addChannel
 * add an empty new channel to the stack. Returns the new channel. */

class Channel *glue_addChannel(int column, int type);

/* loadChannel
 * fill an existing channel with a wave. */

int glue_loadChannel(class SampleChannel *ch, const std::string &fname);

/* deleteChannel
 * Remove a channel from Mixer. */

void glue_deleteChannel(class Channel *ch);

/* freeChannel
 * Unload the sample from a sample channel. */

void glue_freeChannel(class Channel *ch);

/* cloneChannel
 * Make an exact copy of Channel *ch. */

int glue_cloneChannel(class Channel *ch);

/* toggle/set*
 * Toggle or set several channel properties. If gui == true the signal comes
 * from a manual interaction on the GUI, otherwise it's a MIDI/Jack/external
 * signal. */

void glue_toggleArm(class Channel *ch, bool gui=true);
void glue_setChanVol(class Channel *ch, float v, bool gui=true);
void glue_setMute(class Channel *ch, bool gui=true);
void glue_setSoloOn (class Channel *ch, bool gui=true);
void glue_setSoloOff(class Channel *ch, bool gui=true);

void glue_setPitch(class gdEditor *win, class SampleChannel *ch, float val,
  bool numeric);

void glue_setPanning(class gdEditor *win, class SampleChannel *ch, float val);

/* setBeginEndChannel
 * sets start/end points in the sample editor. Recalc=false: don't recalc
 * internal position. check=true: check the points' consistency */

void glue_setBeginEndChannel(class gdEditor *win, class SampleChannel *ch,
  int b, int e, bool recalc=false, bool check=true);

void glue_setBoost(class gdEditor *win, class SampleChannel *ch, float val,
  bool numeric);

/* setVolEditor
 * handles the volume inside the SAMPLE EDITOR (not the main gui). The
 * numeric flag tells if we want to handle the dial or the numeric input
 * field. */

void glue_setVolEditor(class gdEditor *win, class SampleChannel *ch, float val,
  bool numeric);

#ifdef WITH_VST

class Plugin *glue_addPlugin(class Channel *ch, int index, int stackType);
void glue_freePlugin(class Channel *ch, int index, int stackType);

#endif

#endif
