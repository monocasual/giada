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


#include "core/channels/state.h"
#include "core/mixer.h"
#include "core/kernelMidi.h"
#include "core/midiMapConf.h"
#include "midiLighter.h"


namespace giada {
namespace m 
{
MidiLighter::MidiLighter(ChannelState* c)
: state         (std::make_unique<MidiLighterState>())
, m_channelState(c)
{
}


/* -------------------------------------------------------------------------- */


MidiLighter::MidiLighter(const patch::Channel& p, ChannelState* c)
: state         (std::make_unique<MidiLighterState>(p))
, m_channelState(c)
{
}


/* -------------------------------------------------------------------------- */


MidiLighter::MidiLighter(const MidiLighter& o, ChannelState* c)
: state         (std::make_unique<MidiLighterState>(*o.state))
, m_channelState(c)
{
}


/* -------------------------------------------------------------------------- */


void MidiLighter::parse(const mixer::Event& e, bool audible) const
{
    if (state->enabled.load() == false)
        return;

    uint32_t l_playing = state->playing.load();
    uint32_t l_mute    = state->mute.load();
    uint32_t l_solo    = state->solo.load();

	switch (e.type) {

        case mixer::EventType::KEY_PRESS:
        case mixer::EventType::KEY_RELEASE:
        case mixer::EventType::KEY_KILL:
        case mixer::EventType::SEQUENCER_STOP:
            if (l_playing != 0x0) sendStatus(l_playing, audible); 
            break;

        case mixer::EventType::CHANNEL_MUTE:
            if (l_mute != 0x0) sendMute(l_mute); 
            break;

        case mixer::EventType::CHANNEL_SOLO:
            if (l_solo != 0x0) sendSolo(l_solo); 
            break;

        default: break;
    }
}


/* -------------------------------------------------------------------------- */


void MidiLighter::sendMute(uint32_t l_mute) const
{
	if (m_channelState->mute.load() == true)
		kernelMidi::sendMidiLightning(l_mute, midimap::midimap.muteOn);
	else
		kernelMidi::sendMidiLightning(l_mute, midimap::midimap.muteOff);    
}


/* -------------------------------------------------------------------------- */


void MidiLighter::sendSolo(uint32_t l_solo) const
{
	if (m_channelState->solo.load() == true)
		kernelMidi::sendMidiLightning(l_solo, midimap::midimap.soloOn);
	else
		kernelMidi::sendMidiLightning(l_solo, midimap::midimap.soloOff);
}


/* -------------------------------------------------------------------------- */


void MidiLighter::sendStatus(uint32_t l_playing, bool audible) const
{
    switch (m_channelState->playStatus.load()) {
        
        case ChannelStatus::OFF:
            kernelMidi::sendMidiLightning(l_playing, midimap::midimap.stopped);
            break;
        
        case ChannelStatus::WAIT:
            kernelMidi::sendMidiLightning(l_playing, midimap::midimap.waiting);
            break;

        case ChannelStatus::ENDING:
            kernelMidi::sendMidiLightning(l_playing, midimap::midimap.stopping);
            break;

        case ChannelStatus::PLAY:
            kernelMidi::sendMidiLightning(l_playing, audible ? midimap::midimap.playing : midimap::midimap.playingInaudible);
            break;

        default: break;        
    }
}
}} // giada::m::
