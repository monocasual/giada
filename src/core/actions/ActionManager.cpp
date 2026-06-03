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

#include "src/core/actions/ActionManager.h"
#include "src/core/actions/action.h"
#include "src/core/actions/actionFactory.h"
#include "src/core/const.h"
#include "src/core/model/actions.h"
#include "src/core/model/model.h"
#include "src/core/patch.h"
#include "src/deps/mcl-utils/src/container.hpp"
#include "src/utils/log.h"
#include "src/utils/ver.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <unordered_map>

namespace utils = mcl::utils;

namespace giada::m
{
namespace
{
constexpr int MAX_LIVE_RECS_CHUNK = 128;

/* -------------------------------------------------------------------------- */

TickRange sanitizeTickRange_(TickRange r, Tick ticksInLoop)
{
	if (!r.isValid())
		r.setB(r.getA() + G_DEFAULT_ACTION_SIZE);

	/* Avoid overflow. */

	const Tick overflow = r.getB() - ticksInLoop + Tick{1};
	if (overflow > Tick{0})
		r -= overflow;

	return r;
}

Tick sanitizeTick_(Tick t, Tick ticksInLoop)
{
	return std::min(t, ticksInLoop - Tick{1});
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ActionManager::ActionManager(model::Model& m)
: m_model(m)
{
	m_liveActions.reserve(MAX_LIVE_RECS_CHUNK);
}

/* -------------------------------------------------------------------------- */

void ActionManager::reset()
{
	m_liveActions.clear();
	actionFactory::reset();
	m_model.get().actions.clearAll();
	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void ActionManager::cloneActions(ID channelId, Scene scene, ID newChannelId)
{
	copyActions(channelId, scene, scene, newChannelId);
}

/* -------------------------------------------------------------------------- */

void ActionManager::copyActionsToScene(ID channelId, Scene src, Scene dst)
{
	copyActions(channelId, src, dst, channelId);
}

/* -------------------------------------------------------------------------- */

void ActionManager::copyAllActionsToScene(Scene src, Scene dst)
{
	copyActions({}, src, dst, {});
}

/* -------------------------------------------------------------------------- */

void ActionManager::liveRec(ID channelId, Scene scene, MidiEvent e, Tick globalTick)
{
	assert(e.isNoteOnOff()); // Can't record any other kind of events for now

	/* TODO - this might allocate on the MIDI thread */
	if (m_liveActions.size() >= m_liveActions.capacity())
		m_liveActions.reserve(m_liveActions.size() + MAX_LIVE_RECS_CHUNK);

	m_liveActions.push_back(actionFactory::makeAction(actionFactory::getNewActionId(), channelId, scene, globalTick, e));
}

/* -------------------------------------------------------------------------- */

void ActionManager::recordMidiAction(ID channelId, Scene scene, int note, float velocity,
    TickRange range, Tick ticksInLoop)
{
	MidiEvent e1 = MidiEvent::makeFrom3Bytes(MidiEvent::CHANNEL_NOTE_ON, note, 0);
	MidiEvent e2 = MidiEvent::makeFrom3Bytes(MidiEvent::CHANNEL_NOTE_OFF, note, 0);

	e1.setVelocityFloat(velocity);
	e2.setVelocityFloat(velocity);

	rec(channelId, scene, sanitizeTickRange_(range, ticksInLoop), e1, e2);
}

/* -------------------------------------------------------------------------- */

void ActionManager::recordSampleAction(ID channelId, Scene scene, int type,
    TickRange range, Tick ticksInLoop)
{
	if (isSinglePressMode(channelId))
	{
		range = sanitizeTickRange_(range, ticksInLoop);

		MidiEvent e1 = MidiEvent::makeFrom3Bytes(MidiEvent::CHANNEL_NOTE_ON, 0, G_MAX_VELOCITY);
		MidiEvent e2 = MidiEvent::makeFrom3Bytes(MidiEvent::CHANNEL_NOTE_OFF, 0, G_MAX_VELOCITY);
		rec(channelId, scene, range, e1, e2);
	}
	else
	{
		MidiEvent e1 = MidiEvent::makeFrom3Bytes(type, 0, G_MAX_VELOCITY);
		rec(channelId, scene, sanitizeTick_(range.getA(), ticksInLoop), e1);
	}
}

/* -------------------------------------------------------------------------- */

void ActionManager::deleteMidiAction(ID actionId)
{
	const Action* curr = m_model.get().actions.findAction(actionId);

	assert(curr->isValid());
	assert(curr->event.getStatus() == MidiEvent::CHANNEL_NOTE_ON);

	/* Check if 'next' exist first: could be orphaned. */

	const Action* next = m_model.get().actions.findAction(curr->nextId);

	if (next != nullptr)
		deleteAction(curr->id, next->id);
	else
		deleteAction(curr->id);
}

/* -------------------------------------------------------------------------- */

void ActionManager::deleteSampleAction(ID actionId)
{
	const Action* curr = m_model.get().actions.findAction(actionId);
	const Action* next = m_model.get().actions.findAction(curr->nextId);

	if (next != nullptr) // For ChannelMode::SINGLE_PRESS combo
		deleteAction(curr->id, next->id);
	else
		deleteAction(curr->id);
}

/* -------------------------------------------------------------------------- */

void ActionManager::updateMidiAction(ID channelId, Scene scene, ID actionId, int note,
    float velocity, TickRange range, Tick ticksInLoop)
{
	const Action* action = m_model.get().actions.findAction(actionId);

	deleteAction(action->id, action->nextId);
	recordMidiAction(channelId, scene, note, velocity, range, ticksInLoop);
}

/* -------------------------------------------------------------------------- */

void ActionManager::updateSampleAction(ID channelId, Scene scene, ID actionId,
    int type, TickRange range, Tick ticksInLoop)
{
	const Action* action = m_model.get().actions.findAction(actionId);

	if (isSinglePressMode(channelId))
		deleteAction(action->id, action->nextId);
	else
		deleteAction(action->id);

	recordSampleAction(channelId, scene, type, range, ticksInLoop);
}

/* -------------------------------------------------------------------------- */

void ActionManager::updateVelocity(ID actionId, float value)
{
	const Action* action = m_model.get().actions.findAction(actionId);

	MidiEvent event(action->event);
	event.setVelocityFloat(value);
	updateEvent(action->id, event);
}

/* -------------------------------------------------------------------------- */

std::unordered_set<ID> ActionManager::consolidate(Scene scene)
{
	for (auto it = m_liveActions.begin(); it != m_liveActions.end(); ++it)
		consolidate(*it, it - m_liveActions.begin()); // Pass current index

	m_model.get().actions.rec(m_liveActions, scene);
	m_model.swap(model::SwapType::SOFT);

	std::unordered_set<ID> out;
	for (const Action& action : m_liveActions)
		out.insert(action.channelId);

	m_liveActions.clear();
	return out;
}

/* -------------------------------------------------------------------------- */

void ActionManager::clearAllActions(Scene scene)
{
	if (scene.isValid())
		m_model.get().actions.clearActions(scene);
	else
		m_model.get().actions.clearAll();
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

bool ActionManager::areComposite(const Action& a1, const Action& a2) const
{
	return a1.event.getStatus() == MidiEvent::CHANNEL_NOTE_ON &&
	       a2.event.getStatus() == MidiEvent::CHANNEL_NOTE_OFF &&
	       a1.event.getNote() == a2.event.getNote() &&
	       a1.channelId == a2.channelId;
}

/* -------------------------------------------------------------------------- */

bool ActionManager::isSinglePressMode(ID channelId) const
{
	return m_model.get().tracks.getChannel(channelId).sampleChannel->mode == SamplePlayerMode::SINGLE_PRESS;
}

/* -------------------------------------------------------------------------- */

void ActionManager::consolidate(Action& a1, std::size_t i)
{
	/* This algorithm must start searching from the element next to 'a1': since
	live actions are recorded in linear sequence, the potential partner of 'a1'
	always lies beyond a1 itself. Without this trick (i.e. if it loops from
	vector.begin() each time) the algorithm would end up matching wrong partners. */

	for (auto it = m_liveActions.begin() + i; it != m_liveActions.end(); ++it)
	{
		Action& a2 = *it;

		if (!areComposite(a1, a2))
			continue;

		a1.nextId = a2.id;
		a2.prevId = a1.id;

		break;
	}
}

/* -------------------------------------------------------------------------- */

void ActionManager::copyActions(ID channelId, Scene src, Scene dst, ID newChannelId)
{
	// Both channel IDs must have the same validity state
	assert(channelId.isValid() == newChannelId.isValid());

	const Scene                scene = src == dst ? src : dst;
	std::vector<Action>        actions;
	std::unordered_map<ID, ID> map; // Action ID mapper, old -> new

	for (const Action& a : m_model.get().actions.getAll())
	{
		if (channelId.isValid() && (a.channelId != channelId || a.scene != src))
			return;

		ID newActionId = actionFactory::getNewActionId();

		map.insert({a.id, newActionId});

		Action clone(a);
		clone.id        = newActionId;
		clone.channelId = newChannelId.isValid() ? newChannelId : clone.channelId;
		clone.scene     = scene;

		actions.push_back(clone);
	};

	/* Update nextId and prevId relationships given the new action ID. */

	for (Action& a : actions)
	{
		if (a.prevId.isValid())
			a.prevId = map.at(a.prevId);
		if (a.nextId.isValid())
			a.nextId = map.at(a.nextId);
	}

	m_model.get().actions.rec(actions, scene);
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

const Action* ActionManager::findAction(ID id) const
{
	return m_model.get().actions.findAction(id);
}

bool ActionManager::hasActions(ID channelId, int type) const
{
	return m_model.get().actions.hasActions(channelId, type);
}

bool ActionManager::hasActions(Scene scene) const
{
	return m_model.get().actions.hasActions(scene);
}

std::vector<Action> ActionManager::getActionsOnChannel(ID channelId, Scene scene) const
{
	return m_model.get().actions.getActionsOnChannel(channelId, scene);
}

/* -------------------------------------------------------------------------- */

void ActionManager::clearChannel(ID channelId, Scene sceneToClear)
{
	for (const std::size_t sceneIndex : utils::container::range(G_MAX_NUM_SCENES))
		if (!sceneToClear.isValid() || (sceneToClear.isValid() && sceneToClear.getIndex() == sceneIndex))
			m_model.get().actions.clearChannel(channelId, Scene{sceneIndex});
	m_model.swap(model::SwapType::HARD);
}

void ActionManager::clearActions(ID channelId, int type)
{
	m_model.get().actions.clearActions(channelId, type);
	m_model.swap(model::SwapType::HARD);
}

Action ActionManager::rec(ID channelId, Scene scene, Tick tick, MidiEvent e)
{
	Action action = m_model.get().actions.rec(channelId, scene, tick, e);

	m_model.swap(model::SwapType::HARD);
	return action;
}

void ActionManager::rec(ID channelId, Scene scene, TickRange range, MidiEvent e1, MidiEvent e2)
{
	m_model.get().actions.rec(channelId, scene, range, e1, e2);
	m_model.swap(model::SwapType::HARD);
}

void ActionManager::updateSiblings(ID id, ID prevId, ID nextId)
{
	m_model.get().actions.updateSiblings(id, prevId, nextId);
	m_model.swap(model::SwapType::HARD);
}

void ActionManager::deleteAction(ID id)
{
	m_model.get().actions.deleteAction(id);
	m_model.swap(model::SwapType::HARD);
}

void ActionManager::deleteAction(ID currId, ID nextId)
{
	m_model.get().actions.deleteAction(currId, nextId);
	m_model.swap(model::SwapType::HARD);
}

void ActionManager::updateEvent(ID id, MidiEvent e)
{
	m_model.get().actions.updateEvent(id, e);
	m_model.swap(model::SwapType::HARD);
}
} // namespace giada::m
