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

#include "clock.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/kernelAudio.h"
#include "core/kernelMidi.h"
#include "core/mixerHandler.h"
#include "core/model/model.h"
#include "core/recorderHandler.h"
#include "core/sequencer.h"
#include "glue/events.h"
#include "utils/log.h"
#include "utils/math.h"
#include <atomic>
#include <cassert>

namespace giada::m::clock
{
namespace
{
/* quantizerStep_
Tells how many frames to wait to perform a quantized action. */

int quantizerStep_ = 1;

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

/* -------------------------------------------------------------------------- */

/* recomputeFrames_
Updates bpm, frames, beats and so on. Private version. */

void recomputeFrames_(model::Clock& c)
{
	c.framesInLoop = static_cast<int>((conf::conf.samplerate * (60.0f / c.bpm)) * c.beats);
	c.framesInBar  = static_cast<int>(c.framesInLoop / (float)c.bars);
	c.framesInBeat = static_cast<int>(c.framesInLoop / (float)c.beats);
	c.framesInSeq  = c.framesInBeat * G_MAX_BEATS;

	if (c.quantize != 0)
		quantizerStep_ = c.framesInBeat / c.quantize;
}

/* -------------------------------------------------------------------------- */

void setBpm_(float current)
{
	float previous = model::get().clock.bpm;

	model::get().clock.bpm = current;
	recomputeFrames_(model::get().clock);

	m::recorderHandler::updateBpm(previous, current, quantizerStep_);

	model::swap(model::SwapType::HARD);

	u::log::print("[clock::setBpm_] Bpm changed to %f\n", current);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void init(int sampleRate, float midiTCfps)
{
	midiTCrate_ = static_cast<int>((sampleRate / midiTCfps) * G_MAX_IO_CHANS); // stereo values

	model::get().clock.bars     = G_DEFAULT_BARS;
	model::get().clock.beats    = G_DEFAULT_BEATS;
	model::get().clock.bpm      = G_DEFAULT_BPM;
	model::get().clock.quantize = G_DEFAULT_QUANTIZE;
	recomputeFrames_(model::get().clock);

	model::swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void recomputeFrames()
{
	recomputeFrames_(model::get().clock);
	model::swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

bool isRunning()
{
	return model::get().clock.status == ClockStatus::RUNNING;
}

bool isActive()
{
	const model::Clock& c = model::get().clock;
	return c.status == ClockStatus::RUNNING || c.status == ClockStatus::WAITING;
}

bool quantoHasPassed()
{
	const model::Clock& c = model::get().clock;
	return clock::getQuantizerValue() != 0 && c.state->currentFrame.load() % quantizerStep_ == 0;
}

bool isOnBar()
{
	const model::Clock& c = model::get().clock;

	int currentFrame = c.state->currentFrame.load();

	if (c.status == ClockStatus::WAITING || currentFrame == 0)
		return false;
	return currentFrame % c.framesInBar == 0;
}

bool isOnBeat()
{
	const model::Clock& c = model::get().clock;

	if (c.status == ClockStatus::WAITING)
		return c.state->currentFrameWait.load() % c.framesInBeat == 0;
	return c.state->currentFrame.load() % c.framesInBeat == 0;
}

bool isOnFirstBeat()
{
	return model::get().clock.state->currentFrame.load() == 0;
}

/* -------------------------------------------------------------------------- */

void setBpm(float b)
{
	b = std::clamp(b, G_MIN_BPM, G_MAX_BPM);

	/* If JACK is being used, let it handle the bpm change. */

#ifdef WITH_AUDIO_JACK
	if (kernelAudio::getAPI() == G_SYS_API_JACK)
	{
		kernelAudio::jackSetBpm(b);
		return;
	}
#endif

	setBpm_(b);
}

void setBeats(int newBeats, int newBars)
{
	newBeats = std::clamp(newBeats, 1, G_MAX_BEATS);
	newBars  = std::clamp(newBars, 1, newBeats); // Bars cannot be greater than beats

	model::get().clock.beats = newBeats;
	model::get().clock.bars  = newBars;
	recomputeFrames_(model::get().clock);

	model::swap(model::SwapType::HARD);
}

void setQuantize(int q)
{
	model::get().clock.quantize = q;
	recomputeFrames_(model::get().clock);

	model::swap(model::SwapType::HARD);
}

void setStatus(ClockStatus s)
{
	model::get().clock.status = s;
	model::swap(model::SwapType::SOFT);

	if (s == ClockStatus::RUNNING)
	{
		if (conf::conf.midiSync == MIDI_SYNC_CLOCK_M)
		{
			kernelMidi::send(MIDI_START, -1, -1);
			kernelMidi::send(MIDI_POSITION_PTR, 0, 0);
		}
	}
	else if (s == ClockStatus::STOPPED)
	{
		if (conf::conf.midiSync == MIDI_SYNC_CLOCK_M)
			kernelMidi::send(MIDI_STOP, -1, -1);
	}
}

/* -------------------------------------------------------------------------- */

void advance(Frame amount)
{
	const model::Clock& c = model::get().clock;

	if (c.status == ClockStatus::WAITING)
	{
		int f = (c.state->currentFrameWait.load() + amount) % c.framesInLoop;
		c.state->currentFrameWait.store(f);
		return;
	}

	int f = (c.state->currentFrame.load() + amount) % c.framesInLoop;
	int b = f / c.framesInBeat;

	c.state->currentFrame.store(f);
	c.state->currentBeat.store(b);
}

/* -------------------------------------------------------------------------- */

void rewind()
{
	const model::Clock& c = model::get().clock;

	c.state->currentFrame.store(0);
	c.state->currentBeat.store(0);
	c.state->currentFrameWait.store(0);

	sendMIDIrewind();
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

	/* For cueing the slave to a particular start point, Quarter Frame
	 * messages are not used. Instead, an MTC Full Frame message should
	 * be sent. The Full Frame is a SysEx message that encodes the entire
	 * SMPTE time in one message */

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

void recvJackSync()
{
	/* TODO - these things should be processed by a higher level, 
	above clock:: ----> clockManager */

	kernelAudio::JackState jackStateCurr = kernelAudio::jackTransportQuery();

	if (jackStateCurr != jackStatePrev_)
	{
		if (jackStateCurr.frame != jackStatePrev_.frame && jackStateCurr.frame == 0)
		{
			G_DEBUG("JackState received - rewind to frame 0");
			sequencer::rewind();
		}

		// jackStateCurr.bpm == 0 if JACK doesn't send that info
		if (jackStateCurr.bpm != jackStatePrev_.bpm && jackStateCurr.bpm > 1.0f)
		{
			G_DEBUG("JackState received - bpm=" << jackStateCurr.bpm);
			setBpm_(jackStateCurr.bpm);
		}

		if (jackStateCurr.running != jackStatePrev_.running)
		{
			G_DEBUG("JackState received - running=" << jackStateCurr.running);
			jackStateCurr.running ? sequencer::start() : sequencer::stop();
		}
	}

	jackStatePrev_ = jackStateCurr;
}

#endif

/* -------------------------------------------------------------------------- */

bool canQuantize()
{
	const model::Clock& c = model::get().clock;

	return c.quantize > 0 && c.status == ClockStatus::RUNNING;
}

/* -------------------------------------------------------------------------- */

Frame quantize(Frame f)
{
	if (!canQuantize())
		return f;
	return u::math::quantize(f, quantizerStep_) % getFramesInLoop(); // No overflow
}

/* -------------------------------------------------------------------------- */

int         getCurrentFrame() { return model::get().clock.state->currentFrame.load(); }
int         getCurrentBeat() { return model::get().clock.state->currentBeat.load(); }
int         getQuantizerStep() { return quantizerStep_; }
ClockStatus getStatus() { return model::get().clock.status; }
int         getFramesInLoop() { return model::get().clock.framesInLoop; }
int         getFramesInBar() { return model::get().clock.framesInBar; }
int         getFramesInBeat() { return model::get().clock.framesInBeat; }
int         getFramesInSeq() { return model::get().clock.framesInSeq; }
int         getQuantizerValue() { return model::get().clock.quantize; }
float       getBpm() { return model::get().clock.bpm; }
int         getBeats() { return model::get().clock.beats; }
int         getBars() { return model::get().clock.bars; }

/* -------------------------------------------------------------------------- */

float getCurrentSecond()
{
	return getCurrentFrame() / static_cast<float>(conf::conf.samplerate);
}

/* -------------------------------------------------------------------------- */

Frame getMaxFramesInLoop()
{
	return (conf::conf.samplerate * (60.0f / G_MIN_BPM)) * getBeats();
}

/* -------------------------------------------------------------------------- */

float calcBpmFromRec(Frame recordedFrames)
{
	return (60.0f * getBeats()) / (recordedFrames / static_cast<float>(conf::conf.samplerate));
}
} // namespace giada::m::clock
