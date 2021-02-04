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


#ifndef G_CHANNEL_SAMPLE_CONTROLLER_H
#define G_CHANNEL_SAMPLE_CONTROLLER_H


#include "core/types.h"


namespace giada {
namespace m
{
namespace mixer
{
struct Event;
}
struct SamplePlayerState;
class SampleController
{
public:

    SampleController(ChannelState*, SamplePlayerState*);
    SampleController(const SampleController&, ChannelState* c=nullptr, SamplePlayerState* s=nullptr);

    void parse(const mixer::Event& e) const;
    void onLastFrame() const;
    void advance(Frame bufferSize) const;

private:

    void press(Frame localFrame, int velocity, bool manual) const;
    void release(Frame localFrame) const;
    void kill(Frame localFrame) const;
    void rewind(Frame localFrame) const;

    ChannelStatus pressWhileOff(Frame localFrame, int velocity, bool isLoop, bool manual) const;
    ChannelStatus pressWhilePlay(Frame localFrame, SamplePlayerMode mode, bool isLoop, bool manual) const;
    void toggleReadActions() const;

    void onBar(Frame localFrame) const;
    void onFirstBeat(Frame localFrame) const;
    void onStopBySeq() const;
    void parseAction(const Action& a, Frame localFrame) const;
    
    ChannelState*      m_channelState;
    SamplePlayerState* m_samplePlayerState;
};
}} // giada::m::


#endif
