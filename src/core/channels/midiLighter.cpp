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

#include "core/channels/midiLighter.h"
#include "core/channels/channel.h"
#include "core/kernelMidi.h"
#include "core/midiMapper.h"
#include "core/mixer.h"

namespace giada::m::midiLighter
{
namespace
{
void sendMute_(channel::Data& ch, uint32_t l_mute)
{
	MidiMapper&    midiMapper = *ch.midiLighter.midiMapper;
	const MidiMap& midiMap    = midiMapper.getCurrentMap();

	if (ch.mute)
		midiMapper.sendMidiLightning(l_mute, midiMap.muteOn);
	else
		midiMapper.sendMidiLightning(l_mute, midiMap.muteOff);
}

/* -------------------------------------------------------------------------- */

void sendSolo_(channel::Data& ch, uint32_t l_solo)
{
	MidiMapper&    midiMapper = *ch.midiLighter.midiMapper;
	const MidiMap& midiMap    = midiMapper.getCurrentMap();

	if (ch.solo)
		midiMapper.sendMidiLightning(l_solo, midiMap.soloOn);
	else
		midiMapper.sendMidiLightning(l_solo, midiMap.soloOff);
}

/* -------------------------------------------------------------------------- */

void sendStatus_(channel::Data& ch, uint32_t l_playing, bool audible)
{
	MidiMapper&    midiMapper = *ch.midiLighter.midiMapper;
	const MidiMap& midiMap    = midiMapper.getCurrentMap();

	switch (ch.state->playStatus.load())
	{
	case ChannelStatus::OFF:
		midiMapper.sendMidiLightning(l_playing, midiMap.stopped);
		break;

	case ChannelStatus::WAIT:
		midiMapper.sendMidiLightning(l_playing, midiMap.waiting);
		break;

	case ChannelStatus::ENDING:
		midiMapper.sendMidiLightning(l_playing, midiMap.stopping);
		break;

	case ChannelStatus::PLAY:
		midiMapper.sendMidiLightning(l_playing, audible ? midiMap.playing : midiMap.playingInaudible);
		break;

	default:
		break;
	}
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data::Data(MidiMapper& m)
: midiMapper(&m)
{
}

/* -------------------------------------------------------------------------- */

Data::Data(const Patch::Channel& p)
: enabled(p.midiOutL)
, playing(p.midiOutLplaying)
, mute(p.midiOutLmute)
, solo(p.midiOutLsolo)
{
}

/* -------------------------------------------------------------------------- */

void react(channel::Data& ch, const EventDispatcher::Event& e, bool audible)
{
	if (!ch.midiLighter.enabled)
		return;

	uint32_t l_playing = ch.midiLighter.playing.getValue();
	uint32_t l_mute    = ch.midiLighter.mute.getValue();
	uint32_t l_solo    = ch.midiLighter.solo.getValue();

	switch (e.type)
	{

	case EventDispatcher::EventType::KEY_PRESS:
	case EventDispatcher::EventType::KEY_RELEASE:
	case EventDispatcher::EventType::KEY_KILL:
	case EventDispatcher::EventType::SEQUENCER_STOP:
		if (l_playing != 0x0)
			sendStatus_(ch, l_playing, audible);
		break;

	case EventDispatcher::EventType::CHANNEL_MUTE:
		if (l_mute != 0x0)
			sendMute_(ch, l_mute);
		break;

	case EventDispatcher::EventType::CHANNEL_SOLO:
		if (l_solo != 0x0)
			sendSolo_(ch, l_solo);
		break;

	default:
		break;
	}
}
} // namespace giada::m::midiLighter