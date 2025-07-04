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

#include "src/core/actions/actionFactory.h"
#include "src/core/midiEvent.h"
#include <cassert>

namespace giada::m::actionFactory
{
namespace
{
IdManager actionId_;
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void reset()
{
	actionId_ = IdManager();
}

/* -------------------------------------------------------------------------- */

Action makeAction(ID id, ID channelId, Frame frame, MidiEvent e)
{
	Action out{actionId_.generate(id), channelId, frame, e, -1, -1};
	actionId_.set(id);
	return out;
}

Action makeAction(const Patch::Action& a)
{
	actionId_.set(a.id);
	return Action{a.id, a.channelId, a.frame,
	    MidiEvent::makeFromRaw(a.event, /*numBytes=*/3), -1, -1, a.prevId, a.nextId};
}

/* -------------------------------------------------------------------------- */

ID getNewActionId()
{
	return actionId_.generate();
}

/* -------------------------------------------------------------------------- */

model::Actions::Map deserializeActions(const std::vector<Patch::Action>& pactions)
{
	model::Actions::Map out;
	for (const Patch::Action& paction : pactions)
		out[paction.frame].push_back(makeAction(paction));
	return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Patch::Action> serializeActions(const model::Actions::Map& actions)
{
	std::vector<Patch::Action> out;
	for (const auto& kv : actions) // TODO - const auto& [_, actionsInFrame]
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

} // namespace giada::m::actionFactory
