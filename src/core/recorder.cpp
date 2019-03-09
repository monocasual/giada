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
#include "core/model/data.h"
#include "core/channels/channel.h"
#include "core/action.h"
#include "core/recorder.h"


namespace giada {
namespace m {
namespace recorder
{
namespace
{
bool active_   = false;
ID   actionId_ = 0;


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
	std::shared_ptr<model::Layout> data = model::cloneLayout();
	for (auto& kv : data->actions) {
		std::vector<Action>& as = kv.second;
		as.erase(std::remove_if(as.begin(), as.end(), f), as.end());
	}
	optimize_(data->actions);
	updateMapPointers(data->actions);
	model::swapLayout(data);
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init()
{
	active_   = false;
	actionId_ = 0;
	clearAll();
}


/* -------------------------------------------------------------------------- */


void debug(const ActionMap& map)
{
	int total = 0;
	puts("-------------");
	for (auto& kv : map) {
		printf("frame: %d\n", kv.first);
		for (const Action& a : kv.second) {
			total++;
			printf(" (%p) - id=%d, frame=%d, channel=%ld, value=0x%X, prevId=%d, prev=%p, nextId=%d, next=%p\n", 
				(void*) &a, a.id, a.frame, a.channelId, a.event.getRaw(), a.prevId, (void*) a.prev, a.nextId, (void*) a.next);	
		}
	}
	printf("TOTAL: %d\n", total);
	puts("-------------");
}


/* -------------------------------------------------------------------------- */


void clearAll()
{
	std::shared_ptr<model::Layout> data = model::cloneLayout();
	data->actions.clear();
	model::swapLayout(data);
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
	std::shared_ptr<model::Layout> data = model::cloneLayout();

	/* Remove all existing actions: let's start from scratch. */

	data->actions.clear();

	/* Copy all existing actions in local data by cloning them, with just a
	difference: they have a new frame value. */

	for (const auto& kv : model::getLayout()->actions) {
		Frame frame = f(kv.first);
		for (const Action& a : kv.second) {
			Action copy = a;
			copy.frame = frame;
			data->actions[frame].push_back(copy);
		}
		gu_log("[recorder::updateKeyFrames] %d -> %d\n", kv.first, frame);		
	}

	updateMapPointers(data->actions);
	model::swapLayout(data);
}


/* -------------------------------------------------------------------------- */


void updateEvent(ID id, MidiEvent e)
{
	std::shared_ptr<model::Layout> data = model::cloneLayout();
	findAction_(data->actions, id)->event = e;
	model::swapLayout(data);
}


/* -------------------------------------------------------------------------- */


void updateSiblings(ID id, ID prevId, ID nextId)
{
	std::shared_ptr<model::Layout> data = model::cloneLayout();

	Action* pcurr = findAction_(data->actions, id);
	Action* pprev = findAction_(data->actions, prevId);
	Action* pnext = findAction_(data->actions, nextId);

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

	model::swapLayout(data);
}


/* -------------------------------------------------------------------------- */


bool hasActions(ID channelId, int type)
{
	for (const auto& kv : m::model::getLayout()->actions)
		for (const Action& a : kv.second)
			if (a.channelId == channelId && (type == 0 || type == a.event.getStatus()))
				return true;
	return false;
}


/* -------------------------------------------------------------------------- */


bool isActive() { return active_; }
void enable()   { active_ = true; }
void disable()  { active_ = false; }


/* -------------------------------------------------------------------------- */


Action makeAction(ID id, ID channelId, Frame frame, MidiEvent e)
{
	return Action{id, channelId, frame, e, -1, -1};
}


/* -------------------------------------------------------------------------- */


Action rec(ID channelId, Frame frame, MidiEvent event)
{
	/* If key frame doesn't exist yet, the [] operator in std::map is smart 
	enough to insert a new item first. No plug-in data for now. */

	std::shared_ptr<model::Layout> data = model::cloneLayout();
	data->actions[frame].push_back(makeAction(++actionId_, channelId, frame, event));

	updateMapPointers(data->actions);
	model::swapLayout(data);

	return model::getLayout()->actions[frame].back();
}


/* -------------------------------------------------------------------------- */


void rec(std::vector<Action>& as)
{
	if (as.size() == 0)
		return;

	/* Generate new action ID and fix next and prev IDs. */

	for (Action& a : as) {
		int id = a.id;
		a.id = ++actionId_;
		for (Action& aa : as) {
			if (aa.prevId == id) aa.prevId = a.id;
			if (aa.nextId == id) aa.nextId = a.id;
		}
	}

	std::shared_ptr<model::Layout> data = model::cloneLayout();
	for (const Action& a : as)
		data->actions[a.frame].push_back(a);
	updateMapPointers(data->actions);
	model::swapLayout(data);
}


/* -------------------------------------------------------------------------- */


void rec(ID channelId, Frame f1, Frame f2, MidiEvent e1, MidiEvent e2)
{
	std::shared_ptr<model::Layout> data = model::cloneLayout();
	data->actions[f1].push_back(makeAction(++actionId_, channelId, f1, e1));
	data->actions[f2].push_back(makeAction(++actionId_, channelId, f2, e2));

	Action* a1 = findAction_(data->actions, data->actions[f1].back().id);
	Action* a2 = findAction_(data->actions, data->actions[f2].back().id);
	a1->nextId = a2->id;
	a2->prevId = a1->id;

	updateMapPointers(data->actions);
	model::swapLayout(data);
}


/* -------------------------------------------------------------------------- */


const std::vector<Action>* getActionsOnFrame(Frame frame)
{
	return m::model::getLayout()->actions.count(frame) ? &m::model::getLayout()->actions[frame] : nullptr;
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


int getLatestActionId() { return actionId_; }


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
	for (auto& kv : m::model::getLayout()->actions)
		for (const Action& action : kv.second)
			f(action);
}

}}}; // giada::m::recorder::
