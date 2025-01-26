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

#include "core/model/actions.h"
#include "core/actions/actionFactory.h"
#include "utils/log.h"
#include <algorithm>
#include <cassert>
#include <memory>
#ifdef G_DEBUG_MODE
#include <fmt/core.h>
#endif

namespace giada::m::model
{
void Actions::set(model::Actions::Map&& actions)
{
	m_actions = std::move(actions);
}

void Actions::clearAll()
{
	m_actions.clear();
}

/* -------------------------------------------------------------------------- */

void Actions::clearChannel(ID channelId)
{
	removeIf([=](const Action& a)
	{ return a.channelId == channelId; });
}

/* -------------------------------------------------------------------------- */

void Actions::clearActions(ID channelId, int type)
{
	removeIf([=](const Action& a)
	{
		return a.channelId == channelId && a.event.getStatus() == type;
	});
}

/* -------------------------------------------------------------------------- */

void Actions::deleteAction(ID id)
{
	removeIf([=](const Action& a)
	{ return a.id == id; });
}

void Actions::deleteAction(ID currId, ID nextId)
{
	removeIf([=](const Action& a)
	{ return a.id == currId || a.id == nextId; });
}

/* -------------------------------------------------------------------------- */

void Actions::updateKeyFrames(std::function<Frame(Frame old)> f)
{
	Map temp;

	/* Copy all existing actions in local map by cloning them, with just a
	difference: they have a new frame value. */

	for (const auto& [oldFrame, actions] : m_actions)
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

	m_actions = std::move(temp);
}

/* -------------------------------------------------------------------------- */

void Actions::updateEvent(ID id, MidiEvent e)
{
	Action* a = findAction(m_actions, id);
	assert(a != nullptr);
	a->event = e;
}

/* -------------------------------------------------------------------------- */

void Actions::updateSiblings(ID id, ID prevId, ID nextId)
{
	Action* pcurr = findAction(m_actions, id);
	Action* pprev = findAction(m_actions, prevId);
	Action* pnext = findAction(m_actions, nextId);

	pcurr->prevId = pprev->id;
	pcurr->nextId = pnext->id;

	if (pprev->id != 0)
	{
		pprev->nextId = pcurr->id;
	}
	if (pnext->id != 0)
	{
		pnext->prevId = pcurr->id;
	}
}

/* -------------------------------------------------------------------------- */

bool Actions::hasActions(ID channelId, int type) const
{
	for (const auto& [frame, actions] : m_actions)
		for (const Action& a : actions)
			if (a.channelId == channelId && (type == 0 || type == a.event.getStatus()))
				return true;
	return false;
}

/* -------------------------------------------------------------------------- */

const Actions::Map& Actions::getAll() const { return m_actions; }

/* -------------------------------------------------------------------------- */

const Action* Actions::findAction(ID id) const { return findAction(m_actions, id); }

/* -------------------------------------------------------------------------- */

#ifdef G_DEBUG_MODE

void Actions::debug() const
{
	puts("model::actions");

	for (const auto& [frame, actions] : m_actions)
	{
		fmt::print("\tframe: {}\n", frame);
		for (const Action& a : actions)
			fmt::print("\t\t({}) - ID={}, frame={}, channel={}, value=0x{}, prevId={}, nextId={}\n",
			    (void*)&a, a.id, a.frame, a.channelId, a.event.getRaw(), a.prevId, a.nextId);
	}
}

#endif

/* -------------------------------------------------------------------------- */

Action Actions::rec(ID channelId, Frame frame, MidiEvent event)
{
	/* Skip duplicates. */

	if (exists(channelId, frame, event))
		return {};

	Action a = actionFactory::makeAction(0, channelId, frame, event);

	/* If key frame doesn't exist yet, the [] operator in std::map is smart
	enough to insert a new item first. No plug-in data for now. */

	m_actions[frame].push_back(a);

	return a;
}

/* -------------------------------------------------------------------------- */

void Actions::rec(std::vector<Action>& actions)
{
	if (actions.size() == 0)
		return;

	for (const Action& a : actions)
		if (!exists(a.channelId, a.frame, a.event, m_actions))
			m_actions[a.frame].push_back(a);
}

/* -------------------------------------------------------------------------- */

void Actions::rec(ID channelId, Frame f1, Frame f2, MidiEvent e1, MidiEvent e2)
{
	m_actions[f1].push_back(actionFactory::makeAction(0, channelId, f1, e1));
	m_actions[f2].push_back(actionFactory::makeAction(0, channelId, f2, e2));

	Action* a1 = findAction(m_actions, m_actions[f1].back().id);
	Action* a2 = findAction(m_actions, m_actions[f2].back().id);
	a1->nextId = a2->id;
	a2->prevId = a1->id;
}

/* -------------------------------------------------------------------------- */

const std::vector<Action>* Actions::getActionsOnFrame(Frame frame) const
{
	if (m_actions.count(frame) == 0)
		return nullptr;
	return &m_actions.at(frame);
}

/* -------------------------------------------------------------------------- */

Action Actions::getClosestAction(ID channelId, Frame f, int type) const
{
	Action out = {};
	forEachAction([&](const Action& a)
	{
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
	forEachAction([&](const Action& a)
	{
		if (a.channelId == channelId)
			out.push_back(a);
	});
	return out;
}

/* -------------------------------------------------------------------------- */

void Actions::forEachAction(std::function<void(const Action&)> f) const
{
	for (auto& [_, actions] : m_actions)
		for (const Action& action : actions)
			f(action);
}

/* -------------------------------------------------------------------------- */

const Action* Actions::findAction(const Map& src, ID id) const
{
	if (id == 0)
		return nullptr;
	for (const auto& [frame, actions] : src)
		for (const Action& a : actions)
			if (a.id == id)
				return &a;
	return nullptr;
}

Action* Actions::findAction(Map& src, ID id)
{
	return const_cast<Action*>(std::as_const(*this).findAction(src, id));
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
	for (auto& [frame, actions] : m_actions)
		actions.erase(std::remove_if(actions.begin(), actions.end(), f), actions.end());
	optimize(m_actions);
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
	return exists(channelId, frame, event, m_actions);
}
} // namespace giada::m::model
