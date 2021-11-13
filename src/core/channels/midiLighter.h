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

#ifndef G_CHANNEL_MIDI_LIGHTER_H
#define G_CHANNEL_MIDI_LIGHTER_H

#include "core/eventDispatcher.h"
#include "core/midiLearnParam.h"
#include "core/midiMapper.h"
#include "core/patch.h"

namespace giada::m
{
template <typename KernelMidiI>
class MidiLighter final
{
public:
	MidiLighter(MidiMapper<KernelMidiI>&);
	MidiLighter(MidiMapper<KernelMidiI>&, const Patch::Channel&);
	MidiLighter(const MidiLighter& o) = default;

	void sendStatus(ChannelStatus, bool audible);
	void sendMute(bool isMuted);
	void sendSolo(bool isSoloed);

	/* enabled
    Tells whether MIDI lighting is enabled or not. */

	bool enabled;

	/* MIDI learning fields for MIDI lighting. */

	MidiLearnParam playing;
	MidiLearnParam mute;
	MidiLearnParam solo;

private:
	MidiMapper<KernelMidiI>* m_midiMapper;
};

extern template struct MidiLighter<KernelMidi>;
#ifdef WITH_TESTS
extern template struct MidiLighter<KernelMidiMock>;
#endif
} // namespace giada::m

#endif
