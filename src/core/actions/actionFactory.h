/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_ACTION_FACTORY_H
#define G_ACTION_FACTORY_H

#include "core/actions/action.h"
#include "core/idManager.h"
#include "core/model/actions.h"
#include "core/patch.h"

namespace giada::m::actionFactory
{
/* reset
Resets internal ID generator. */

void reset();

/* makeAction
Makes a new action given some data. */

Action makeAction(ID id, ID channelId, Frame frame, MidiEvent e);
Action makeAction(const Patch::Action&);

/* getNewActionId
Returns a new action ID, internally generated. */

ID getNewActionId();

/* (de)serializeActions
Creates new Actions given the patch raw data and vice versa. */

model::Actions::Map        deserializeActions(const std::vector<Patch::Action>&);
std::vector<Patch::Action> serializeActions(const model::Actions::Map&);
} // namespace giada::m::actionFactory

#endif
