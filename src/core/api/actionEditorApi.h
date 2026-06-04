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

#ifndef G_ACTION_EDITOR_API_H
#define G_ACTION_EDITOR_API_H

#include "src/core/model/actions.h"
#include "src/core/patch.h"
#include "src/core/types.h"
#include <vector>

namespace giada::m
{
class Engine;
class Sequencer;
class ActionManager;
class ActionEditorApi
{
public:
	ActionEditorApi(Engine&, Sequencer&, ActionManager&);

	std::vector<const Action*> getActionsOnChannel(ID channelId) const;
	const Action*              findAction(ID) const;

	void recordMidiAction(ID channelId, int note, float velocity, TickRange);
	void deleteMidiAction(ID channelId, ID actionId);
	void updateMidiAction(ID channelId, ID actionId, int note, float velocity, TickRange);
	void recordSampleAction(ID channelId, int type, Tick);
	void updateSampleAction(ID channelId, ID actionId, int type, Tick);
	void deleteSampleAction(ID actionId);
	void updateVelocity(ID actionId, float value);

private:
	Engine&        m_engine;
	Sequencer&     m_sequencer;
	ActionManager& m_actionManager;
};
} // namespace giada::m

#endif
