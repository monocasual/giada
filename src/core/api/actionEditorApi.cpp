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

#include "src/core/api/actionEditorApi.h"
#include "src/core/actions/actionFactory.h"
#include "src/core/engine.h"
#include "src/core/sequencer.h"

namespace giada::m
{
ActionEditorApi::ActionEditorApi(Engine& e, Sequencer& s, ActionManager& ar)
: m_engine(e)
, m_sequencer(s)
, m_actionManager(ar)
{
}

/* -------------------------------------------------------------------------- */

std::vector<Action> ActionEditorApi::getActionsOnChannel(ID channelId) const
{
	return m_actionManager.getActionsOnChannel(channelId, m_sequencer.getCurrentScene());
}

/* -------------------------------------------------------------------------- */

const Action* ActionEditorApi::findAction(ID id) const
{
	return m_actionManager.findAction(id);
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::recordMidiAction(ID channelId, int note, float velocity, TickRange range)
{
	m_actionManager.recordMidiAction(channelId, m_sequencer.getCurrentScene(),
	    note, velocity, range, m_sequencer.getTicksInLoop());
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::deleteMidiAction(ID channelId, const Action& a)
{
	/* Send a note-off first in case we are deleting it in a middle of a
	key_on/key_off sequence. Only if it exists (i.e. it's not orphaned). */

	const Action* noteOff = m_actionManager.findAction(a.nextId);
	if (noteOff != nullptr)
		m_engine.getChannelsApi().sendMidi(channelId, noteOff->event);

	m_actionManager.deleteMidiAction(a);
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::updateMidiAction(ID channelId, const Action& a, int note,
    float velocity, TickRange range)
{
	m_actionManager.updateMidiAction(channelId, m_sequencer.getCurrentScene(), a,
	    note, velocity, range, m_sequencer.getTicksInLoop());
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::recordSampleAction(ID channelId, int type, Frame f1, Frame f2)
{
	m_actionManager.recordSampleAction_DEPR_(channelId, m_sequencer.getCurrentScene(), type, f1, f2, m_sequencer.getFramesInLoop());
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::updateSampleAction(ID channelId, const Action& a, int type, Frame f1, Frame f2)
{
	m_actionManager.updateSampleAction(channelId, m_sequencer.getCurrentScene(), a, type, f1, f2, m_sequencer.getFramesInLoop());
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::deleteSampleAction(const Action& a)
{
	m_actionManager.deleteSampleAction(a);
}

/* -------------------------------------------------------------------------- */

void ActionEditorApi::updateVelocity(const Action& a, float value)
{
	m_actionManager.updateVelocity(a, value);
}
} // namespace giada::m
