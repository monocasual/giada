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
class ActionRecorder;
class ActionEditorApi
{
public:
	ActionEditorApi(Engine&, Sequencer&, ActionRecorder&);

	std::vector<Action> getActionsOnChannel(ID channelId) const;
	const Action*       findAction(ID) const;

	void recordMidiAction(ID channelId, int note, float velocity, Frame f1, Frame f2);
	void deleteMidiAction(ID channelId, const Action&);
	void updateMidiAction(ID channelId, const Action&, int note, float velocity, Frame f1, Frame f2);
	void recordSampleAction(ID channelId, int type, Frame f1, Frame f2);
	void updateSampleAction(ID channelId, const Action&, int type, Frame f1, Frame f2);
	void deleteSampleAction(ID channelId, const Action&);
#if 0
	void recordEnvelopeAction(ID channelId, Frame f, int value);
	void deleteEnvelopeAction(ID channelId, const Action&);
	void updateEnvelopeAction(ID channelId, const Action&, Frame f, int value);
#endif
	void updateVelocity(const Action&, float value);

private:
	Engine&         m_engine;
	Sequencer&      m_sequencer;
	ActionRecorder& m_actionRecorder;
};
} // namespace giada::m

#endif
