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

#include "actions.h"
#include "action.h"
#include "core/idManager.h"
#include "core/model/model.h"
#include "utils/log.h"
#include <algorithm>
#include <cassert>
#include <memory>

namespace giada::m
{
Actions::Actions(model::Model& model)
: m_model(model)
{
}

/* -------------------------------------------------------------------------- */

void Actions::reset()
{
	m_actionId = IdManager();
	clearAll();
}

/* -------------------------------------------------------------------------- */

void Actions::clearAll()
{
	model::DataLock lock = m_model.lockData();
	m_model.getAllShared<Map>().clear();
}

/* -------------------------------------------------------------------------- */

void Actions::clearChannel(ID channelId)
{
	removeIf([=](const Action& a) { return a.channelId == channelId; });
}

/* -------------------------------------------------------------------------- */

void Actions::clearActions(ID channelId, int type)
{
	removeIf([=](const Action& a) {
		return a.channelId == channelId && a.event.getStatus() == type;
	});
}

/* -------------------------------------------------------------------------- */

void Actions::deleteAction(ID id)
{
	removeIf([=](const Action& a) { return a.id == id; });
}

void Actions::deleteAction(ID currId, ID nextId)
{
	removeIf([=](const Action& a) { return a.id == currId || a.id == nextId; });
}

/* -------------------------------------------------------------------------- */

void Actions::updateKeyFrames(std::function<Frame(Frame old)> f)
{
	Map temp;

	/* Copy all existing actions in local map by cloning them, with just a
	difference: they have a new frame value. */

	for (const auto& [oldFrame, actions] : m_model.getAllShared<Map>())
	{
		Frame newFrame = f(oldFrame);
		for (const Action& a : actions)
		{
			Action copy = a;
			copy.frame  = newFrame;
			temp[newFrame].push_back(copy);
		}
		G_DEBUG("{} -> {}", oldFrame, newFrame);
	}

	updateMapPointers(temp);

	model::DataLock lock        = m_model.lockData();
	m_model.getAllShared<Map>() = std::move(temp);
}

/* -------------------------------------------------------------------------- */

void Actions::updateEvent(ID id, MidiEvent e)
{
	model::DataLock lock = m_model.lockData();

	findAction(m_model.getAllShared<Map>(), id)->event = e;
}

/* -------------------------------------------------------------------------- */

void Actions::updateSiblings(ID id, ID prevId, ID nextId)
{
	model::DataLock lock = m_model.lockData();

	Action* pcurr = findAction(m_model.getAllShared<Map>(), id);
	Action* pprev = findAction(m_model.getAllShared<Map>(), prevId);
	Action* pnext = findAction(m_model.getAllShared<Map>(), nextId);

	pcurr->prev   = pprev;
	pcurr->prevId = pprev->id;
	pcurr->next   = pnext;
	pcurr->nextId = pnext->id;

	if (pprev != nullptr)
	{
		pprev->next   = pcurr;
		pprev->nextId = pcurr->id;
	}
	if (pnext != nullptr)
	{
		pnext->prev   = pcurr;
		pnext->prevId = pcurr->id;
	}
}

/* -------------------------------------------------------------------------- */

bool Actions::hasActions(ID channelId, int type) const
{
	for (const auto& [frame, actions] : m_model.getAllShared<Map>())
		for (const Action& a : actions)
			if (a.channelId == channelId && (type == 0 || type == a.event.getStatus()))
				return true;
	return false;
}

/* -------------------------------------------------------------------------- */

Action Actions::makeAction(ID id, ID channelId, Frame frame, MidiEvent e)
{
	Action out{m_actionId.generate(id), channelId, frame, e, -1, -1};
	m_actionId.set(id);
	return out;
}

Action Actions::makeAction(const Patch::Action& a)
{
	m_actionId.set(a.id);
	return Action{a.id, a.channelId, a.frame, a.event, -1, -1, a.prevId,
	    a.nextId};
}

/* -------------------------------------------------------------------------- */

Action Actions::rec(ID channelId, Frame frame, MidiEvent event)
{
	/* Skip duplicates. */

	if (exists(channelId, frame, event))
		return {};

	Action a = makeAction(0, channelId, frame, event);

	/* If key frame doesn't exist yet, the [] operator in std::map is smart 
	enough to insert a new item first. No plug-in data for now. */

	model::DataLock lock = m_model.lockData();

	m_model.getAllShared<Map>()[frame].push_back(a);
	updateMapPointers(m_model.getAllShared<Map>());

	return a;
}

/* -------------------------------------------------------------------------- */

void Actions::rec(std::vector<Action>& actions)
{
	if (actions.size() == 0)
		return;

	model::DataLock lock = m_model.lockData();

	Map& map = m_model.getAllShared<Map>();

	for (const Action& a : actions)
		if (!exists(a.channelId, a.frame, a.event, map))
			map[a.frame].push_back(a);
	updateMapPointers(map);
}

/* -------------------------------------------------------------------------- */

void Actions::rec(ID channelId, Frame f1, Frame f2, MidiEvent e1, MidiEvent e2)
{
	model::DataLock lock = m_model.lockData();

	Map& map = m_model.getAllShared<Map>();

	map[f1].push_back(makeAction(0, channelId, f1, e1));
	map[f2].push_back(makeAction(0, channelId, f2, e2));

	Action* a1 = findAction(map, map[f1].back().id);
	Action* a2 = findAction(map, map[f2].back().id);
	a1->nextId = a2->id;
	a2->prevId = a1->id;

	updateMapPointers(map);
}

/* -------------------------------------------------------------------------- */

const std::vector<Action>* Actions::getActionsOnFrame(Frame frame) const
{
	if (m_model.getAllShared<Map>().count(frame) == 0)
		return nullptr;
	return &m_model.getAllShared<Map>().at(frame);
}

/* -------------------------------------------------------------------------- */

Action Actions::getClosestAction(ID channelId, Frame f, int type) const
{
	Action out = {};
	forEachAction([&](const Action& a) {
		if (a.event.getStatus() != type || a.channelId != channelId)
			return;
		if (!out.isValid() || (a.frame <= f && a.frame > out.frame))
			out = a;
	});
	return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Action> Actions::getActionsOnChannel(ID channelId) const
{
	std::vector<Action> out;
	forEachAction([&](const Action& a) {
		if (a.channelId == channelId)
			out.push_back(a);
	});
	return out;
}

/* -------------------------------------------------------------------------- */

void Actions::forEachAction(std::function<void(const Action&)> f) const
{
	for (auto& [_, actions] : m_model.getAllShared<Map>())
		for (const Action& action : actions)
			f(action);
}

/* -------------------------------------------------------------------------- */

ID Actions::getNewActionId()
{
	return m_actionId.generate();
}

/* -------------------------------------------------------------------------- */

Action* Actions::findAction(Map& src, ID id)
{
	for (auto& [frame, actions] : src)
		for (Action& a : actions)
			if (a.id == id)
				return &a;
	assert(false);
	return nullptr;
}

/* -------------------------------------------------------------------------- */

void Actions::updateMapPointers(Map& src)
{
	for (auto& kv : src)
	{
		for (Action& action : kv.second)
		{
			if (action.nextId != 0)
				action.next = findAction(src, action.nextId);
			if (action.prevId != 0)
				action.prev = findAction(src, action.prevId);
		}
	}
}

/* -------------------------------------------------------------------------- */

void Actions::optimize(Map& map)
{
	for (auto it = map.cbegin(); it != map.cend();)
		it->second.size() == 0 ? it = map.erase(it) : ++it;
}

/* -------------------------------------------------------------------------- */

void Actions::removeIf(std::function<bool(const Action&)> f)
{
	model::DataLock lock = m_model.lockData();

	Map& map = m_model.getAllShared<Map>();
	for (auto& [frame, actions] : map)
		actions.erase(std::remove_if(actions.begin(), actions.end(), f), actions.end());
	optimize(map);
	updateMapPointers(map);
}

/* -------------------------------------------------------------------------- */

bool Actions::exists(ID channelId, Frame frame, const MidiEvent& event, const Map& target) const
{
	for (const auto& [_, actions] : target)
		for (const Action& a : actions)
			if (a.channelId == channelId && a.frame == frame && a.event.getRaw() == event.getRaw())
				return true;
	return false;
}

/* -------------------------------------------------------------------------- */

bool Actions::exists(ID channelId, Frame frame, const MidiEvent& event) const
{
	return exists(channelId, frame, event, m_model.getAllShared<Map>());
}
} // namespace giada::m
