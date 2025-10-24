/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_ACTION_RECORDER_H
#define G_ACTION_RECORDER_H

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
class ActionRecorder // TODO - rename ActionManager
{
public:
	ActionRecorder(model::Model&);

	/* reset
	Brings everything back to the initial state. */

	void reset();

	/* updateBpm
	Changes actions position by calculating the new bpm value. */

	void updateBpm(float ratio, int quantizerStep);

	/* updateSamplerate
	Changes actions position by taking in account the new samplerate. If
	f_system == f_patch nothing will change, otherwise the conversion is
	mandatory. */

	void updateSamplerate(int systemRate, int patchRate);

	/* cloneActions
	Clones actions in channel 'channelId', giving them a new channel ID. Returns
	whether any action has been cloned. */

	void cloneActions(ID channelId, std::size_t scene, ID newChannelId);

	/* copyActionsToScene
	Copies actions of a given channel from a scene to another. */

	void copyActionsToScene(ID channelId, std::size_t srcScene, std::size_t dstScene);

	/* liveRec
	Records a user-generated action. NOTE_ON or NOTE_OFF only for now. */

	void liveRec(ID channelId, std::size_t scene, MidiEvent e, Frame global);

	/* record*Action */

	void recordMidiAction(ID channelId, std::size_t scene, int note, float velocity, Frame f1, Frame f2, Frame framesInLoop);
	void recordSampleAction(ID channelId, std::size_t scene, int type, Frame f1, Frame f2, Frame framesInLoop);

	/* delete*Action */

	void deleteMidiAction(ID channelId, const Action&);
	void deleteSampleAction(ID channelId, const Action&);

	/* update*Action */

	void updateMidiAction(ID channelId, std::size_t scene, const Action&, int note, float velocity, Frame f1, Frame f2, Frame framesInLoop);
	void updateSampleAction(ID channelId, std::size_t scene, const Action&, int type, Frame f1, Frame f2, Frame framesInLoop);
	void updateVelocity(const Action&, float value);

	/* consolidate
	Records all live actions. Returns a set of channels IDs that have been
	recorded. */

	std::unordered_set<ID> consolidate(std::size_t scene);

	/* clearAllActions
	Deletes all recorded actions. */

	void clearAllActions();

	/* Pass-thru functions. See Actions.h */

	const Action*       findAction(ID) const;
	bool                hasActions(ID channelId, int type = 0) const;
	std::vector<Action> getActionsOnChannel(ID channelId, std::size_t scene) const;
	void                clearChannel(ID channelId);
	void                clearActions(ID channelId, int type);
	Action              rec(ID channelId, std::size_t scene, Frame frame, MidiEvent e);
	void                rec(ID channelId, std::size_t scene, Frame f1, Frame f2, MidiEvent e1, MidiEvent e2);
	void                updateSiblings(ID id, ID prevId, ID nextId);
	void                deleteAction(ID channelId, ID id);
	void                deleteAction(ID channelId, ID currId, ID nextId);
	void                updateEvent(ID id, MidiEvent e);

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
	Internal generic method for copying actions between channels and/or scenes. */

	void copyActions(ID channelId, std::size_t srcScene, std::size_t dstScene, ID newChannelId);

	model::Model&       m_model;
	std::vector<Action> m_liveActions;
};
} // namespace giada::m

#endif
