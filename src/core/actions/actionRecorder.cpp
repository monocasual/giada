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

#include "core/actions/actionRecorder.h"
#include "core/actions/action.h"
#include "core/actions/actions.h"
#include "core/const.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "utils/log.h"
#include "utils/ver.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <unordered_map>

namespace giada::m
{
namespace
{
constexpr int MAX_LIVE_RECS_CHUNK = 128;
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ActionRecorder::ActionRecorder(model::Model& m)
: m_model(m)
, m_actions(m)
{
	m_liveActions.reserve(MAX_LIVE_RECS_CHUNK);
}

/* -------------------------------------------------------------------------- */

void ActionRecorder::reset()
{
	m_liveActions.clear();
	m_actions.reset();
}

/* -------------------------------------------------------------------------- */

bool ActionRecorder::isBoundaryEnvelopeAction(const Action& a) const
{
	assert(a.prev != nullptr);
	assert(a.next != nullptr);
	return a.prev->frame > a.frame || a.next->frame < a.frame;
}

/* -------------------------------------------------------------------------- */

void ActionRecorder::updateBpm(float ratio, int quantizerStep)
{
	if (ratio == 1.0f)
		return;

	m_actions.updateKeyFrames([=](Frame old) {
		/* The division here cannot be precise. A new frame can be 44099 and the 
		quantizer set to 44100. That would mean two recs completely useless. So we 
		compute a reject value ('delta'): if it's lower than 6 frames the new frame 
		is collapsed with a quantized frame. FIXME - maybe 6 frames are too low. */
		Frame frame = static_cast<Frame>(old * ratio);
		if (frame != 0)
		{
			Frame delta = quantizerStep % frame;
			if (delta > 0 && delta <= 6)
				frame = frame + delta;
		}
		return frame;
	});
}

/* -------------------------------------------------------------------------- */

void ActionRecorder::updateSamplerate(int systemRate, int patchRate)
{
	if (systemRate == patchRate)
		return;

	float ratio = systemRate / (float)patchRate;

	m_actions.updateKeyFrames([=](Frame old) { return floorf(old * ratio); });
}

/* -------------------------------------------------------------------------- */

bool ActionRecorder::cloneActions(ID channelId, ID newChannelId)
{
	bool                       cloned = false;
	std::vector<Action>        actions;
	std::unordered_map<ID, ID> map; // Action ID mapper, old -> new

	m_actions.forEachAction([&](const Action& a) {
		if (a.channelId != channelId)
			return;

		ID newActionId = m_actions.getNewActionId();

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

	m_actions.rec(actions);

	return cloned;
}

/* -------------------------------------------------------------------------- */

void ActionRecorder::liveRec(ID channelId, MidiEvent e, Frame globalFrame)
{
	assert(e.isNoteOnOff()); // Can't record any other kind of events for now

	/* TODO - this might allocate on the MIDI thread */
	if (m_liveActions.size() >= m_liveActions.capacity())
		m_liveActions.reserve(m_liveActions.size() + MAX_LIVE_RECS_CHUNK);

	m_liveActions.push_back(m_actions.makeAction(m_actions.getNewActionId(), channelId, globalFrame, e));
}

/* -------------------------------------------------------------------------- */

std::unordered_set<ID> ActionRecorder::consolidate()
{
	for (auto it = m_liveActions.begin(); it != m_liveActions.end(); ++it)
		consolidate(*it, it - m_liveActions.begin()); // Pass current index

	m_actions.rec(m_liveActions);

	std::unordered_set<ID> out;
	for (const Action& action : m_liveActions)
		out.insert(action.channelId);

	m_liveActions.clear();
	return out;
}

/* -------------------------------------------------------------------------- */

void ActionRecorder::clearAllActions()
{
	for (Channel& ch : m_model.get().channels)
		ch.hasActions = false;
	m_model.swap(model::SwapType::HARD);

	m_actions.clearAll();
}

/* -------------------------------------------------------------------------- */

Actions::Map ActionRecorder::deserializeActions(const std::vector<Patch::Action>& pactions)
{
	Actions::Map out;

	/* First pass: add actions with no relationship, that is with no prev/next
	pointers filled in. */

	for (const Patch::Action& paction : pactions)
		out[paction.frame].push_back(m_actions.makeAction(paction));

	/* Second pass: fill in previous and next actions, if any. Is this the
	fastest/smartest way to do it? Maybe not. Optimizations are welcome. */

	for (const Patch::Action& paction : pactions)
	{
		if (paction.nextId == 0 && paction.prevId == 0)
			continue;
		Action* curr = const_cast<Action*>(getActionPtrById(paction.id, out));
		assert(curr != nullptr);
		if (paction.nextId != 0)
		{
			curr->next = getActionPtrById(paction.nextId, out);
			assert(curr->next != nullptr);
		}
		if (paction.prevId != 0)
		{
			curr->prev = getActionPtrById(paction.prevId, out);
			assert(curr->prev != nullptr);
		}
	}

	return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Patch::Action> ActionRecorder::serializeActions(const Actions::Map& actions)
{
	std::vector<Patch::Action> out;
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

/* -------------------------------------------------------------------------- */

const Action* ActionRecorder::getActionPtrById(int id, const Actions::Map& source)
{
	for (const auto& [_, actions] : source)
		for (const Action& action : actions)
			if (action.id == id)
				return &action;
	return nullptr;
}

/* -------------------------------------------------------------------------- */

bool ActionRecorder::areComposite(const Action& a1, const Action& a2) const
{
	return a1.event.getStatus() == MidiEvent::NOTE_ON &&
	       a2.event.getStatus() == MidiEvent::NOTE_OFF &&
	       a1.event.getNote() == a2.event.getNote() &&
	       a1.channelId == a2.channelId;
}

/* -------------------------------------------------------------------------- */

void ActionRecorder::consolidate(const Action& a1, std::size_t i)
{
	/* This algorithm must start searching from the element next to 'a1': since 
	live actions are recorded in linear sequence, the potential partner of 'a1' 
	always lies beyond a1 itself. Without this trick (i.e. if it loops from 
	vector.begin() each time) the algorithm would end up matching wrong partners. */

	for (auto it = m_liveActions.begin() + i; it != m_liveActions.end(); ++it)
	{

		const Action& a2 = *it;

		if (!areComposite(a1, a2))
			continue;

		const_cast<Action&>(a1).nextId = a2.id;
		const_cast<Action&>(a2).prevId = a1.id;

		break;
	}
}

/* -------------------------------------------------------------------------- */

const std::vector<Action>* ActionRecorder::getActionsOnFrame(Frame f) const
{
	return m_actions.getActionsOnFrame(f);
}

bool ActionRecorder::hasActions(ID channelId, int type) const
{
	return m_actions.hasActions(channelId, type);
}

Action ActionRecorder::getClosestAction(ID channelId, Frame f, int type) const
{
	return m_actions.getClosestAction(channelId, f, type);
}

std::vector<Action> ActionRecorder::getActionsOnChannel(ID channelId) const
{
	return m_actions.getActionsOnChannel(channelId);
}

void ActionRecorder::clearChannel(ID channelId)
{
	m_actions.clearChannel(channelId);
}

void ActionRecorder::clearActions(ID channelId, int type)
{
	m_actions.clearActions(channelId, type);
}

Action ActionRecorder::rec(ID channelId, Frame frame, MidiEvent e)
{
	return m_actions.rec(channelId, frame, e);
}

void ActionRecorder::rec(ID channelId, Frame f1, Frame f2, MidiEvent e1, MidiEvent e2)
{
	return m_actions.rec(channelId, f1, f2, e1, e2);
}

void ActionRecorder::updateSiblings(ID id, ID prevId, ID nextId)
{
	m_actions.updateSiblings(id, prevId, nextId);
}

void ActionRecorder::deleteAction(ID id)
{
	m_actions.deleteAction(id);
}

void ActionRecorder::deleteAction(ID currId, ID nextId)
{
	m_actions.deleteAction(currId, nextId);
}

void ActionRecorder::updateEvent(ID id, MidiEvent e)
{
	m_actions.updateEvent(id, e);
}
} // namespace giada::m
