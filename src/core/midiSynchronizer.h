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
#include "deps/geompp/src/range.hpp"

namespace giada::m::model
{
class Sequencer;
}

namespace giada::m
{
class KernelMidi;
class MidiEvent;
class MidiSynchronizer final
{
public:
	MidiSynchronizer(const Conf::Data&, KernelMidi&);

	/* receive
	Receives a MidiEvent and reacts accordingly. Valid only when in SLAVE mode. */

	void receive(const MidiEvent&, int numBeatsInLoop);

	/* advance
    Generates MIDI sync output data when needed. Call this on each audio block. */

	void advance(geompp::Range<Frame>, int framesInBeat);

	void sendRewind();
	void sendStart();
	void sendStop();

private:
	/* computeClock
	Computes the current bpm value and sends the corresponding event to the
	engine when necessary. */

	void computeClock(double timestamp);

	/* computePosition 
	Given a SPP (Song Position Pointer), it jumps to the right beat. */

	void computePosition(int sppPosition, int numBeatsInLoop);

	KernelMidi&       m_kernelMidi;
	const Conf::Data& m_conf;

	double m_timeElapsed;
	double m_lastTimestamp;
	double m_lastDelta;
	double m_lastBpm;
};
} // namespace giada::m

#endif
