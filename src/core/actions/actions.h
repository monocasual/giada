/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_ACTIONS_H
#define G_ACTIONS_H

#include "action.h"
#include "core/idManager.h"
#include "core/midiEvent.h"
#include "core/patch.h"
#include "core/types.h"
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace giada::m::model
{
class Model;
}

namespace giada::m
{
class Actions
{
public:
	using Map = std::map<Frame, std::vector<Action>>;

	Actions(model::Model& model);

	/* forEachAction
    Applies a read-only callback on each action recorded. NEVER do anything
    inside the callback that might alter the ActionMap. */

	void forEachAction(std::function<void(const Action&)> f) const;

	/* getActionsOnChannel
    Returns a vector of actions belonging to channel 'ch'. */

	std::vector<Action> getActionsOnChannel(ID channelId) const;

	/* getClosestAction
    Given a frame 'f' returns the closest action. */

	Action getClosestAction(ID channelId, Frame f, int type) const;

	/* getActionsOnFrame
    Returns a pointer to a vector of actions recorded on frame 'f', or nullptr 
    if the frame has no actions. */

	const std::vector<Action>* getActionsOnFrame(Frame f) const;

	/* hasActions
    Checks if the channel has at least one action recorded. */

	bool hasActions(ID channelId, int type = 0) const;

	/* makeAction
    Makes a new action given some data. */
	//TODO - move to actionManager

	Action makeAction(ID id, ID channelId, Frame frame, MidiEvent e);
	Action makeAction(const Patch::Action& a);

	/* reset
	Brings everything back to the initial state. */

	void reset();

	/* clearAll
    Deletes all recorded actions. */

	void clearAll();

	/* clearChannel
    Clears all actions from a channel. */

	void clearChannel(ID channelId);

	/* clearActions
    Clears the actions by type from a channel. */

	void clearActions(ID channelId, int type);

	/* deleteAction (1)
    Deletes a specific action. */

	void deleteAction(ID id);

	/* deleteAction (2)
    Deletes a specific pair of actions. Useful for composite stuff (i.e. MIDI). */

	void deleteAction(ID currId, ID nextId);

	/* updateKeyFrames
    Update all the key frames in the internal map of actions, according to a 
    lambda function 'f'. */

	void updateKeyFrames(std::function<Frame(Frame old)> f);

	/* updateEvent
    Changes the event in action 'a'. */

	void updateEvent(ID id, MidiEvent e);

	/* updateSiblings
    Changes previous and next actions in action with id 'id'. Mostly used for
    chained actions such as envelopes. */

	void updateSiblings(ID id, ID prevId, ID nextId);

	/* rec (1)
    Records an action and returns it. Used by the Action Editor. */

	Action rec(ID channelId, Frame frame, MidiEvent e);

	/* rec (2)
    Transfer a vector of actions into the current ActionMap. This is called by 
    recordHandler when a live session is over and consolidation is required. */

	void rec(std::vector<Action>& actions);

	/* rec (3)
    Records two actions on channel 'channel'. Useful when recording composite 
    actions in the Action Editor. */

	void rec(ID channelId, Frame f1, Frame f2, MidiEvent e1, MidiEvent e2);

	/* getNewActionId
    Returns a new action ID, internally generated. */
	//TODO - move to actionManager

	ID getNewActionId();

private:
	bool exists(ID channelId, Frame frame, const MidiEvent& event, const Map& target) const;
	bool exists(ID channelId, Frame frame, const MidiEvent& event) const;

	Action* findAction(Map& src, ID id);

	/* updateMapPointers
    Updates all prev/next actions pointers into the action map. This is required
    after an action has been recorded, since pushing back new actions in a Action 
    vector makes it reallocating the existing ones. */

	void updateMapPointers(Map& src);

	/* optimize
    Removes frames without actions. */

	void optimize(Map& map);

	void removeIf(std::function<bool(const Action&)> f);

	model::Model& m_model;

	//TODO - move to actionManager
	IdManager m_actionId;
};
} // namespace giada::m

#endif
