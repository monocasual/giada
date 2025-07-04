/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/types.h"
#include "src/core/worker.h"

namespace giada::m
{
class KernelMidi;
class MidiEvent;
class MidiSynchronizer final
{
public:
	MidiSynchronizer(KernelMidi&);

	/* receive
	Receives a MidiEvent and reacts accordingly. Valid only when in SLAVE mode. */

	void receive(const MidiEvent&, int numBeatsInLoop);

	/* startSendClock, stopSendClock
	Sends MIDI clock data for synchronization with other MIDI devices. Valid only
	when in MASTER mode. */

	void startSendClock(float bpm);
	void stopSendClock() const;

	void sendRewind();
	void sendStart();
	void sendStop();

	void setClockBpm(float);

	std::function<void(int)>   onChangePosition;
	std::function<void(float)> onChangeBpm;
	std::function<void()>      onStart;
	std::function<void()>      onStop;

private:
	/* computeClock
	Computes the current bpm value and sends the corresponding event to the
	engine when necessary. */

	void computeClock(double timestamp);

	/* computePosition
	Given a SPP (Song Position Pointer), it jumps to the right beat. */

	void computePosition(int sppPosition, int numBeatsInLoop);

	KernelMidi& m_kernelMidi;

	/* m_worker
	A separate thread responsible for the MIDI Clock output. */

	Worker m_worker;

	double m_timeElapsed;
	double m_lastTimestamp;
	double m_lastDelta;
	double m_lastBpm;
};
} // namespace giada::m

#endif
