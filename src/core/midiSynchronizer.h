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

#ifndef G_MIDI_SYNCHRONIZER_H
#define G_MIDI_SYNCHRONIZER_H

#include "core/conf.h"
#include "core/types.h"

namespace giada::m::model
{
class Sequencer;
}

namespace giada::m
{
class KernelMidi;
class MidiSynchronizer final
{
public:
	MidiSynchronizer(const Conf::Data&, KernelMidi&);

	/* reset
	Brings everything back to the initial state. */

	void reset();

	/* sendMIDIsync
    Generates MIDI sync output data. */

	void sendMIDIsync(const model::Sequencer& clock);

	/* sendMIDIrewind
    Rewinds timecode to beat 0 and also send a MTC full frame to cue the slave. */

	void sendMIDIrewind();

	void sendMIDIstart();
	void sendMIDIstop();

private:
	/* midiTC*
    MIDI timecode variables. */

	int m_midiTCrate    = 0; // Send MTC data every m_midiTCrate frames
	int m_midiTCframes  = 0;
	int m_midiTCseconds = 0;
	int m_midiTCminutes = 0;
	int m_midiTChours   = 0;

	KernelMidi&       m_kernelMidi;
	const Conf::Data& m_conf;
};
} // namespace giada::m

#endif
