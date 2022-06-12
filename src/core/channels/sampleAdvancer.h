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

#ifndef G_CHANNEL_SAMPLE_ADVANCER_H
#define G_CHANNEL_SAMPLE_ADVANCER_H

#include "core/sequencer.h"

namespace giada::m
{
struct ChannelShared;
class SampleAdvancer final
{
public:
	void onLastFrame(ChannelShared&, bool seqIsRunning, bool natural, SamplePlayerMode, bool isLoop) const;
	void advance(ID channelId, ChannelShared&, const Sequencer::Event&, SamplePlayerMode, bool isLoop) const;

private:
	void rewind(ChannelShared&, Frame localFrame) const;
	void stop(ChannelShared&, Frame localFrame) const;
	void play(ChannelShared&, Frame localFrame) const;
	void onFirstBeat(ChannelShared&, Frame localFrame, bool isLoop) const;
	void onBar(ChannelShared&, Frame localFrame, SamplePlayerMode) const;
	void onNoteOn(ChannelShared&, Frame localFrame, SamplePlayerMode) const;
	void parseActions(ID channelId, ChannelShared&, const std::vector<Action>&, Frame localFrame, SamplePlayerMode) const;
};
} // namespace giada::m

#endif
