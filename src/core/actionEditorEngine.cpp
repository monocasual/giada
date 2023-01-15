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

#include "core/actionEditorEngine.h"
#include "core/actions/actionFactory.h"
#include "core/engine.h"
#include "core/sequencer.h"

namespace giada::m
{
ActionEditorEngine::ActionEditorEngine(Engine& e, Sequencer& s, ActionRecorder& ar)
: m_engine(e)
, m_sequencer(s)
, m_actionRecorder(ar)
{
}

/* -------------------------------------------------------------------------- */

std::vector<Action> ActionEditorEngine::getActionsOnChannel(ID channelId) const
{
	return m_actionRecorder.getActionsOnChannel(channelId);
}

/* -------------------------------------------------------------------------- */

std::vector<Patch::Action> ActionEditorEngine::serializeActions() const
{
	return actionFactory::serializeActions(m_engine.model.getAllShared<Actions::Map>());
}

Actions::Map ActionEditorEngine::deserializeActions(const std::vector<Patch::Action>& as) const
{
	return actionFactory::deserializeActions(as);
}

/* -------------------------------------------------------------------------- */

void ActionEditorEngine::recordMidiAction(ID channelId, int note, int velocity, Frame f1, Frame f2)
{
	m_actionRecorder.recordMidiAction(channelId, note, velocity, f1, f2, m_sequencer.getFramesInLoop());
}

/* -------------------------------------------------------------------------- */

void ActionEditorEngine::deleteMidiAction(ID channelId, const Action& a)
{
	/* Send a note-off first in case we are deleting it in a middle of a 
	key_on/key_off sequence. */

	m_engine.getChannelsEngine().sendMidi(channelId, a.next->event);
	m_actionRecorder.deleteMidiAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

void ActionEditorEngine::updateMidiAction(ID channelId, const Action& a, int note, int velocity, Frame f1, Frame f2)
{
	m_actionRecorder.updateMidiAction(channelId, a, note, velocity, f1, f2, m_sequencer.getFramesInLoop());
}

/* -------------------------------------------------------------------------- */

void ActionEditorEngine::recordSampleAction(ID channelId, int type, Frame f1, Frame f2)
{
	m_actionRecorder.recordSampleAction(channelId, type, f1, f2);
}

/* -------------------------------------------------------------------------- */

void ActionEditorEngine::updateSampleAction(ID channelId, const Action& a, int type, Frame f1, Frame f2)
{
	m_actionRecorder.updateSampleAction(channelId, a, type, f1, f2);
}

/* -------------------------------------------------------------------------- */

void ActionEditorEngine::deleteSampleAction(ID channelId, const Action& a)
{
	m_actionRecorder.deleteSampleAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

void ActionEditorEngine::recordEnvelopeAction(ID channelId, Frame f, int value)
{
	m_actionRecorder.recordEnvelopeAction(channelId, f, value, m_sequencer.getFramesInLoop() - 1);
}

/* -------------------------------------------------------------------------- */

void ActionEditorEngine::deleteEnvelopeAction(ID channelId, const Action& a)
{
	m_actionRecorder.deleteEnvelopeAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

void ActionEditorEngine::updateEnvelopeAction(ID channelId, const Action& a, Frame f, int value)
{
	m_actionRecorder.updateEnvelopeAction(channelId, a, f, value, m_sequencer.getFramesInLoop() - 1);
}

/* -------------------------------------------------------------------------- */

void ActionEditorEngine::updateVelocity(const Action& a, int value)
{
	m_actionRecorder.updateVelocity(a, value);
}
} // namespace giada::m
