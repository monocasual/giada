/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * glue
 * Intermediate layer GUI <-> CORE.
 *
 * How to know if you need another glue_ function? Ask yourself if the
 * new action will ever be called via MIDI or keyboard/mouse. If yes,
 * put it here.
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


#ifndef GLUE_IO_H
#define GLUE_IO_H


/* keyPress / keyRelease
 * handle the key pressure, either via mouse/keyboard or MIDI. If gui
 * is true it means that the event comes from the main window (mouse,
 * keyb or MIDI), otherwise the event comes from the action recorder. */

void glue_keyPress  (class Channel       *ch, bool ctrl=0, bool shift=0);
void glue_keyPress  (class SampleChannel *ch, bool ctrl=0, bool shift=0);
void glue_keyPress  (class MidiChannel   *ch, bool ctrl=0, bool shift=0);
void glue_keyRelease(class Channel       *ch, bool ctrl=0, bool shift=0);
void glue_keyRelease(class SampleChannel *ch, bool ctrl=0, bool shift=0);

/* start/stopActionRec
Handles the action recording. If gui == true the signal comes from an user
interaction, otherwise it's a MIDI/Jack/external signal. */

void glue_startStopActionRec(bool gui=true);
void glue_startActionRec(bool gui=true);
void glue_stopActionRec(bool gui=true);

/* start/stopInputRec
 * Handle the input recording (take). If gui == true the signal comes
 * from an internal interaction on the GUI, otherwise it's a
 * MIDI/Jack/external signal. Alert displays the popup message
 * if there are no available channels. */

void glue_startStopInputRec(bool gui=true, bool alert=true);
int  glue_startInputRec    (bool gui=true);
int  glue_stopInputRec     (bool gui=true);


#endif
