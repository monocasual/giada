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


#include <algorithm>
#include <cassert>
#include "core/channels/channel.h"
#include "core/channels/state.h"
#include "core/wave.h"
#include "core/clock.h"
#include "samplePlayer.h"


namespace giada {
namespace m 
{
SamplePlayer::SamplePlayer(ChannelState* c)
: state             (std::make_unique<SamplePlayerState>())
, m_waveId          (0)
, m_sampleController(c, state.get())
, m_channelState    (c)
{
}


/* -------------------------------------------------------------------------- */


SamplePlayer::SamplePlayer(const SamplePlayer& o, ChannelState* c)
: state             (std::make_unique<SamplePlayerState>(*o.state))
, m_waveId          (o.m_waveId)
, m_waveReader      (o.m_waveReader)
, m_sampleController(o.m_sampleController, c, state.get())
, m_channelState    (c)
{
}


/* -------------------------------------------------------------------------- */


SamplePlayer::SamplePlayer(const patch::Channel& p, ChannelState* c)
: state             (std::make_unique<SamplePlayerState>(p))
, m_waveId          (p.waveId)
, m_sampleController(c, state.get())
, m_channelState    (c)
{
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::parse(const mixer::Event& e) const
{
    if (e.type == mixer::EventType::CHANNEL_PITCH)
        state->pitch.store(e.action.event.getVelocityFloat());

    if (hasWave())
        m_sampleController.parse(e);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::advance(Frame bufferSize) const
{
    m_sampleController.advance(bufferSize);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::render(AudioBuffer& out) const
{
    assert(m_channelState != nullptr);

    if (m_waveReader.wave == nullptr || !m_channelState->isPlaying())
        return;

    /* Advance SampleController: this is needed for quantization. */

    Frame begin   = state->begin.load();
    Frame end     = state->end.load();
    Frame tracker = state->tracker.load();
    float pitch   = state->pitch.load();
    Frame used    = 0;

    /* Audio data is temporarily stored to the working audio buffer. */

    AudioBuffer& buffer = m_channelState->buffer;

    /* Adjust tracker in case someone has changed the begin/end points in the
    meantime. */
    
    if (tracker < begin || tracker >= end)
        tracker = begin;

    /* If rewinding, fill the tail first, then reset the tracker to the begin
    point. The rest is performed as usual. */

    if (state->rewinding) {
		if (tracker < end)
            m_waveReader.fill(buffer, tracker, 0, pitch);
        state->rewinding = false;
		tracker = begin;
    }

    used     = m_waveReader.fill(buffer, tracker, state->offset, pitch);
    tracker += used;

G_DEBUG ("block=[" << tracker - used << ", " << tracker << ")" << 
         ", used=" << used << ", range=[" << begin << ", " << end << ")" <<
         ", offset=" << state->offset << ", globalFrame=" << clock::getCurrentFrame());

    if (tracker >= end) {
G_DEBUG ("last frame tracker=" << tracker);
        tracker = begin;
        m_sampleController.onLastFrame();
        if (shouldLoop()) {
            Frame offset = std::min(static_cast<Frame>(used / pitch), buffer.countFrames() - 1);
            tracker += m_waveReader.fill(buffer, tracker, offset, pitch);
        }
    }

    state->offset = 0;
    state->tracker.store(tracker);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::loadWave(const Wave* w)
{
    m_waveReader.wave = w;

    state->tracker.store(0);
    state->shift.store(0);
    state->begin.store(0);

    if (w != nullptr) {
        m_waveId = w->id;
        m_channelState->playStatus.store(ChannelStatus::OFF);
        m_channelState->name = w->getBasename(/*ext=*/false);
        state->end.store(w->getSize() - 1);
    }
    else {
        m_waveId = 0;
        m_channelState->playStatus.store(ChannelStatus::EMPTY);
        m_channelState->name = "";
        state->end.store(0);
    }
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::setWave(const Wave& w)
{
    m_waveReader.wave = &w;
    m_waveId = w.id;
}


void SamplePlayer::setInvalidWave()
{
    m_waveReader.wave = nullptr;
    m_waveId = 0;
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::kickIn(Frame f)
{
    assert(hasWave());
    
	state->tracker.store(f);
	m_channelState->playStatus.store(ChannelStatus::PLAY);    
}


/* -------------------------------------------------------------------------- */


bool SamplePlayer::shouldLoop() const
{
    ChannelStatus    playStatus = m_channelState->playStatus.load();
    SamplePlayerMode mode       = state->mode.load();
    
    return (mode == SamplePlayerMode::LOOP_BASIC  || 
            mode == SamplePlayerMode::LOOP_REPEAT || 
            mode == SamplePlayerMode::SINGLE_ENDLESS) && playStatus == ChannelStatus::PLAY;
}


/* -------------------------------------------------------------------------- */


bool SamplePlayer::hasWave() const        { return m_waveReader.wave != nullptr; }
bool SamplePlayer::hasLogicalWave() const { return hasWave() && m_waveReader.wave->isLogical(); }
bool SamplePlayer::hasEditedWave() const  { return hasWave() && m_waveReader.wave->isEdited(); }


/* -------------------------------------------------------------------------- */


ID SamplePlayer::getWaveId() const
{
    return m_waveId;
}


/* -------------------------------------------------------------------------- */


Frame SamplePlayer::getWaveSize() const
{
    return hasWave() ? m_waveReader.wave->getSize() : 0;
}
}} // giada::m::
