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

#ifndef G_CHANNEL_SAMPLE_ACTION_RECORDER_H
#define G_CHANNEL_SAMPLE_ACTION_RECORDER_H

#include "core/eventDispatcher.h"
#include "core/types.h"

namespace giada::m
{
class ActionRecorder;
struct ChannelShared;
class SampleActionRecorder final
{
public:
	SampleActionRecorder(ActionRecorder&);

	void killReadActions(ChannelShared&) const;
	void toggleReadActions(ChannelShared&, bool treatRecsAsLoops, bool seqIsRunning) const;
	void keyPress(ID channelId, ChannelShared&, Frame currentFrameQuantized, SamplePlayerMode, bool& hasActions) const;
	void keyRelease(ID channelId, bool canRecordActions, Frame currentFrameQuantized, SamplePlayerMode, bool& hasActions) const;
	void keyKill(ID channelId, bool canRecordActions, Frame currentFrameQuantized, SamplePlayerMode, bool& hasActions) const;

private:
	void record(ID channelId, int note, Frame currentFrameQuantized, bool& hasActions) const;
	void startReadActions(ChannelShared&, bool treatRecsAsLoops) const;
	void stopReadActions(ChannelShared&, ChannelStatus, bool treatRecsAsLoops, bool seqIsRunning) const;

	ActionRecorder* m_actionRecorder;
};

} // namespace giada::m

#endif
