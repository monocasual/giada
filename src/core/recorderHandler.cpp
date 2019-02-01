/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <algorithm>
#include <cmath>
#include <cassert>
#include "../utils/log.h"
#include "../utils/ver.h"
#include "recorder.h"
#include "action.h"
#include "clock.h"
#include "const.h"
#include "recorderHandler.h"


namespace giada {
namespace m {
namespace recorderHandler
{
namespace
{
std::vector<const Action*> recs_;


/* -------------------------------------------------------------------------- */


const Action* getActionById_(int id, const recorder::ActionMap& source)
{
	for (auto& kv : source)
		for (const Action* action : kv.second)
			if (action->id == id)
				return action;
	return nullptr;
}


/* -------------------------------------------------------------------------- */


/* areComposite_
Composite: NOTE_ON + NOTE_OFF on the same note. */

bool areComposite_(const Action* a1, const Action* a2)
{
	return a1->event.getStatus() == MidiEvent::NOTE_ON  &&
	       a2->event.getStatus() == MidiEvent::NOTE_OFF &&
	       a1->event.getNote() == a2->event.getNote()   &&
	       a1->channel == a2->channel;
}


/* -------------------------------------------------------------------------- */


/* consolidate_
Given an action 'a1' tries to find the matching NOTE_OFF. This algorithm must
start searching from the element next to 'a1': since live actions are recorded
in linear sequence, the potential partner of 'a1' always lies beyond a1 itself. 
Without this trick (i.e. if it loops from vector.begin() each time) the
algorithm would end up matching wrong partners. */

void consolidate_(const Action* a1, size_t i)
{
	for (auto it = recs_.begin() + i; it != recs_.end(); ++it) {

		const Action* a2 = *it;

		if (!areComposite_(a1, a2))
			continue;

		const_cast<Action*>(a1)->next = a2;
		const_cast<Action*>(a2)->prev = a1;

		break;
	}		
}


/* -------------------------------------------------------------------------- */


void consolidate_()
{
	for (auto it = recs_.begin(); it != recs_.end(); ++it)
		consolidate_(*it, it - recs_.begin());  // Pass current index
}

/* -------------------------------------------------------------------------- */


void readPatch_DEPR_(const std::vector<patch::action_t>& pactions)
{
	for (const patch::action_t paction : pactions)
		recs_.push_back(recorder::makeAction(-1, paction.channel, paction.frame, MidiEvent(paction.event)));
	
	consolidate();
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


bool isBoundaryEnvelopeAction(const Action* a)
{
	assert(a->prev != nullptr);
	assert(a->next != nullptr);
	return a->prev->frame > a->frame || a->next->frame < a->frame;
}


/* -------------------------------------------------------------------------- */


void updateBpm(float oldval, float newval, int oldquanto)
{
	recorder::updateKeyFrames([=](Frame old) 
	{
		/* The division here cannot be precise. A new frame can be 44099 and the 
		quantizer set to 44100. That would mean two recs completely useless. So we 
		compute a reject value ('scarto'): if it's lower than 6 frames the new frame 
		is collapsed with a quantized frame. FIXME - maybe 6 frames are too low. */
		Frame frame = (old / newval) * oldval;
		if (frame != 0) {
			Frame delta = oldquanto % frame;
			if (delta > 0 && delta <= 6)
				frame = frame + delta;
		}
		return frame;
	});
}


/* -------------------------------------------------------------------------- */


void updateSamplerate(int systemRate, int patchRate)
{
	if (systemRate == patchRate)
		return;

	float ratio = systemRate / (float) patchRate;

	recorder::updateKeyFrames([=](Frame old) { return floorf(old * ratio); });
}


/* -------------------------------------------------------------------------- */


bool cloneActions(int chanIndex, int newChanIndex)
{
	recorder::ActionMap temp = recorder::getActionMap();

	bool cloned   = false;
	int  actionId = recorder::getLatestActionId(); 

	recorder::forEachAction([&](const Action* a) 
	{
		if (a->channel == chanIndex) {
			Action* clone = new Action(*a);
			clone->id      = ++actionId;
			clone->channel = newChanIndex;
			temp[clone->frame].push_back(clone);
			cloned = true;
		}
	});

	recorder::updateActionId(actionId);
	recorder::updateActionMap(std::move(temp));

	return cloned;
}


/* -------------------------------------------------------------------------- */


void liveRec(int channel, MidiEvent e)
{
	assert(e.isNoteOnOff()); // Can't record any other kind of events for now
	recs_.push_back(recorder::makeAction(-1, channel, clock::getCurrentFrame(), e));
}


/* -------------------------------------------------------------------------- */


std::unordered_set<int> consolidate()
{
	consolidate_();
	recorder::rec(recs_);

	std::unordered_set<int> out;
	for (const Action* action : recs_)
		out.insert(action->channel);

	recs_.clear();
	return out;
}


/* -------------------------------------------------------------------------- */


void writePatch(int chanIndex, std::vector<patch::action_t>& pactions)
{
	recorder::forEachAction([&] (const Action* a) 
	{
		if (a->channel != chanIndex) 
			return;
		pactions.push_back(patch::action_t { 
			a->id, 
			a->channel, 
			a->frame, 
			a->event.getRaw(), 
			a->prev != nullptr ? a->prev->id : -1,
			a->next != nullptr ? a->next->id : -1
		});
	});
}



/* -------------------------------------------------------------------------- */


void readPatch(const std::vector<patch::action_t>& pactions)
{
	if (u::ver::isLess(patch::versionMajor, patch::versionMinor, patch::versionPatch, 0, 15, 3)) {
		readPatch_DEPR_(pactions);
		return;
	}

	recorder::ActionMap temp = recorder::getActionMap();

	/* First pass: add actions with no relationship (no prev/next). */

	for (const patch::action_t paction : pactions) {
		temp[paction.frame].push_back(recorder::makeAction(
			paction.id, 
			paction.channel, 
			paction.frame, 
			MidiEvent(paction.event)));
		recorder::updateActionId(paction.id + 1);
	}

	/* Second pass: fill in previous and next actions, if any. */

	for (const patch::action_t paction : pactions) {
		if (paction.next == -1 && paction.prev == -1) 
			continue;
		Action* curr = const_cast<Action*>(getActionById_(paction.id, temp));
		assert(curr != nullptr);
		if (paction.next != -1) {
			curr->next = getActionById_(paction.next, temp);
			assert(curr->next != nullptr);
		}
		if (paction.prev != -1) {
			curr->prev = getActionById_(paction.prev, temp);
			assert(curr->prev != nullptr);
		}
	}

	recorder::updateActionMap(std::move(temp));
}
}}}; // giada::m::recorderHandler::


