/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include "core/types.h"


namespace giada {
namespace m
{
namespace mixer
{
struct Event;
}
struct ChannelState;

/* SampleActionRecorder
Records actions for channels and optionally manages the 'read action' state ('R' 
button on Sample Channels). */

class SampleActionRecorder
{
public:

    SampleActionRecorder(ChannelState*, SamplePlayerState*);
    SampleActionRecorder(const SampleActionRecorder&, ChannelState* c=nullptr, 
        SamplePlayerState* sc=nullptr);

    void parse(const mixer::Event& e) const;

private:
    void record(int note) const;
    void onKeyPress() const;
    void onKeyRelease() const;
    void onFirstBeat() const;

    void toggleReadActions() const;
    void startReadActions() const;
    void stopReadActions(ChannelStatus curRecStatus) const;

    bool canRecord() const;

    ChannelState*      m_channelState;
    SamplePlayerState* m_samplePlayerState;
};
}} // giada::m::


#endif
