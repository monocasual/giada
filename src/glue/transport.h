/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#ifndef G_GLUE_TRANSPORT_H
#define G_GLUE_TRANSPORT_H


/* start, stop, rewind sequencer
If gui == true the signal comes from an user interaction on the GUI,
otherwise it's a MIDI/Jack/external signal. */

void glue_startStopSeq(bool gui=true);
void glue_startSeq(bool gui=true);
void glue_stopSeq(bool gui=true);
void glue_rewindSeq(bool gui=true, bool notifyJack=true);
void glue_startStopMetronome(bool gui=true);


#endif
