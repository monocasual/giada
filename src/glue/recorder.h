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


#ifndef G_GLUE_RECORDER_H
#define G_GLUE_RECORDER_H


#include <vector>
#include "../core/recorder.h"


class geChannel;


namespace giada {
namespace c     {
namespace recorder 
{
void clearAllActions(geChannel* gch);
void clearMuteActions(geChannel* gch);
void clearVolumeActions(geChannel* gch);
void clearStartStopActions(geChannel* gch);

/* recordMidiAction
Records a new MIDI action at frame_a. If frame_b == 0, uses the default action
size. This function is designed for the Piano Roll (not for live recording). */

void recordMidiAction(int chan, int note, int frame_a, int frame_b=0);

/* getMidiActions
Returns a list of Composite actions, ready to be displayed in a MIDI note
editor as pairs of NoteOn+NoteOff. */

std::vector<giada::m::recorder::Composite> getMidiActions(int channel, 
	int frameLimit);

}}} // giada::c::recorder::

#endif
