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

namespace giada::m::channel
{
struct Data;
}

namespace giada::m::midiLighter
{
template <typename KernelMidiI>
class Data final
{
public:
	Data(MidiMapper<KernelMidiI>&);
	Data(MidiMapper<KernelMidiI>&, const Patch::Channel&);
	Data(const Data& o) = default;

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
	MidiMapper<KernelMidiI>* midiMapper;
};

extern template struct Data<KernelMidi>;
#ifdef WITH_TESTS
extern template struct Data<KernelMidiMock>;
#endif
} // namespace giada::m::midiLighter

#endif
