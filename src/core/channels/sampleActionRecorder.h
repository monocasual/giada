/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#ifndef G_CHANNEL_SAMPLE_ACTION_RECORDER_H
#define G_CHANNEL_SAMPLE_ACTION_RECORDER_H

#include "core/eventDispatcher.h"

namespace giada::m
{
class ActionRecorder;
class Sequencer;
class Channel;
class SampleActionRecorder final
{
public:
	SampleActionRecorder(ActionRecorder&, Sequencer&);

	void react(Channel&, const EventDispatcher::Event&, bool treatRecsAsLoops,
	    bool seqIsRunning, bool canRecordActions) const;

private:
	void record(Channel&, int note) const;
	void onKeyPress(Channel&) const;
	void startReadActions(Channel&, bool treatRecsAsLoops) const;
	void stopReadActions(Channel&, ChannelStatus, bool treatRecsAsLoops, bool seqIsRunning) const;
	void toggleReadActions(Channel&, bool treatRecsAsLoops, bool seqIsRunning) const;
	void killReadActions(Channel& ch) const;

	ActionRecorder* m_actionRecorder;
	Sequencer*      m_sequencer;
};

} // namespace giada::m

#endif
