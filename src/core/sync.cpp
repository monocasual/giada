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

#include "sync.h"
#include "core/conf.h"
#include "core/kernelAudio.h"
#include "core/kernelMidi.h"
#include "core/model/model.h"

namespace giada::m::sync
{
namespace
{
/* midiTC*
MIDI timecode variables. */

int midiTCrate_    = 0; // Send MTC data every midiTCrate_ frames
int midiTCframes_  = 0;
int midiTCseconds_ = 0;
int midiTCminutes_ = 0;
int midiTChours_   = 0;

#ifdef WITH_AUDIO_JACK
kernelAudio::JackState jackStatePrev_;
#endif
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::function<void()>      onJackRewind    = nullptr;
std::function<void(float)> onJackChangeBpm = nullptr;
std::function<void()>      onJackStart     = nullptr;
std::function<void()>      onJackStop      = nullptr;

/* -------------------------------------------------------------------------- */

void init(int sampleRate, float midiTCfps)
{
	midiTCrate_ = static_cast<int>((sampleRate / midiTCfps) * G_MAX_IO_CHANS); // stereo values
}

/* -------------------------------------------------------------------------- */

void sendMIDIsync()
{
	const model::Clock& c = model::get().clock;

	/* Sending MIDI sync while waiting is meaningless. */

	if (c.status == ClockStatus::WAITING)
		return;

	int currentFrame = c.state->currentFrame.load();

	/* TODO - only Master (_M) is implemented so far. */

	if (conf::conf.midiSync == MIDI_SYNC_CLOCK_M)
	{
		if (currentFrame % (c.framesInBeat / 24) == 0)
			kernelMidi::send(MIDI_CLOCK, -1, -1);
		return;
	}

	if (conf::conf.midiSync == MIDI_SYNC_MTC_M)
	{

		/* check if a new timecode frame has passed. If so, send MIDI TC
		 * quarter frames. 8 quarter frames, divided in two branches:
		 * 1-4 and 5-8. We check timecode frame's parity: if even, send
		 * range 1-4, if odd send 5-8. */

		if (currentFrame % midiTCrate_ != 0) // no timecode frame passed
			return;

		/* frame low nibble
		 * frame high nibble
		 * seconds low nibble
		 * seconds high nibble */

		if (midiTCframes_ % 2 == 0)
		{
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCframes_ & 0x0F) | 0x00, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCframes_ >> 4) | 0x10, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCseconds_ & 0x0F) | 0x20, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCseconds_ >> 4) | 0x30, -1);
		}

		/* minutes low nibble
		 * minutes high nibble
		 * hours low nibble
		 * hours high nibble SMPTE frame rate */

		else
		{
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCminutes_ & 0x0F) | 0x40, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTCminutes_ >> 4) | 0x50, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTChours_ & 0x0F) | 0x60, -1);
			kernelMidi::send(MIDI_MTC_QUARTER, (midiTChours_ >> 4) | 0x70, -1);
		}

		midiTCframes_++;

		/* check if total timecode frames are greater than timecode fps:
		 * if so, a second has passed */

		if (midiTCframes_ > conf::conf.midiTCfps)
		{
			midiTCframes_ = 0;
			midiTCseconds_++;
			if (midiTCseconds_ >= 60)
			{
				midiTCminutes_++;
				midiTCseconds_ = 0;
				if (midiTCminutes_ >= 60)
				{
					midiTChours_++;
					midiTCminutes_ = 0;
				}
			}
			//u::log::print("%d:%d:%d:%d\n", midiTChours_, midiTCminutes_, midiTCseconds_, midiTCframes_);
		}
	}
}

/* -------------------------------------------------------------------------- */

void sendMIDIrewind()
{
	midiTCframes_  = 0;
	midiTCseconds_ = 0;
	midiTCminutes_ = 0;
	midiTChours_   = 0;

	/* For cueing the slave to a particular start point, Quarter Frame messages 
    are not used. Instead, an MTC Full Frame message should be sent. The Full 
    Frame is a SysEx message that encodes the entire SMPTE time in one message. */

	if (conf::conf.midiSync == MIDI_SYNC_MTC_M)
	{
		kernelMidi::send(MIDI_SYSEX, 0x7F, 0x00); // send msg on channel 0
		kernelMidi::send(0x01, 0x01, 0x00);       // hours 0
		kernelMidi::send(0x00, 0x00, 0x00);       // mins, secs, frames 0
		kernelMidi::send(MIDI_EOX, -1, -1);       // end of sysex
	}
	else if (conf::conf.midiSync == MIDI_SYNC_CLOCK_M)
		kernelMidi::send(MIDI_POSITION_PTR, 0, 0);
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_AUDIO_JACK

void recvJackSync(const kernelAudio::JackState& state)
{
	assert(onJackRewind != nullptr);
	assert(onJackChangeBpm != nullptr);
	assert(onJackStart != nullptr);
	assert(onJackStop != nullptr);

	kernelAudio::JackState jackStateCurr = state;

	if (jackStateCurr != jackStatePrev_)
	{
		if (jackStateCurr.frame != jackStatePrev_.frame && jackStateCurr.frame == 0)
		{
			G_DEBUG("JackState received - rewind to frame 0");
			onJackRewind();
		}

		// jackStateCurr.bpm == 0 if JACK doesn't send that info
		if (jackStateCurr.bpm != jackStatePrev_.bpm && jackStateCurr.bpm > 1.0f)
		{
			G_DEBUG("JackState received - bpm=" << jackStateCurr.bpm);
			onJackChangeBpm(jackStateCurr.bpm);
		}

		if (jackStateCurr.running != jackStatePrev_.running)
		{
			G_DEBUG("JackState received - running=" << jackStateCurr.running);
			jackStateCurr.running ? onJackStart() : onJackStop();
		}
	}

	jackStatePrev_ = jackStateCurr;
}

#endif
} // namespace giada::m::sync
