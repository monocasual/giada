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

#ifndef G_RENDERING_SAMPLE_REACTIONS_H
#define G_RENDERING_SAMPLE_REACTIONS_H

#include "src/core/types.h"
#include "src/types.h"

namespace giada::m
{
struct ChannelShared;
class ActionRecorder;
} // namespace giada::m

namespace giada::m::rendering
{
void recordSampleKeyPress(ID channelId, std::size_t scene, ChannelShared&, Frame currentFrameQuantized, SamplePlayerMode, ActionRecorder&);
void recordSampleKeyRelease(ID channelId, std::size_t scene, Frame currentFrameQuantized, ActionRecorder&);
void recordSampleKeyKill(ID channelId, std::size_t scene, Frame currentFrameQuantized, ActionRecorder&);
void killSampleReadActions(ChannelShared&);
void toggleSampleReadActions(ChannelShared&, bool treatRecsAsLoops, bool seqIsRunning);

/* [...]SampleChannel
Actions manually performed on a Sample channel. */

void stopSampleChannelBySeq(ChannelShared&, bool chansStopOnSeqHalt, bool isLoop);
void stopSampleChannel(ChannelShared&, Frame localFrame);
void pressSampleChannel(ID channelId, ChannelShared&, SamplePlayerMode, float velocity, bool canQuantize, bool isLoop, bool velocityAsVol);
void releaseSampleChannel(ChannelShared&, SamplePlayerMode);
void killSampleChannel(ChannelShared&, SamplePlayerMode);
void rewindSampleChannel(ChannelShared&, Frame localFrame);
void playSampleChannel(ChannelShared&, Frame localFrame);

} // namespace giada::m::rendering

#endif
