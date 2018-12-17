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


namespace giada {
namespace m
{
class Plugin;
class Channel;
}
namespace c {
namespace io 
{
/* keyPress / keyRelease
Handle the key pressure, either via mouse/keyboard or MIDI. If gui is true the 
event comes from the main window (mouse, keyboard or MIDI), otherwise the event 
comes from the action recorder. */

void keyPress  (m::Channel* ch, bool ctrl, bool shift, int velocity);
void keyRelease(m::Channel* ch, bool ctrl, bool shift);

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
