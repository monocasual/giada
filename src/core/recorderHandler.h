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


#ifndef G_RECORDER_HANDLER_H
#define G_RECORDER_HANDLER_H


#include "midiEvent.h"
#include "patch.h"


namespace giada {
namespace m 
{
struct Action;

namespace recorderHandler
{
bool isBoundaryEnvelopeAction(const Action* a);

/* updateBpm
Changes actions position by calculating the new bpm value. */

void updateBpm(float oldval, float newval, int oldquanto);

/* updateSamplerate
Changes actions position by taking in account the new samplerate. If 
f_system == f_patch nothing will change, otherwise the conversion is 
mandatory. */

void updateSamplerate(int systemRate, int patchRate);

/* cloneActions
Clones actions in channel 'chanIndex', giving them a new channel index. Returns
whether any action has been cloned. */

bool cloneActions(int chanIndex, int newChanIndex);

/* liveRec
Records a user-generated action. NOTE_ON or NOTE_OFF only for now. */

void liveRec(int channel, MidiEvent e);

void consolidate();

void writePatch(int chanIndex, std::vector<patch::action_t>& pactions);
void readPatch(const std::vector<patch::action_t>& pactions);

}}}; // giada::m::recorderHandler::


#endif
