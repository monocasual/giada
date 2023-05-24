/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "utils/log.h"
#include "utils/time.h"
#include <numeric>

namespace giada::m
{
MidiSynchronizer::MidiSynchronizer(KernelMidi& k)
: onChangePosition(nullptr)
, onChangeBpm(nullptr)
, onStart(nullptr)
, onStop(nullptr)
, m_kernelMidi(k)
, m_worker(1000) // Default sleep time, will be reset anyway on startSendClock()
, m_timeElapsed(0.0)
, m_lastTimestamp(0.0)
, m_lastDelta(0.0)
, m_lastBpm(G_DEFAULT_BPM)
{
}

/* -------------------------------------------------------------------------- */

void MidiSynchronizer::receive(const MidiEvent& e, int numBeatsInLoop)
{
	assert(onStart != nullptr);
	assert(onStop != nullptr);

	/* MidiSynchronizer, if working in SLAVE mode, can receive SYSTEM_* MIDI 
	types. More specifically:
		* SYSTEM_CLOCK - when another MIDI device sends CLOCK data to perform
		  synchronization;
		* SYSTEM_START - when another MIDI device is about to start;
		* SYSTEM_STOP - when another MIDI device is about to stop;
		* SYSTEM_SPP - when another MIDI device has changed song position. */

	if (!m_kernelMidi.canSyncSlave() || e.getType() != MidiEvent::Type::SYSTEM)
		return;

	switch (e.getByte1())
	{
	case MidiEvent::SYSTEM_CLOCK:
		computeClock(e.getTimestamp());
		break;

	case MidiEvent::SYSTEM_START:
		onStart();
		break;

	case MidiEvent::SYSTEM_STOP:
		onStop();
		break;

	case MidiEvent::SYSTEM_SPP:
		computePosition(e.getSppPosition(), numBeatsInLoop);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void MidiSynchronizer::startSendClock(float bpm)
{
	if (!m_kernelMidi.canSyncMaster())
		return;

	setClockBpm(bpm);
	const MidiEvent clockEvent = MidiEvent::makeFrom1Byte(MidiEvent::SYSTEM_CLOCK);

	m_worker.start([this, clockEvent]() {
		if (!m_kernelMidi.send(clockEvent))
			G_DEBUG("Can't send MIDI out message!", );
	});
}

void MidiSynchronizer::stopSendClock() const
{
	m_worker.stop();
}

/* -------------------------------------------------------------------------- */

void MidiSynchronizer::setClockBpm(float bpm)
{
	const float rateMs = 60000.0 / (24.0 * bpm);
	m_worker.setSleep(rateMs);
}

/* -------------------------------------------------------------------------- */

void MidiSynchronizer::sendRewind()
{
	if (m_kernelMidi.canSyncMaster())
		m_kernelMidi.send(MidiEvent::makeFrom3Bytes(MidiEvent::SYSTEM_SPP, 0, 0));
}

/* -------------------------------------------------------------------------- */

void MidiSynchronizer::sendStart()
{
	if (m_kernelMidi.canSyncMaster())
		m_kernelMidi.send(MidiEvent::makeFrom1Byte(MidiEvent::SYSTEM_START));
}

/* -------------------------------------------------------------------------- */

void MidiSynchronizer::sendStop()
{
	if (m_kernelMidi.canSyncMaster())
		m_kernelMidi.send(MidiEvent::makeFrom1Byte(MidiEvent::SYSTEM_STOP));
}

/* -------------------------------------------------------------------------- */

void MidiSynchronizer::computeClock(double timestamp)
{
	assert(onChangeBpm != nullptr);

	/* A MIDI clock event (SYSTEM_CLOCK) is sent 24 times per quarter note, that 
	is 24 times per beat. This is tempo-relative, since the tempo defines the 
	length of a quarter note (aka frames in beat) and so the duration of each 
	pulse. Faster tempo -> faster SYSTEM_CLOCK events stream. Here we are
	interpreting that rate and converting into a BPM value. */

	/* SMOOTHNESS
	Smooth factor for delta and bpm. The smaller the value, the stronger the
	effect. SMOOTHNESS == 1.0 means no smooth effect. */

	constexpr double SMOOTHNESS = 0.1;

	/* BPM_CHANGE_FREQ
	How fast the bpm is changed, in seconds. */

	constexpr double BPM_CHANGE_FREQ = 1.0;

	/* Skip the very first iteration where the last timestamp does not exist
	yet. It would screw up BPM computation otherwise. */

	if (m_lastTimestamp == 0.0)
	{
		m_lastTimestamp = timestamp;
		return;
	}

	/* Compute a raw timestamp delta (rawDelta) and then smooth it out with the
	previous one (m_lastDelta) by a SMOOTHNESS factor. */

	const double rawDelta = timestamp - m_lastTimestamp;
	m_lastDelta           = (rawDelta * SMOOTHNESS) + (m_lastDelta * (1.0 - SMOOTHNESS));

	/* Do the same as delta for the BPM value. The raw bpm formula is a simplified
	version of 
		rawBpm = ((1.0 / m_lastDelta) / MIDI_CLOCK_PPQ) * 60.0;
	where MIDI_CLOCK_PPQ == 24.0 */

	const double rawBpm = 2.5 / m_lastDelta;
	m_lastBpm           = (rawBpm * SMOOTHNESS) + (m_lastBpm * (1.0 - SMOOTHNESS));

	m_lastTimestamp = timestamp;
	m_timeElapsed   = m_timeElapsed + m_lastDelta;

	if (m_timeElapsed > BPM_CHANGE_FREQ)
	{
		onChangeBpm(m_lastBpm);
		m_timeElapsed = 0;
	}
}

/* -------------------------------------------------------------------------- */

void MidiSynchronizer::computePosition(int sppPosition, int numBeatsInLoop)
{
	assert(onChangePosition != nullptr);

	/* Each MIDI Beat spans 6 MIDI Clocks. In other words, each MIDI Beat is a 
	16th note (since there are 24 MIDI Clocks in a quarter note).

	So 1 MIDI beat = a 16th note = 6 clock pulses. A quarter (aka a beat) is
	4 MIDI beats. */

	const int beat = (sppPosition / 4) % numBeatsInLoop;

	onChangePosition(beat);
}
} // namespace giada::m
