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

#include "core/channels/midiLighter.h"
#include "core/kernelMidi.h"
#include "core/midiMapper.h"

namespace giada::m
{
template <typename KernelMidiI>
MidiLighter<KernelMidiI>::MidiLighter(MidiMapper<KernelMidiI>& m)
: enabled(false)
, onSend(nullptr)
, m_midiMapper(&m)
{
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
MidiLighter<KernelMidiI>::MidiLighter(MidiMapper<KernelMidiI>& m, const Patch::Channel& p)
: enabled(p.midiOutL)
, playing(p.midiOutLplaying)
, mute(p.midiOutLmute)
, solo(p.midiOutLsolo)
, m_midiMapper(&m)
{
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void MidiLighter<KernelMidiI>::sendStatus(ChannelStatus status, bool audible) const
{
	const MidiMap& midiMap   = m_midiMapper->currentMap;
	const uint32_t l_playing = playing.getValue();

	if (l_playing == 0x0)
		return;

	switch (status)
	{
	case ChannelStatus::OFF:
		send(l_playing, midiMap.stopped);
		break;

	case ChannelStatus::WAIT:
		send(l_playing, midiMap.waiting);
		break;

	case ChannelStatus::ENDING:
		send(l_playing, midiMap.stopping);
		break;

	case ChannelStatus::PLAY:
		send(l_playing, audible ? midiMap.playing : midiMap.playingInaudible);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void MidiLighter<KernelMidiI>::sendMute(bool isMuted) const
{
	const MidiMap& midiMap = m_midiMapper->currentMap;
	const uint32_t l_mute  = mute.getValue();

	if (l_mute != 0x0)
		send(l_mute, isMuted ? midiMap.muteOn : midiMap.muteOff);
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void MidiLighter<KernelMidiI>::sendSolo(bool isSoloed) const
{
	const MidiMap& midiMap = m_midiMapper->currentMap;
	const uint32_t l_solo  = solo.getValue();

	if (l_solo != 0x0)
		send(l_solo, isSoloed ? midiMap.soloOn : midiMap.soloOff);
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void MidiLighter<KernelMidiI>::send(uint32_t learnt, const MidiMap::Message& msg) const
{
	assert(onSend != nullptr);

	m_midiMapper->sendMidiLightning(learnt, msg);
	onSend();
}

/* -------------------------------------------------------------------------- */

template class MidiLighter<KernelMidi>;
#ifdef WITH_TESTS
template class MidiLighter<KernelMidiMock>;
#endif
} // namespace giada::m