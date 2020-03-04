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


#ifndef G_CHANNEL_SAMPLE_PLAYER_H
#define G_CHANNEL_SAMPLE_PLAYER_H


#include "core/types.h"
#include "core/const.h"
#include "core/mixer.h" // TODO - forward declare
#include "core/audioBuffer.h" // TODO - forward declare
#include "core/channels/waveReader.h"
#include "core/channels/sampleController.h"


namespace giada {
namespace m
{
class  Wave;
struct SamplePlayerState;
class SamplePlayer
{
public:

    SamplePlayer(ChannelState*);
    SamplePlayer(const patch::Channel& p, ChannelState*);
    SamplePlayer(const SamplePlayer&, ChannelState* c=nullptr);

    void parse(const mixer::Event& e) const;
    void advance(Frame bufferSize) const;
    void render(AudioBuffer& out) const;

    bool hasWave() const;
    bool hasLogicalWave() const;
    bool hasEditedWave() const;
    ID getWaveId() const;
    Frame getWaveSize() const;

    /* loadWave
    Loads Wave 'w' into this channel and sets it up (name, markers, ...). */

    void loadWave(const Wave* w);
    
    /* setWave
    Just sets the pointer to a Wave object. Used during de-serialization. */

    void setWave(const Wave& w);

    /* setInvalidWave
    Same as setWave(nullptr) plus the invalid ID (i.e. 0). */
    
    void setInvalidWave(); 

    /* kickIn
    Starts the player right away at frame 'f'. Used when launching a loop after
    being live recorded. */
    
    void kickIn(Frame f);


    /* state
    Pointer to mutable SamplePlayerState state. */

    std::unique_ptr<SamplePlayerState> state;

private:

    bool shouldLoop() const;

    ID m_waveId;

    /* m_waveReader
    Used to read data from Wave and fill incoming buffer. */

    WaveReader m_waveReader;

    /* m_sampleController
    Managers events for this Sample Player. */

    SampleController m_sampleController;

    /* m_channelState
    Pointer to Channel state. Needed to alter the playStatus status when the
    sample is over. */

    ChannelState* m_channelState;
};
}} // giada::m::


#endif
