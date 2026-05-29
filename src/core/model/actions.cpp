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

#include "src/core/model/actions.h"
#include "src/core/actions/actionFactory.h"
#include "src/deps/mcl-utils/src/container.hpp"
#include "src/utils/log.h"
#include <algorithm>
#include <cassert>
#include <memory>
#if G_DEBUG_MODE
#include <fmt/core.h>
#endif

namespace utils = mcl::utils;

namespace giada::m::model
{
void Actions::set(model::Actions::Map&& actions)
{
	m_actions_DEPR_ = std::move(actions);
}

void Actions::clearAll()
{
	m_actions_DEPR_.clear();
}

/* -------------------------------------------------------------------------- */

void Actions::clearChannel(ID channelId, Scene scene)
{
	removeIf([=](const Action& a)
	{ return a.channelId == channelId && a.scene == scene; });
}

/* -------------------------------------------------------------------------- */

void Actions::clearActions(ID channelId, int type)
{
	removeIf([=](const Action& a)
	{
		return a.channelId == channelId && a.event.getStatus() == type;
	});
}

void Actions::clearActions(Scene scene)
{
	removeIf([=](const Action& a)
	{ return a.scene == scene; });
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

	for (const auto& [oldFrame, actions] : m_actions_DEPR_)
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

	m_actions_DEPR_ = std::move(temp);
}

/* -------------------------------------------------------------------------- */

void Actions::updateEvent(ID id, MidiEvent e)
{
	Action* a = findAction(m_actions_DEPR_, id);
	assert(a != nullptr);
	a->event = e;
}

/* -------------------------------------------------------------------------- */

void Actions::updateSiblings(ID id, ID prevId, ID nextId)
{
	Action* pcurr = findAction(m_actions_DEPR_, id);
	Action* pprev = findAction(m_actions_DEPR_, prevId);
	Action* pnext = findAction(m_actions_DEPR_, nextId);

	pcurr->prevId = pprev->id;
	pcurr->nextId = pnext->id;

	if (pprev->id.isValid())
	{
		pprev->nextId = pcurr->id;
	}
	if (pnext->id.isValid())
	{
		pnext->prevId = pcurr->id;
	}
}

/* -------------------------------------------------------------------------- */

bool Actions::hasActions(ID channelId, int type) const
{
	for (const auto& [frame, actions] : m_actions_DEPR_)
		for (const Action& a : actions)
			if (a.channelId == channelId && (type == 0 || type == a.event.getStatus()))
				return true;
	return false;
}

bool Actions::hasActions(Scene scene) const
{
	return utils::container::hasIf(m_actions_DEPR_, [scene](const auto& pair)
	{
		const auto& [_, actions] = pair;
		for (const Action& a : actions)
			if (a.scene == scene)
				return true;
		return false;
	});
}

/* -------------------------------------------------------------------------- */

const Actions::Map&        Actions::getAll_DEPR_() const { return m_actions_DEPR_; }
const std::vector<Action>& Actions::getAll() const { return m_actions; }

/* -------------------------------------------------------------------------- */

const Action* Actions::findAction(ID id) const
{
	if (!id.isValid())
		return nullptr;
	for (const Action& a : m_actions)
		if (a.id == id)
			return &a;
	return nullptr;
}

/* -------------------------------------------------------------------------- */

#if G_DEBUG_MODE

void Actions::debug() const
{
	puts("model::actions");
	for (const Action& a : m_actions)
		fmt::print("\t\tframe={}, ID={}, scene={}, channel={}, value=0x{}, prevId={}, nextId={}\n",
		    a.frame, a.id.getValue(), a.scene.getIndex(), a.channelId.getValue(), a.event.getRaw(),
		    a.prevId.getValue(), a.nextId.getValue());
}

#endif

/* -------------------------------------------------------------------------- */

Action Actions::rec(ID channelId, Scene scene, Frame frame, MidiEvent event)
{
	/* Skip duplicates. */

	if (exists(channelId, scene, frame, event))
		return {};

	Action a = actionFactory::makeAction({}, channelId, scene, frame, event);

	m_actions.push_back(a);
	sort();

	return a;
}

/* -------------------------------------------------------------------------- */

void Actions::rec(std::vector<Action>& actions, Scene scene)
{
	if (actions.size() == 0)
		return;

	for (const Action& a : actions)
		if (!exists(a.channelId, scene, a.frame, a.event))
			m_actions.push_back(a);

	sort();
}

/* -------------------------------------------------------------------------- */

void Actions::rec(ID channelId, Scene scene, Frame f1, Frame f2, MidiEvent e1, MidiEvent e2)
{
	Action a1 = actionFactory::makeAction({}, channelId, scene, f1, e1);
	Action a2 = actionFactory::makeAction({}, channelId, scene, f2, e2);
	m_actions.push_back(a1);
	m_actions.push_back(a2);

	Action* a1ptr = findAction(a1.id);
	Action* a2ptr = findAction(a2.id);

	assert(a1ptr != nullptr && a2ptr != nullptr);

	a1ptr->nextId = a2ptr->id;
	a2ptr->prevId = a1ptr->id;

	sort();
}

/* -------------------------------------------------------------------------- */

const std::vector<Action>* Actions::getActionsOnFrame(Frame frame) const
{
	if (m_actions_DEPR_.count(frame) == 0)
		return nullptr;
	return &m_actions_DEPR_.at(frame);
}

/* -------------------------------------------------------------------------- */

std::vector<Action> Actions::getActionsOnChannel(ID channelId, Scene scene) const
{
	std::vector<Action> out;
	for (const Action& a : m_actions)
		if (a.channelId == channelId && a.scene == scene)
			out.push_back(a);
	return out;
}

/* -------------------------------------------------------------------------- */

const Action* Actions::findAction(const Map& src, ID id) const
{
	if (!id.isValid())
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

Action* Actions::findAction(ID id)
{
	return const_cast<Action*>(std::as_const(*this).findAction(id));
}

/* -------------------------------------------------------------------------- */

void Actions::optimize(Map& map)
{
	for (auto it = map.cbegin(); it != map.cend();)
		it->second.size() == 0 ? it = map.erase(it) : ++it;
}

/* -------------------------------------------------------------------------- */

void Actions::sort()
{
	std::ranges::sort(m_actions, std::ranges::less{}, &Action::frame);
}

/* -------------------------------------------------------------------------- */

void Actions::removeIf(std::function<bool(const Action&)> f)
{
	for (auto& [frame, actions] : m_actions_DEPR_)
		actions.erase(std::remove_if(actions.begin(), actions.end(), f), actions.end());
	optimize(m_actions_DEPR_);
}

/* -------------------------------------------------------------------------- */

bool Actions::exists_DEPR_(ID channelId, Scene scene, Frame frame, const MidiEvent& event, const Map& target) const
{
	for (const auto& [_, actions] : target)
		for (const Action& a : actions)
			if (a.channelId == channelId && a.frame == frame && a.event.getRaw() == event.getRaw() && a.scene == scene)
				return true;
	return false;
}

/* -------------------------------------------------------------------------- */

bool Actions::exists_DEPR_(ID channelId, Scene scene, Frame frame, const MidiEvent& event) const
{
	return exists_DEPR_(channelId, scene, frame, event, m_actions_DEPR_);
}

/* -------------------------------------------------------------------------- */

bool Actions::exists(ID channelId, Scene scene, Frame frame, const MidiEvent& event, const std::vector<Action>& target) const
{
	for (const Action& a : target)
		if (a.channelId == channelId && a.frame == frame && a.event.getRaw() == event.getRaw() && a.scene == scene)
			return true;
	return false;
}

/* -------------------------------------------------------------------------- */

bool Actions::exists(ID channelId, Scene scene, Frame frame, const MidiEvent& event) const
{
	return exists(channelId, scene, frame, event, m_actions);
}
} // namespace giada::m::model
