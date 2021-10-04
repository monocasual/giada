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

#ifndef G_SYNC_H
#define G_SYNC_H

#ifdef WITH_AUDIO_JACK
#include "core/jackTransport.h"
#endif
#include "core/types.h"
#include "core/conf.h"
#include <functional>

namespace giada::m::kernelAudio
{
struct JackState;
}

namespace giada::m::model
{
struct Sequencer;
}

namespace giada::m
{
class KernelMidi;
class Synchronizer final
{
public:
	Synchronizer(const Conf::Data&, KernelMidi&);

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

#ifdef WITH_AUDIO_JACK

	/* recvJackSync
    Receives a new JACK state. Called by Kernel Audio on each audio block. */

	void recvJackSync(const JackTransport::State& state);

	/* onJack[...]
    Callbacks called when something happens in the JACK state. */

	std::function<void()>      onJackRewind;
	std::function<void(float)> onJackChangeBpm;
	std::function<void()>      onJackStart;
	std::function<void()>      onJackStop;
#endif

private:
	/* midiTC*
    MIDI timecode variables. */

	int m_midiTCrate    = 0; // Send MTC data every m_midiTCrate frames
	int m_midiTCframes  = 0;
	int m_midiTCseconds = 0;
	int m_midiTCminutes = 0;
	int m_midiTChours   = 0;

#ifdef WITH_AUDIO_JACK

	JackTransport::State m_jackStatePrev;

#endif

	KernelMidi&       m_kernelMidi;
	const Conf::Data& m_conf;
};
} // namespace giada::m

#endif
