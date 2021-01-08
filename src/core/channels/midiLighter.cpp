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


#include "core/channels/channel.h"
#include "core/mixer.h"
#include "core/kernelMidi.h"
#include "core/midiMapConf.h"
#include "midiLighter.h"


namespace giada::m::midiLighter
{
namespace
{
void sendMute_(channel::Data& ch, uint32_t l_mute)
{
	if (ch.mute)
		kernelMidi::sendMidiLightning(l_mute, midimap::midimap.muteOn);
	else
		kernelMidi::sendMidiLightning(l_mute, midimap::midimap.muteOff);
}


/* -------------------------------------------------------------------------- */


void sendSolo_(channel::Data& ch, uint32_t l_solo)
{
	if (ch.solo)
		kernelMidi::sendMidiLightning(l_solo, midimap::midimap.soloOn);
	else
		kernelMidi::sendMidiLightning(l_solo, midimap::midimap.soloOff);
}


/* -------------------------------------------------------------------------- */


void sendStatus_(channel::Data& ch, uint32_t l_playing, bool audible)
{
    switch (ch.state->playStatus.load()) {
        
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
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Data::Data(const patch::Channel& p)
: enabled(p.midiOutL)
, playing(p.midiOutLplaying)
, mute   (p.midiOutLmute)
, solo   (p.midiOutLsolo)
{
}


/* -------------------------------------------------------------------------- */


void react(channel::Data& ch, const eventDispatcher::Event& e, bool audible)
{
    if (!ch.midiLighter.enabled)
        return;

    uint32_t l_playing = ch.midiLighter.playing.getValue();
    uint32_t l_mute    = ch.midiLighter.mute.getValue();
    uint32_t l_solo    = ch.midiLighter.solo.getValue();

	switch (e.type) {

        case eventDispatcher::EventType::KEY_PRESS:
        case eventDispatcher::EventType::KEY_RELEASE:
        case eventDispatcher::EventType::KEY_KILL:
        case eventDispatcher::EventType::SEQUENCER_STOP:
            if (l_playing != 0x0) sendStatus_(ch, l_playing, audible);
            break;

        case eventDispatcher::EventType::CHANNEL_MUTE:
            if (l_mute != 0x0) sendMute_(ch, l_mute);
            break;

        case eventDispatcher::EventType::CHANNEL_SOLO:
            if (l_solo != 0x0) sendSolo_(ch, l_solo);
            break;

        default: break;
    }
}
} // giada::m::midiLighter::