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


#include <memory>
#include <algorithm>
#include <cassert>
#include "utils/log.h"
#include "core/model/model.h"
#include "core/channels/channel.h"
#include "core/action.h"
#include "core/idManager.h"
#include "core/recorder.h"


namespace giada {
namespace m {
namespace recorder
{
namespace
{
IdManager actionId_;


/* -------------------------------------------------------------------------- */


Action* findAction_(ActionMap& src, ID id)
{
	for (auto& kv : src)
		for (Action& a : kv.second)
			if (a.id == id)
				return &a;
	assert(false);
	return nullptr;	
}


/* -------------------------------------------------------------------------- */

/* optimize
Removes frames without actions. */

void optimize_(ActionMap& map)
{
	for (auto it = map.cbegin(); it != map.cend();)
		it->second.size() == 0 ? it = map.erase(it) : ++it;
}


/* -------------------------------------------------------------------------- */


void removeIf_(std::function<bool(const Action&)> f)
{
	model::onSwap(model::actions, [&](model::Actions& a)
	{
		for (auto& kv : a.map) {
			std::vector<Action>& as = kv.second;
			as.erase(std::remove_if(as.begin(), as.end(), f), as.end());
		}
		optimize_(a.map);
		updateMapPointers(a.map);
	});
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init()
{
	clearAll();
}


/* -------------------------------------------------------------------------- */


void clearAll()
{
	model::onSwap(model::actions, [&](model::Actions& a)
	{
		a.map.clear();
	});
}


/* -------------------------------------------------------------------------- */


void clearChannel(ID channelId)
{
	removeIf_([=](const Action& a) { return a.channelId == channelId; });
}


/* -------------------------------------------------------------------------- */


void clearActions(ID channelId, int type)
{
	removeIf_([=](const Action& a)
	{ 
		return a.channelId == channelId && a.event.getStatus() == type;
	});
}


/* -------------------------------------------------------------------------- */


void deleteAction(ID id)
{
	removeIf_([=](const Action& a) { return a.id == id; });
}


void deleteAction(ID currId, ID nextId)
{
	removeIf_([=](const Action& a) { return a.id == currId || a.id == nextId; });
}


/* -------------------------------------------------------------------------- */


void updateKeyFrames(std::function<Frame(Frame old)> f)
{
	std::unique_ptr<model::Actions> ma = model::actions.clone();
	
	/* Remove all existing actions: let's start from scratch. */

	ma->map.clear();

	/* Copy all existing actions in local data by cloning them, with just a
	difference: they have a new frame value. */

	{
		model::ActionsLock lock(model::actions);

		for (const auto& kv : model::actions.get()->map) {
			Frame frame = f(kv.first);
			for (const Action& a : kv.second) {
				Action copy = a;
				copy.frame = frame;
				ma->map[frame].push_back(copy);
			}
			gu_log("[recorder::updateKeyFrames] %d -> %d\n", kv.first, frame);
		}
	}

	updateMapPointers(ma->map);

	model::actions.swap(std::move(ma));
}


/* -------------------------------------------------------------------------- */


void updateEvent(ID id, MidiEvent e)
{
	model::onSwap(model::actions, [&](model::Actions& a)
	{
		findAction_(a.map, id)->event = e;
	});
}


/* -------------------------------------------------------------------------- */


void updateSiblings(ID id, ID prevId, ID nextId)
{
	model::onSwap(model::actions, [&](model::Actions& a)
	{
		Action* pcurr = findAction_(a.map, id);
		Action* pprev = findAction_(a.map, prevId);
		Action* pnext = findAction_(a.map, nextId);

		pcurr->prev   = pprev;
		pcurr->prevId = pprev->id;
		pcurr->next   = pnext;
		pcurr->nextId = pnext->id;

		if (pprev != nullptr) {
			pprev->next   = pcurr;
			pprev->nextId = pcurr->id;
		}
		if (pnext != nullptr) {
			pnext->prev   = pcurr;
			pnext->prevId = pcurr->id;
		}
	});
}


/* -------------------------------------------------------------------------- */


bool hasActions(ID channelId, int type)
{
	model::ActionsLock lock(model::actions);
	
	for (const auto& kv : model::actions.get()->map)
		for (const Action& a : kv.second)
			if (a.channelId == channelId && (type == 0 || type == a.event.getStatus()))
				return true;
	return false;
}


/* -------------------------------------------------------------------------- */


Action makeAction(ID id, ID channelId, Frame frame, MidiEvent e)
{
	Action out {actionId_.get(id), channelId, frame, e, -1, -1};
	actionId_.set(id);
	return out;
}


/* -------------------------------------------------------------------------- */


Action rec(ID channelId, Frame frame, MidiEvent event)
{
	Action a = makeAction(0, channelId, frame, event);
	
	/* If key frame doesn't exist yet, the [] operator in std::map is smart 
	enough to insert a new item first. No plug-in data for now. */

	model::onSwap(model::actions, [&](model::Actions& mas)
	{
		mas.map[frame].push_back(a);
		updateMapPointers(mas.map);
	});

	return a;
}


/* -------------------------------------------------------------------------- */


void rec(std::vector<Action>& as)
{
	if (as.size() == 0)
		return;

	/* Generate new action ID and fix next and prev IDs. */

	for (Action& a : as) {
		int id = a.id;
		a.id = actionId_.get();
		for (Action& aa : as) {
			if (aa.prevId == id) aa.prevId = a.id;
			if (aa.nextId == id) aa.nextId = a.id;
		}
	}
	
	model::onSwap(model::actions, [&](model::Actions& mas)
	{
		for (const Action& a : as)
			mas.map[a.frame].push_back(a);
		updateMapPointers(mas.map);
	});
}


/* -------------------------------------------------------------------------- */


void rec(ID channelId, Frame f1, Frame f2, MidiEvent e1, MidiEvent e2)
{
	model::onSwap(model::actions, [&](model::Actions& mas)
	{
		mas.map[f1].push_back(makeAction(0, channelId, f1, e1));
		mas.map[f2].push_back(makeAction(0, channelId, f2, e2));

		Action* a1 = findAction_(mas.map, mas.map[f1].back().id);
		Action* a2 = findAction_(mas.map, mas.map[f2].back().id);
		a1->nextId = a2->id;
		a2->prevId = a1->id;

		updateMapPointers(mas.map);
	});
}


/* -------------------------------------------------------------------------- */


const std::vector<Action>* getActionsOnFrame(Frame frame)
{
	model::ActionsLock lock(model::actions);
	
	if (model::actions.get()->map.count(frame) == 0)
		return nullptr;
	return &model::actions.get()->map.at(frame);
}


/* -------------------------------------------------------------------------- */


Action getClosestAction(ID channelId, Frame f, int type)
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


int getLatestActionId() { assert(false); /*return actionId_;*/ }


/* -------------------------------------------------------------------------- */


std::vector<Action> getActionsOnChannel(ID channelId)
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


void updateMapPointers(ActionMap& src)
{
	for (auto& kv : src) {
		for (Action& action : kv.second) {
			if (action.nextId != 0)
				action.next = findAction_(src, action.nextId);
			if (action.prevId != 0)
				action.prev = findAction_(src, action.prevId);
		}
	}
}


/* -------------------------------------------------------------------------- */


void forEachAction(std::function<void(const Action&)> f)
{
	model::ActionsLock lock(model::actions);
	
	for (auto& kv : model::actions.get()->map)
		for (const Action& action : kv.second)
			f(action);
}
}}}; // giada::m::recorder::
