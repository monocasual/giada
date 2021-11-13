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
template <typename KernelMidiI>
Data<KernelMidiI>::Data(MidiMapper<KernelMidiI>& m)
: enabled(false)
, midiMapper(&m)
{
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
Data<KernelMidiI>::Data(MidiMapper<KernelMidiI>& m, const Patch::Channel& p)
: enabled(p.midiOutL)
, playing(p.midiOutLplaying)
, mute(p.midiOutLmute)
, solo(p.midiOutLsolo)
, midiMapper(&m)
{
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void Data<KernelMidiI>::sendStatus(ChannelStatus status, bool audible)
{
	const MidiMap& midiMap   = midiMapper->getCurrentMap();
	const uint32_t l_playing = playing.getValue();

	if (l_playing == 0x0)
		return;

	switch (status)
	{
	case ChannelStatus::OFF:
		midiMapper->sendMidiLightning(l_playing, midiMap.stopped);
		break;

	case ChannelStatus::WAIT:
		midiMapper->sendMidiLightning(l_playing, midiMap.waiting);
		break;

	case ChannelStatus::ENDING:
		midiMapper->sendMidiLightning(l_playing, midiMap.stopping);
		break;

	case ChannelStatus::PLAY:
		midiMapper->sendMidiLightning(l_playing, audible ? midiMap.playing : midiMap.playingInaudible);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void Data<KernelMidiI>::sendMute(bool isMuted)
{
	const MidiMap& midiMap = midiMapper->getCurrentMap();
	const uint32_t l_mute  = mute.getValue();

	if (l_mute != 0x0)
		midiMapper->sendMidiLightning(l_mute, isMuted ? midiMap.muteOn : midiMap.muteOff);
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void Data<KernelMidiI>::sendSolo(bool isSoloed)
{
	const MidiMap& midiMap = midiMapper->getCurrentMap();
	const uint32_t l_solo  = solo.getValue();

	if (l_solo != 0x0)
		midiMapper->sendMidiLightning(l_solo, isSoloed ? midiMap.soloOn : midiMap.soloOff);
}

/* -------------------------------------------------------------------------- */

template struct Data<KernelMidi>;
#ifdef WITH_TESTS
template struct Data<KernelMidiMock>;
#endif
} // namespace giada::m::midiLighter