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

#ifndef G_ACTION_MANAGER_H
#define G_ACTION_MANAGER_H

#include "src/core/midiEvent.h"
#include "src/core/model/model.h"
#include "src/core/types.h"
#include <cstddef>
#include <unordered_set>

namespace giada::patch
{
struct Action;
}

namespace giada::m
{
struct Action;
class ActionManager
{
public:
	ActionManager(model::Model&);

	/* reset
	Brings everything back to the initial state. */

	void reset();

	/* cloneActions
	Clones actions in channel 'channelId', giving them a new channel ID. Returns
	whether any action has been cloned. */

	void cloneActions(ID channelId, Scene, ID newChannelId);

	/* copyActionsToScene
	Copies actions of a given channel from a scene to another. */

	void copyActionsToScene(ID channelId, Scene src, Scene dst);

	/* copyAllActionsToScene
	Copies all actions in all channels from a scene to another. */

	void copyAllActionsToScene(Scene src, Scene dst);

	/* liveRec
	Records a user-generated action. NOTE_ON or NOTE_OFF only for now. */

	void liveRec(ID channelId, Scene, MidiEvent e, Frame global);

	/* record*Action */

	void recordMidiAction(ID channelId, Scene scene, int note, float velocity, TickRange, Tick ticksInLoop);
	void recordSampleAction(ID channelId, Scene, int type, TickRange, Tick ticksInLoop);

	/* delete*Action */

	void deleteMidiAction(const Action&);
	void deleteSampleAction(const Action&);

	/* update*Action */

	void updateMidiAction(ID channelId, Scene, const Action&, int note, float velocity, TickRange, Tick ticksInLoop);
	void updateSampleAction(ID channelId, Scene, const Action&, int type, TickRange, Tick ticksInLoop);
	void updateVelocity(const Action&, float value);

	/* consolidate
	Records all live actions. Returns a set of channels IDs that have been
	recorded. */

	std::unordered_set<ID> consolidate(Scene);

	/* clearAllActions
	Deletes all recorded actions from the given scene. Pass an invalid scene
	to delete everything. */

	void clearAllActions(Scene);

	/* Pass-thru functions. See Actions.h */

	const Action*       findAction(ID) const;
	bool                hasActions(ID channelId, int type = 0) const;
	bool                hasActions(Scene) const;
	std::vector<Action> getActionsOnChannel(ID channelId, Scene) const;

	/* clearChannel
	Removes all actions from channel 'channelId' from a given scene. Pass an invalid Scene
	object to remove all actions from all scenes. */

	void clearChannel(ID channelId, Scene);

	void   clearActions(ID channelId, int type);
	Action rec(ID channelId, Scene, Tick, MidiEvent e);
	void   rec(ID channelId, Scene, TickRange, MidiEvent e1, MidiEvent e2);
	void   updateSiblings(ID id, ID prevId, ID nextId);
	void   deleteAction(ID id);
	void   deleteAction(ID currId, ID nextId);
	void   updateEvent(ID id, MidiEvent e);

private:
	/* areComposite
	Composite: NOTE_ON + NOTE_OFF on the same note. */

	bool areComposite(const Action& a1, const Action& a2) const;

	Frame fixVerticalEnvActions(Frame f, const Action& a1, const Action& a2) const;
	bool  isSinglePressMode(ID channelId) const;

	/* consolidate
	Given an action 'a1' tries to find the matching NOTE_OFF and updates the
	action accordingly. */

	void consolidate(const Action& a1, std::size_t i);

	/* cloneActions
	Internal generic method for copying actions between channels and/or scenes.
	Pass invalid channelId's (both) to copy everything between scenes. */

	void copyActions(ID channelId, Scene src, Scene dst, ID newChannelId);

	model::Model&       m_model;
	std::vector<Action> m_liveActions;
};
} // namespace giada::m

#endif
