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

#ifndef G_REC_MANAGER_H
#define G_REC_MANAGER_H

#include "core/eventDispatcher.h"
#include "core/types.h"
#include "core/weakAtomic.h"

namespace giada::m::model
{
class Model;
}

namespace giada::m
{
class ActionRecorder;
class ChannelManager;
class Mixer;
class Sequencer;
class Recorder final
{
public:
	struct InputRecData
	{
		RecTriggerMode recTriggerMode;
		InputRecMode   inputMode;
	};

	Recorder(model::Model&, Sequencer&, ChannelManager&, Mixer&, ActionRecorder&);

	bool isRecordingActions() const;
	bool isRecordingInput() const;

	/* canEnableRecOnSignal
    True if rec-on-signal can be enabled: can't set it while sequencer is 
    running, in order to prevent mistakes while live recording. */

	bool canEnableRecOnSignal() const;

	/* canEnableFreeInputRec
    True if free loop-length can be enabled: Can't set it if there's already a 
    filled Sample Channel in the current project. */

	bool canEnableFreeInputRec() const;

	/* canRecordActions
	True if actions are recordable right now. */

	bool canRecordActions() const;

	/* react
	Reacts to live events coming from the EventDispatcher (human events). */

	void react(const EventDispatcher::EventBuffer&, int sampleRate);

	void prepareActionRec(RecTriggerMode);
	void startActionRecOnCallback();
	void stopActionRec();

	bool prepareInputRec(RecTriggerMode, InputRecMode);
	void startInputRec();
	void startInputRecOnCallback();
	void stopInputRec(InputRecMode, int sampleRate);

private:
	model::Model&   m_model;
	Sequencer&      m_sequencer;
	ChannelManager& m_channelManager;
	Mixer&          m_mixer;
	ActionRecorder& m_actionRecorder;

	WeakAtomic<bool> m_isRecordingActions;
	WeakAtomic<bool> m_isRecordingInput;
};
} // namespace giada::m

#endif
