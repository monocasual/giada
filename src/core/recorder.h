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


#ifndef G_RECORDER_H
#define G_RECORDER_H


#include <map>
#include <vector>
#include <functional>
#include "types.h"
#include "midiEvent.h"


namespace giada {
namespace m 
{
class Action;

namespace recorder
{
using ActionMap = std::map<Frame, std::vector<const Action*>>;

void debug();
/* init
Initializes the recorder: everything starts from here. */

void init(pthread_mutex_t* mixerMutex);

/* clearAll
Deletes all recorded actions. */

void clearAll();

/* clearChannel
Clears all actions from a channel. */

void clearChannel(int channel);

/* clearActions
Clears the actions by type from a channel. */

void clearActions(int channel, int type);

/* deleteAction
Deletes a specific action. */

void deleteAction(const Action* a);

/* updateKeyFrames
Update all the key frames in the internal map of actions, according to a lambda 
function 'f'. */

void updateKeyFrames(std::function<Frame(Frame old)> f);

/* updateActionMap
Replaces the current map of actions with a new one. Warning: 'am' will be moved
as a replacement (no copy). */

void updateActionMap(ActionMap&& am);

/* updateEvent
Changes the event in action 'a'. */

void updateEvent(const Action* a, MidiEvent e);

/* updateSiblings
Changes previous and next actions in action 'a'. Mostly used for chained actions
such as envelopes. */

void updateSiblings(const Action* a, const Action* prev, const Action* next);

void updateActionId(int id);

/* hasActions
Checks if the channel has at least one action recorded. */

bool hasActions(int channel, int type=0);

/* isActive
Is recorder recording something? */

bool isActive();

void enable();
void disable();

const Action* makeAction(int id, int channel, Frame frame, MidiEvent e);

/* rec (1)
Records an action and returns it. */

const Action* rec(int channel, Frame frame, MidiEvent e);

/* rec (2)
Transfer a vector of actions into the current ActionMap. This is called by 
recordHandler when a live session is over and consolidation is required. */

void rec(const std::vector<const Action*>& actions);

/* forEachAction
Applies a read-only callback on each action recorded. NEVER do anything inside 
the callback that might alter the ActionMap. */

void forEachAction(std::function<void(const Action*)> f);

/* getActionsOnFrame
Returns a vector of actions recorded on frame 'f'. */

std::vector<const Action*> getActionsOnFrame(Frame f);

/* getActionsOnChannel
Returns a vector of actions belonging to channel 'ch'. */

std::vector<const Action*> getActionsOnChannel(int ch);

/* getClosestAction
Given a frame 'f' returns the closest action. */

const Action* getClosestAction(int channel, Frame f, int type);


int getLatestActionId();

/* getActionMap
Returns a copy of the internal action map. Used only by recorderHandler. */

ActionMap getActionMap();

}}}; // giada::m::recorder::


#endif
