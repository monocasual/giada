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


#ifndef G_GLUE_IO_H
#define G_GLUE_IO_H


class Channel;
class SampleChannel;
class MidiChannel;

namespace giada {
namespace c     {
namespace io 
{
/* keyPress / keyRelease
 * handle the key pressure, either via mouse/keyboard or MIDI. If gui
 * is true it means that the event comes from the main window (mouse,
 * keyb or MIDI), otherwise the event comes from the action recorder. */

void keyPress  (Channel*       ch, bool ctrl, bool shift, int velocity);
void keyPress  (SampleChannel* ch, bool ctrl, bool shift, int velocity);
void keyPress  (MidiChannel*   ch, bool ctrl, bool shift);
void keyRelease(Channel*       ch, bool ctrl, bool shift);
void keyRelease(SampleChannel* ch, bool ctrl, bool shift);

/* start/stopActionRec
Handles the action recording. If gui == true the signal comes from an user
interaction, otherwise it's a MIDI/Jack/external signal. */

void startStopActionRec(bool gui=true);
void startActionRec(bool gui=true);
void stopActionRec(bool gui=true);

/* start/stopInputRec
Handles the input recording (take). If gui == true the signal comes from an
internal interaction on the GUI, otherwise it's a MIDI/Jack/external signal. */

void startStopInputRec(bool gui=true);
int  startInputRec    (bool gui=true);
int  stopInputRec     (bool gui=true);

}}} // giada::c::io::

#endif
