/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_MODEL_ACTIONS_H
#define G_MODEL_ACTIONS_H

#include "src/const.h"
#include "src/core/actions/action.h"
#include "src/core/midiEvent.h"
#include "src/core/types.h"
#include <functional>
#include <map>
#include <memory>
#include <span>
#include <vector>

namespace giada::m::model
{
class Actions
{
public:
	/* getActionsOnChannel
	Returns a vector of actions belonging to channel 'ch'. */

	std::vector<Action> getActionsOnChannel(ID channelId, Scene) const;

	/* getActionsInTickRange
	Returns a span of all actions included in the required range. */

	const std::span<const Action> getActionsInTickRange(TickRange) const;

	/* hasActions (1)
	Checks if the channel has at least one action recorded. */

	bool hasActions(ID channelId, int type = 0) const;

	/* hasActions (2)
	True if there is at least one action in the given scene. */

	bool hasActions(Scene scene) const;

	/* getAll
	Returns a reference to the internal map. */

	const std::vector<Action>& getAll() const;

	/* findAction
	Finds action given ID. Returns nullptr if not found. */

	const Action* findAction(ID) const;
	Action*       findAction(ID id);

#if G_DEBUG_MODE
	void debug() const;
#endif

	/* set
	Sets a new whole vector of actions. Use this when deserializing stuff. */

	void set(std::vector<Action>&&);

	/* clearAll
	Deletes all recorded actions. */

	void clearAll();

	/* clearChannel
	Clears all actions from a channel in a given scene. */

	void clearChannel(ID channelId, Scene);

	/* clearActions (1)
	Clears the actions by type from a channel. */

	void clearActions(ID channelId, int type);

	/* clearActions (2)
	Clears all actions from a given scene. */

	void clearActions(Scene);

	/* deleteAction (1)
	Deletes a specific action. */

	void deleteAction(ID id);

	/* deleteAction (2)
	Deletes a specific pair of actions. Useful for composite stuff (i.e. MIDI). */

	void deleteAction(ID currId, ID nextId);

	/* updateEvent
	Changes the event in action 'a'. */

	void updateEvent(ID id, MidiEvent e);

	/* updateSiblings
	Changes previous and next actions in action with id 'id'. Mostly used for
	chained actions such as envelopes. */

	void updateSiblings(ID id, ID prevId, ID nextId);

	/* rec (1)
	Records an action and returns it. Used by the Action Editor. */

	Action rec_DEPR_(ID channelId, Scene, Frame frame, MidiEvent e);
	Action rec(ID channelId, Scene, Tick, MidiEvent e);

	/* rec (2)
	Transfer a vector of actions into the current ActionMap. This is called by
	recordHandler when a live session is over and consolidation is required. */

	void rec(std::vector<Action>& actions, Scene);

	/* rec (3)
	Records two actions on channel 'channel'. Useful when recording composite
	actions in the Action Editor. */

	void rec_DEPR_(ID channelId, Scene, Frame f1, Frame f2, MidiEvent e1, MidiEvent e2);
	void rec(ID channelId, Scene, TickRange, MidiEvent e1, MidiEvent e2);

private:
	bool exists(ID channelId, Scene, Frame frame, const MidiEvent&, const std::vector<Action>& target) const;
	bool exists(ID channelId, Scene, Frame frame, const MidiEvent&) const;
	bool exists(ID channelId, Scene, Tick, const MidiEvent&, const std::vector<Action>& target) const;
	bool exists(ID channelId, Scene, Tick, const MidiEvent&) const;

	/* sort
	Sorts the internal vector of Actions, frame-wise, ascending. Mandatory each
	time you add a new Action. */

	void sort();

	/* m_actions
	Stored actions. Must always be sorted frame-wise, ascending, to allow
	the fetch alogrithm to work properly. Sorting is needed any time you add
	a new action to the vector. */

	std::vector<Action> m_actions;
};
} // namespace giada::m::model

#endif
