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

#ifndef G_ACTION_EDITOR_ENGINE_H
#define G_ACTION_EDITOR_ENGINE_H

#include "core/actions/actions.h"
#include "core/patch.h"
#include "core/types.h"
#include <vector>

namespace giada::m
{
class Action;
class Engine;
class Sequencer;
class ActionRecorder;
class ActionEditorEngine
{
public:
	ActionEditorEngine(Engine&, model::Model&, Sequencer&, ActionRecorder&);

	std::vector<Action>        getActionsOnChannel(ID channelId) const;
	std::vector<Patch::Action> serializeActions() const;
	Actions::Map               deserializeActions(const std::vector<Patch::Action>& as) const;

	void recordMidiAction(ID channelId, int note, int velocity, Frame f1, Frame f2);
	void deleteMidiAction(ID channelId, const Action&);
	void updateMidiAction(ID channelId, const Action&, int note, int velocity, Frame f1, Frame f2);
	void recordSampleAction(ID channelId, int type, Frame f1, Frame f2);
	void updateSampleAction(ID channelId, const Action&, int type, Frame f1, Frame f2);
	void deleteSampleAction(ID channelId, const Action&);
	void recordEnvelopeAction(ID channelId, Frame f, int value);
	void deleteEnvelopeAction(ID channelId, const Action&);
	void updateEnvelopeAction(ID channelId, const Action&, Frame f, int value);
	void updateVelocity(const Action&, int value);

private:
	Engine&         m_engine;
	model::Model&   m_model;
	Sequencer&      m_sequencer;
	ActionRecorder& m_actionRecorder;
};
} // namespace giada::m

#endif
