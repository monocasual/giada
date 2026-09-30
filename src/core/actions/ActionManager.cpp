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

	m_model.write(model::SwapType::HARD, [](model::DocumentEdit& edit)
	{
		edit.get().actions.clearAll();
	});
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
    Tick tick, Tick ticksInLoop)
{
	MidiEvent e1 = MidiEvent::makeFrom3Bytes(type, 0, G_MAX_VELOCITY);
	rec(channelId, scene, sanitizeTick_(tick, ticksInLoop), e1);
}

/* -------------------------------------------------------------------------- */

void ActionManager::deleteMidiAction(ID actionId)
{
	const auto  view    = m_model.read();
	const auto& actions = view.getDocument().actions;
	const auto  curr    = actions.findAction(actionId);

	assert(curr.has_value());
	assert(curr->isValid());
	assert(curr->event.getStatus() == MidiEvent::CHANNEL_NOTE_ON);

	/* Check if 'next' exist first: could be orphaned. */

	const auto next = actions.findAction(curr->nextId);

	if (next.has_value())
		deleteAction(curr->id, next->id);
	else
		deleteAction(curr->id);
}

/* -------------------------------------------------------------------------- */

void ActionManager::deleteSampleAction(ID actionId)
{
	const auto action = findAction(actionId);
	assert(action.has_value());
	deleteAction(action->id);
}

/* -------------------------------------------------------------------------- */

void ActionManager::updateMidiAction(ID channelId, Scene scene, ID actionId, int note,
    float velocity, TickRange range, Tick ticksInLoop)
{
	const auto action = findAction(actionId);
	assert(action.has_value());

	deleteAction(action->id, action->nextId);
	recordMidiAction(channelId, scene, note, velocity, range, ticksInLoop);
}

/* -------------------------------------------------------------------------- */

void ActionManager::updateSampleAction(ID channelId, Scene scene, ID actionId,
    int type, Tick tick, Tick ticksInLoop)
{
	const auto action = findAction(actionId);
	assert(action.has_value());

	deleteAction(action->id);
	recordSampleAction(channelId, scene, type, tick, ticksInLoop);
}

/* -------------------------------------------------------------------------- */

void ActionManager::updateVelocity(ID actionId, float value)
{
	const auto action = findAction(actionId);
	assert(action.has_value());

	MidiEvent event(action->event);
	event.setVelocityFloat(value);
	updateEvent(action->id, event);
}

/* -------------------------------------------------------------------------- */

std::unordered_set<ID> ActionManager::consolidate(Scene scene)
{
	for (auto it = m_liveActions.begin(); it != m_liveActions.end(); ++it)
		consolidate(*it, it - m_liveActions.begin()); // Pass current index

	m_model.write(model::SwapType::SOFT, [this, scene](model::DocumentEdit& edit)
	{
		edit.get().actions.rec(m_liveActions, scene);
	});

	m_model.flush();

	std::unordered_set<ID> out;
	for (const model::Action& action : m_liveActions)
		out.insert(action.channelId);

	m_liveActions.clear();
	return out;
}

/* -------------------------------------------------------------------------- */

void ActionManager::clearAllActions(Scene scene)
{
	m_model.write(model::SwapType::HARD, [scene](model::DocumentEdit& edit)
	{
		if (scene.isValid())
			edit.get().actions.clearActions(scene);
		else
			edit.get().actions.clearAll();
	});
}

/* -------------------------------------------------------------------------- */

bool ActionManager::areComposite(const model::Action& a1, const model::Action& a2) const
{
	return a1.event.getStatus() == MidiEvent::CHANNEL_NOTE_ON &&
	       a2.event.getStatus() == MidiEvent::CHANNEL_NOTE_OFF &&
	       a1.event.getNote() == a2.event.getNote() &&
	       a1.channelId == a2.channelId;
}

/* -------------------------------------------------------------------------- */

void ActionManager::consolidate(model::Action& a1, std::size_t i)
{
	/* This algorithm must start searching from the element next to 'a1': since
	live actions are recorded in linear sequence, the potential partner of 'a1'
	always lies beyond a1 itself. Without this trick (i.e. if it loops from
	vector.begin() each time) the algorithm would end up matching wrong partners. */

	for (auto it = m_liveActions.begin() + i; it != m_liveActions.end(); ++it)
	{
		model::Action& a2 = *it;

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

	m_model.write(model::SwapType::HARD, [channelId, src, dst, newChannelId](model::DocumentEdit& edit)
	{
		model::Document&           doc   = edit.get();
		const Scene                scene = src == dst ? src : dst;
		std::vector<model::Action> actions;
		std::unordered_map<ID, ID> map; // Action ID mapper, old -> new

		for (const model::Action& a : doc.actions.getAll())
		{
			if (channelId.isValid() && (a.channelId != channelId || a.scene != src))
				return;

			ID newActionId = actionFactory::getNewActionId();

			map.insert({a.id, newActionId});

			model::Action clone(a);
			clone.id        = newActionId;
			clone.channelId = newChannelId.isValid() ? newChannelId : clone.channelId;
			clone.scene     = scene;

			actions.push_back(clone);
		};

		/* Update nextId and prevId relationships given the new action ID. */

		for (model::Action& a : actions)
		{
			if (a.prevId.isValid())
				a.prevId = map.at(a.prevId);
			if (a.nextId.isValid())
				a.nextId = map.at(a.nextId);
		}

		doc.actions.rec(actions, scene);
	});
}

/* -------------------------------------------------------------------------- */

std::optional<model::Action> ActionManager::findAction(ID id) const
{
	return m_model.read().getDocument().actions.findAction(id);
}

bool ActionManager::hasActions(ID channelId, int type) const
{
	return m_model.read().getDocument().actions.hasActions(channelId, type);
}

bool ActionManager::hasActions(Scene scene) const
{
	return m_model.read().getDocument().actions.hasActions(scene);
}

std::vector<model::Action> ActionManager::getActionsOnChannel(ID channelId, Scene scene) const
{
	return m_model.read().getDocument().actions.getActionsOnChannel(channelId, scene);
}

/* -------------------------------------------------------------------------- */

void ActionManager::clearChannel(ID channelId, Scene sceneToClear)
{
	m_model.write(model::SwapType::HARD, [channelId, sceneToClear](model::DocumentEdit& edit)
	{
		for (const std::size_t sceneIndex : utils::container::range(G_MAX_NUM_SCENES))
			if (!sceneToClear.isValid() || (sceneToClear.isValid() && sceneToClear.getIndex() == sceneIndex))
				edit.get().actions.clearChannel(channelId, Scene{sceneIndex});
	});
}

void ActionManager::clearActions(ID channelId, int type)
{
	m_model.write(model::SwapType::HARD, [channelId, type](model::DocumentEdit& edit)
	{
		edit.get().actions.clearActions(channelId, type);
	});
}

void ActionManager::rec(ID channelId, Scene scene, Tick tick, MidiEvent e)
{
	m_model.write(model::SwapType::HARD, [channelId, scene, tick, e](model::DocumentEdit& edit)
	{
		edit.get().actions.rec(channelId, scene, tick, e);
	});
}

void ActionManager::rec(ID channelId, Scene scene, TickRange range, MidiEvent e1, MidiEvent e2)
{
	m_model.write(model::SwapType::HARD, [channelId, scene, range, e1, e2](model::DocumentEdit& edit)
	{
		edit.get().actions.rec(channelId, scene, range, e1, e2);
	});
}

void ActionManager::deleteAction(ID id)
{
	m_model.write(model::SwapType::HARD, [id](model::DocumentEdit& edit)
	{
		edit.get().actions.deleteAction(id);
	});
}

void ActionManager::deleteAction(ID currId, ID nextId)
{
	m_model.write(model::SwapType::HARD, [currId, nextId](model::DocumentEdit& edit)
	{
		edit.get().actions.deleteAction(currId, nextId);
	});
}

void ActionManager::updateEvent(ID id, MidiEvent e)
{
	m_model.write(model::SwapType::HARD, [id, e](model::DocumentEdit& edit)
	{
		edit.get().actions.updateEvent(id, e);
	});
}
} // namespace giada::m
