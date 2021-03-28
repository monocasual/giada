/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "recorderHandler.h"
#include "action.h"
#include "clock.h"
#include "const.h"
#include "model/model.h"
#include "patch.h"
#include "recorder.h"
#include "utils/log.h"
#include "utils/ver.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <unordered_map>

namespace giada::m::recorderHandler
{
namespace
{
constexpr int MAX_LIVE_RECS_CHUNK = 128;

std::vector<Action> recs_;

/* -------------------------------------------------------------------------- */

const Action* getActionPtrById_(int id, const recorder::ActionMap& source)
{
	for (const auto& [_, actions] : source)
		for (const Action& action : actions)
			if (action.id == id)
				return &action;
	return nullptr;
}

/* -------------------------------------------------------------------------- */

/* areComposite_
Composite: NOTE_ON + NOTE_OFF on the same note. */

bool areComposite_(const Action& a1, const Action& a2)
{
	return a1.event.getStatus() == MidiEvent::NOTE_ON &&
	       a2.event.getStatus() == MidiEvent::NOTE_OFF &&
	       a1.event.getNote() == a2.event.getNote() &&
	       a1.channelId == a2.channelId;
}

/* -------------------------------------------------------------------------- */

/* consolidate_
Given an action 'a1' tries to find the matching NOTE_OFF. This algorithm must
start searching from the element next to 'a1': since live actions are recorded
in linear sequence, the potential partner of 'a1' always lies beyond a1 itself. 
Without this trick (i.e. if it loops from vector.begin() each time) the
algorithm would end up matching wrong partners. */

void consolidate_(const Action& a1, std::size_t i)
{
	for (auto it = recs_.begin() + i; it != recs_.end(); ++it)
	{

		const Action& a2 = *it;

		if (!areComposite_(a1, a2))
			continue;

		const_cast<Action&>(a1).nextId = a2.id;
		const_cast<Action&>(a2).prevId = a1.id;

		break;
	}
}

/* -------------------------------------------------------------------------- */

void consolidate_()
{
	for (auto it = recs_.begin(); it != recs_.end(); ++it)
		consolidate_(*it, it - recs_.begin()); // Pass current index
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void init()
{
	recs_.reserve(MAX_LIVE_RECS_CHUNK);
}

/* -------------------------------------------------------------------------- */

bool isBoundaryEnvelopeAction(const Action& a)
{
	assert(a.prev != nullptr);
	assert(a.next != nullptr);
	return a.prev->frame > a.frame || a.next->frame < a.frame;
}

/* -------------------------------------------------------------------------- */

void updateBpm(float oldval, float newval, int oldquanto)
{
	recorder::updateKeyFrames([=](Frame old) {
		/* The division here cannot be precise. A new frame can be 44099 and the 
		quantizer set to 44100. That would mean two recs completely useless. So we 
		compute a reject value ('scarto'): if it's lower than 6 frames the new frame 
		is collapsed with a quantized frame. FIXME - maybe 6 frames are too low. */
		Frame frame = static_cast<Frame>((old / newval) * oldval);
		if (frame != 0)
		{
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

	float ratio = systemRate / (float)patchRate;

	recorder::updateKeyFrames([=](Frame old) { return floorf(old * ratio); });
}

/* -------------------------------------------------------------------------- */

bool cloneActions(ID channelId, ID newChannelId)
{
	bool                       cloned = false;
	std::vector<Action>        actions;
	std::unordered_map<ID, ID> map; // Action ID mapper, old -> new

	recorder::forEachAction([&](const Action& a) {
		if (a.channelId != channelId)
			return;

		ID newActionId = recorder::getNewActionId();

		map.insert({a.id, newActionId});

		Action clone(a);
		clone.id        = newActionId;
		clone.channelId = newChannelId;

		actions.push_back(clone);
		cloned = true;
	});

	/* Update nextId and prevId relationships given the new action ID. */

	for (Action& a : actions)
	{
		if (a.prevId != 0)
			a.prevId = map.at(a.prevId);
		if (a.nextId != 0)
			a.nextId = map.at(a.nextId);
	}

	recorder::rec(actions);

	return cloned;
}

/* -------------------------------------------------------------------------- */

void liveRec(ID channelId, MidiEvent e, Frame globalFrame)
{
	assert(e.isNoteOnOff()); // Can't record any other kind of events for now

	/* TODO - this might allocate on the MIDI thread */
	if (recs_.size() >= recs_.capacity())
		recs_.reserve(recs_.size() + MAX_LIVE_RECS_CHUNK);

	recs_.push_back(recorder::makeAction(recorder::getNewActionId(), channelId, globalFrame, e));
}

/* -------------------------------------------------------------------------- */

std::unordered_set<ID> consolidate()
{
	consolidate_();
	recorder::rec(recs_);

	std::unordered_set<ID> out;
	for (const Action& action : recs_)
		out.insert(action.channelId);

	recs_.clear();
	return out;
}

/* -------------------------------------------------------------------------- */

void clearAllActions()
{
	for (channel::Data& ch : model::get().channels)
		ch.hasActions = false;

	model::swap(model::SwapType::HARD);

	recorder::clearAll();
}

/* -------------------------------------------------------------------------- */

recorder::ActionMap deserializeActions(const std::vector<patch::Action>& pactions)
{
	recorder::ActionMap out;

	/* First pass: add actions with no relationship, that is with no prev/next
	pointers filled in. */

	for (const patch::Action& paction : pactions)
		out[paction.frame].push_back(recorder::makeAction(paction));

	/* Second pass: fill in previous and next actions, if any. Is this the
	fastest/smartest way to do it? Maybe not. Optimizations are welcome. */

	for (const patch::Action& paction : pactions)
	{
		if (paction.nextId == 0 && paction.prevId == 0)
			continue;
		Action* curr = const_cast<Action*>(getActionPtrById_(paction.id, out));
		assert(curr != nullptr);
		if (paction.nextId != 0)
		{
			curr->next = getActionPtrById_(paction.nextId, out);
			assert(curr->next != nullptr);
		}
		if (paction.prevId != 0)
		{
			curr->prev = getActionPtrById_(paction.prevId, out);
			assert(curr->prev != nullptr);
		}
	}

	return out;
}

/* -------------------------------------------------------------------------- */

std::vector<patch::Action> serializeActions(const recorder::ActionMap& actions)
{
	std::vector<patch::Action> out;
	for (const auto& kv : actions)
	{
		for (const Action& a : kv.second)
		{
			out.push_back({
			    a.id,
			    a.channelId,
			    a.frame,
			    a.event.getRaw(),
			    a.prevId,
			    a.nextId,
			});
		}
	}
	return out;
}
} // namespace giada::m::recorderHandler