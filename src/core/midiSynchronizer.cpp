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

#include "core/midiSynchronizer.h"
#include "core/conf.h"
#include "core/kernelMidi.h"
#include "core/midiEvent.h"
#include "core/model/sequencer.h"

namespace giada::m
{
MidiSynchronizer::MidiSynchronizer(const Conf::Data& c, KernelMidi& k)
: m_kernelMidi(k)
, m_conf(c)
{
}

/* -------------------------------------------------------------------------- */

void MidiSynchronizer::advance(geompp::Range<Frame> block, int framesInBeat)
{
	if (m_conf.midiSync == G_MIDI_SYNC_NONE)
		return;

	/* A MIDI clock event (MIDI_CLOCK) is sent 24 times per quarter note, that 
	is 24 times per beat. This is tempo-relative, since the tempo defines the 
	length of a quarter note (aka frames in beat) and so the duration of each 
	pulse. Faster tempo -> faster MIDI_CLOCK events stream. */

	const int rate = framesInBeat / 24.0;
	for (Frame frame = block.a; frame < block.b; frame++)
	{
		if (frame % rate == 0)
			m_kernelMidi.send(MidiEvent::makeFrom1Byte(MidiEvent::SYSTEM_CLOCK));
	}
}

/* -------------------------------------------------------------------------- */

void MidiSynchronizer::sendRewind()
{
	if (m_conf.midiSync == G_MIDI_SYNC_NONE)
		return;
	m_kernelMidi.send(MidiEvent::makeFrom1Byte(MidiEvent::SYSTEM_SPP));
}

/* -------------------------------------------------------------------------- */

void MidiSynchronizer::sendStart()
{
	if (m_conf.midiSync == G_MIDI_SYNC_NONE)
		return;
	m_kernelMidi.send(MidiEvent::makeFrom1Byte(MidiEvent::SYSTEM_START));
	m_kernelMidi.send(MidiEvent::makeFrom1Byte(MidiEvent::SYSTEM_SPP));
}

/* -------------------------------------------------------------------------- */

void MidiSynchronizer::sendStop()
{
	if (m_conf.midiSync == G_MIDI_SYNC_NONE)
		return;
	m_kernelMidi.send(MidiEvent::makeFrom1Byte(MidiEvent::SYSTEM_STOP));
}
} // namespace giada::m
