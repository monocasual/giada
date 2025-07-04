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

#include "src/core/api/actionEditorApi.h"
#include "src/core/actions/actionFactory.h"
#include "src/core/engine.h"
#include "src/core/sequencer.h"

namespace giada::m
{
ActionEditorApi::ActionEditorApi(Engine& e, Sequencer& s, ActionRecorder& ar)
: m_engine(e)
, m_sequencer(s)
, m_actionRecorder(ar)
{
}

/* -------------------------------------------------------------------------- */

std::vector<Action> ActionEditorApi::getActionsOnChannel(ID channelId) const
{
	return m_actionRecorder.getActionsOnChannel(channelId);
}

/* -------------------------------------------------------------------------- */

const Action* ActionEditorApi::findAction(ID id) const
{
	return m_actionRecorder.findAction(id);
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::recordMidiAction(ID channelId, int note, float velocity, Frame f1, Frame f2)
{
	m_actionRecorder.recordMidiAction(channelId, note, velocity, f1, f2, m_sequencer.getFramesInLoop());
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::deleteMidiAction(ID channelId, const Action& a)
{
	/* Send a note-off first in case we are deleting it in a middle of a
	key_on/key_off sequence. Only if it exists (i.e. it's not orphaned). */

	const Action* noteOff = m_actionRecorder.findAction(a.nextId);
	if (noteOff != nullptr)
		m_engine.getChannelsApi().sendMidi(channelId, noteOff->event);

	m_actionRecorder.deleteMidiAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::updateMidiAction(ID channelId, const Action& a, int note, float velocity, Frame f1, Frame f2)
{
	m_actionRecorder.updateMidiAction(channelId, a, note, velocity, f1, f2, m_sequencer.getFramesInLoop());
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::recordSampleAction(ID channelId, int type, Frame f1, Frame f2)
{
	m_actionRecorder.recordSampleAction(channelId, type, f1, f2, m_sequencer.getFramesInLoop());
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::updateSampleAction(ID channelId, const Action& a, int type, Frame f1, Frame f2)
{
	m_actionRecorder.updateSampleAction(channelId, a, type, f1, f2, m_sequencer.getFramesInLoop());
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::deleteSampleAction(ID channelId, const Action& a)
{
	m_actionRecorder.deleteSampleAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

#if 0

void ActionEditorApi::recordEnvelopeAction(ID channelId, Frame f, int value)
{
	m_actionRecorder.recordEnvelopeAction(channelId, f, value, m_sequencer.getFramesInLoop() - 1);
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::deleteEnvelopeAction(ID channelId, const Action& a)
{
	m_actionRecorder.deleteEnvelopeAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::updateEnvelopeAction(ID channelId, const Action& a, Frame f, int value)
{
	m_actionRecorder.updateEnvelopeAction(channelId, a, f, value, m_sequencer.getFramesInLoop() - 1);
}

#endif

/* -------------------------------------------------------------------------- */

void ActionEditorApi::updateVelocity(const Action& a, float value)
{
	m_actionRecorder.updateVelocity(a, value);
}
} // namespace giada::m
