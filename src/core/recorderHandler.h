/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <unordered_set>
#include "midiEvent.h"


namespace giada {
namespace m 
{
namespace patch
{
struct Action;
}
struct Action;
namespace recorderHandler
{
void init();

bool isBoundaryEnvelopeAction(const Action& a);

/* updateBpm
Changes actions position by calculating the new bpm value. */

void updateBpm(float oldval, float newval, int oldquanto);

/* updateSamplerate
Changes actions position by taking in account the new samplerate. If 
f_system == f_patch nothing will change, otherwise the conversion is 
mandatory. */

void updateSamplerate(int systemRate, int patchRate);

/* cloneActions
Clones actions in channel 'channelId', giving them a new channel ID. Returns
whether any action has been cloned. */

bool cloneActions(ID channelId, ID newChannelId);

/* liveRec
Records a user-generated action. NOTE_ON or NOTE_OFF only for now. */

void liveRec(ID channelId, MidiEvent e);

/* consolidate
Records all live actions. Returns a set of channels IDs that have been 
recorded. */

std::unordered_set<ID> consolidate();

/* clearAllActions
Deletes all recorded actions. */

void clearAllActions();

recorder::ActionMap makeActionsFromPatch(const std::vector<patch::Action>& pactions);

}}}; // giada::m::recorderHandler::


#endif
