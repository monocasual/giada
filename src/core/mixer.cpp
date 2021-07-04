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

#include "core/mixer.h"
#include "core/const.h"
#include "core/model/model.h"
#include "core/sequencer.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "utils/log.h"
#include "utils/math.h"

namespace giada::m::mixer
{
namespace
{
/* CH_LEFT, CH_RIGHT
Channels identifiers. */

constexpr int CH_LEFT  = 0;
constexpr int CH_RIGHT = 1;

/* recBuffer_
Working buffer for audio recording. */

mcl::AudioBuffer recBuffer_;

/* inBuffer_
Working buffer for input channel. Used for the in->out bridge. */

mcl::AudioBuffer inBuffer_;

/* inputTracker_
Frame position while recording. */

Frame inputTracker_ = 0;

/* signalCb_
Callback triggered when the input signal level reaches a threshold. */

std::function<void()> signalCb_ = nullptr;

/* endOfRecCb_
Callback triggered when the end of the internal recording buffer has been 
reached.*/

std::function<void()> endOfRecCb_ = nullptr;

/* signalCbFired_
Boolean guard to determine whether the signal callback has been fired or not.
Checking if signalCb_ != null (i.e. a callback is still present, so not fired
yet) is not enough, as the actual firing takes place on a different thread in
a slightly different moment (see fireSignalCb_() below). */

bool signalCbFired_ = false;

/* -------------------------------------------------------------------------- */

/* fireSignalCb_
Invokes the signal callback. This is done by pumping a FUNCTION event to the
event dispatcher, rather than invoking the callback directly. This is done on
purpose: the callback might (and surely will) contain blocking stuff from 
model:: that the realtime thread cannot perform directly. */

void fireSignalCb_()
{
	eventDispatcher::pumpUIevent({eventDispatcher::EventType::FUNCTION, 0, 0, []() {
		                              signalCb_();
		                              signalCb_ = nullptr;
	                              }});
}

/* -------------------------------------------------------------------------- */

/* fireEndOfRecCb_
Same rationale of fireSignalCb_, for the endOfRecCb_ callback. */

void fireEndOfRecCb_()
{
	eventDispatcher::pumpUIevent({eventDispatcher::EventType::FUNCTION, 0, 0, []() {
		                              endOfRecCb_();
		                              endOfRecCb_ = nullptr;
	                              }});
}

/* -------------------------------------------------------------------------- */

/* thresholdReached_
Returns true if left or right channel's peak has reached a certain threshold. */

bool thresholdReached_(Peak p, float threshold)
{
	return u::math::linearToDB(p.left) > threshold ||
	       u::math::linearToDB(p.right) > threshold;
}

/* -------------------------------------------------------------------------- */

/* lineInRec
Records from line in. 'maxFrames' determines how many frames to record before
the internal tracker loops over. The value changes whether you are recording
in RIGID or FREE mode. */

void lineInRec_(const mcl::AudioBuffer& inBuf, Frame maxFrames, float inVol)
{
	assert(maxFrames <= recBuffer_.countFrames());

	if (inputTracker_ >= maxFrames && endOfRecCb_ != nullptr)
	{
		fireEndOfRecCb_();
		return;
	}

	const Frame framesToCopy = -1; // copy everything
	const Frame srcOffset    = 0;
	const Frame destOffset   = inputTracker_ % maxFrames; // loop over at maxFrames

	recBuffer_.sum(inBuf, framesToCopy, srcOffset, destOffset, inVol);

	inputTracker_ += inBuf.countFrames();
}

/* -------------------------------------------------------------------------- */

/* processLineIn
Computes line in peaks and prepares the internal working buffer for input
recording. */

void processLineIn_(const model::Mixer& mixer, const mcl::AudioBuffer& inBuf,
    float inVol, float recTriggerLevel)
{
	const Peak peak{inBuf.getPeak(CH_LEFT), inBuf.getPeak(CH_RIGHT)};

	if (signalCb_ != nullptr && thresholdReached_(peak, recTriggerLevel) && !signalCbFired_)
	{
		G_DEBUG("Signal > threshold!");
		fireSignalCb_();
		signalCbFired_ = true;
	}

	mixer.state->peakInL.store(peak.left);
	mixer.state->peakInR.store(peak.right);

	/* Prepare the working buffer for input stream, which will be processed 
	later on by the Master Input Channel with plug-ins. */

	assert(inBuf.countChannels() <= inBuffer_.countChannels());

	inBuffer_.set(inBuf, inVol);
}

/* -------------------------------------------------------------------------- */

void processChannels_(const model::Layout& layout, mcl::AudioBuffer& out, mcl::AudioBuffer& in)
{
	for (const channel::Data& c : layout.channels)
		if (!c.isInternal())
			channel::render(c, &out, &in, isChannelAudible(c));
}

/* -------------------------------------------------------------------------- */

void processSequencer_(const model::Layout& layout, mcl::AudioBuffer& out, const mcl::AudioBuffer& in)
{
	/* Advance sequencer first, then render it (rendering is just about
	generating metronome audio). This way the metronome is aligned with 
	everything else. */

	const sequencer::EventBuffer& events = sequencer::advance(in.countFrames());
	sequencer::render(out);

	/* No channel processing if layout is locked: another thread is changing
    data (e.g. Plugins or Waves). */

	if (layout.locked)
		return;

	for (const channel::Data& c : layout.channels)
		if (!c.isInternal())
			channel::advance(c, events);
}

/* -------------------------------------------------------------------------- */

void renderMasterIn_(const model::Layout& layout, mcl::AudioBuffer& in)
{
	channel::render(layout.getChannel(mixer::MASTER_IN_CHANNEL_ID), nullptr, &in, true);
}

void renderMasterOut_(const model::Layout& layout, mcl::AudioBuffer& out)
{
	channel::render(layout.getChannel(mixer::MASTER_OUT_CHANNEL_ID), &out, nullptr, true);
}

void renderPreview_(const model::Layout& layout, mcl::AudioBuffer& out)
{
	channel::render(layout.getChannel(mixer::PREVIEW_CHANNEL_ID), &out, nullptr, true);
}

/* -------------------------------------------------------------------------- */

/* limit_
Applies a very dumb hard limiter. */

void limit_(mcl::AudioBuffer& outBuf)
{
	for (int i = 0; i < outBuf.countFrames(); i++)
		for (int j = 0; j < outBuf.countChannels(); j++)
			outBuf[i][j] = std::max(-1.0f, std::min(outBuf[i][j], 1.0f));
}

/* -------------------------------------------------------------------------- */

/* finalizeOutput
Last touches after the output has been rendered: apply inToOut if any, apply
output volume, compute peak. */

void finalizeOutput_(const model::Mixer& mixer, mcl::AudioBuffer& outBuf,
    const RenderInfo& info)
{
	if (info.inToOut)
		outBuf.sum(inBuffer_, info.outVol);
	else
		outBuf.applyGain(info.outVol);

	if (info.limitOutput)
		limit_(outBuf);

	mixer.state->peakOutL.store(outBuf.getPeak(CH_LEFT));
	mixer.state->peakOutR.store(outBuf.getPeak(CH_RIGHT));
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void init(Frame maxFramesInLoop, Frame framesInBuffer)
{
	/* Allocate working buffers. recBuffer_ has variable size: it depends on how
	many frames there are in the current loop. */

	recBuffer_.alloc(maxFramesInLoop, G_MAX_IO_CHANS);
	inBuffer_.alloc(framesInBuffer, G_MAX_IO_CHANS);

	u::log::print("[mixer::init] buffers ready - maxFramesInLoop=%d, framesInBuffer=%d\n",
	    maxFramesInLoop, framesInBuffer);
}

/* -------------------------------------------------------------------------- */

void enable()
{
	model::get().mixer.state->active.store(true);
	u::log::print("[mixer::enable] enabled\n");
}

void disable()
{
	model::get().mixer.state->active.store(false);
	while (model::isLocked())
		;
	u::log::print("[mixer::disable] disabled\n");
}

/* -------------------------------------------------------------------------- */

void allocRecBuffer(Frame frames)
{
	recBuffer_.alloc(frames, G_MAX_IO_CHANS);
}

void clearRecBuffer()
{
	recBuffer_.clear();
}

const mcl::AudioBuffer& getRecBuffer()
{
	return recBuffer_;
}

/* -------------------------------------------------------------------------- */

int render(mcl::AudioBuffer& out, const mcl::AudioBuffer& in, const RenderInfo& info)
{
	const model::Lock   rtLock = model::get_RT();
	const model::Mixer& mixer  = rtLock.get().mixer;

	inBuffer_.clear();

	/* Reset peak computation. */

	mixer.state->peakOutL.store(0.0);
	mixer.state->peakOutR.store(0.0);
	mixer.state->peakInL.store(0.0);
	mixer.state->peakInR.store(0.0);

	/* Process line IN if input has been enabled in KernelAudio. */

	if (info.hasInput)
	{
		processLineIn_(mixer, in, info.inVol, info.recTriggerLevel);
		renderMasterIn_(rtLock.get(), inBuffer_);
	}

	/* Record input audio and advance the sequencer only if clock is active:
	can't record stuff with the sequencer off. */

	if (info.isClockActive)
	{
		if (info.canLineInRec)
			lineInRec_(in, info.maxFramesToRec, info.inVol);
		if (info.isClockRunning)
			processSequencer_(rtLock.get(), out, inBuffer_);
	}

	/* Channel processing. Don't do it if layout is locked: another thread is 
	changing data (e.g. Plugins or Waves). */

	if (!rtLock.get().locked)
		processChannels_(rtLock.get(), out, inBuffer_);

	/* Render remaining internal channels. */

	renderMasterOut_(rtLock.get(), out);
	renderPreview_(rtLock.get(), out);

	/* Post processing. */

	finalizeOutput_(mixer, out, info);

	return 0;
}

/* -------------------------------------------------------------------------- */

void startInputRec(Frame from)
{
	inputTracker_  = from;
	signalCbFired_ = false;
}

Frame stopInputRec()
{
	Frame ret      = inputTracker_;
	inputTracker_  = 0;
	signalCbFired_ = false;
	return ret;
}

/* -------------------------------------------------------------------------- */

void setSignalCallback(std::function<void()> f) { signalCb_ = f; }
void setEndOfRecCallback(std::function<void()> f) { endOfRecCb_ = f; }

/* -------------------------------------------------------------------------- */

bool isChannelAudible(const channel::Data& c)
{
	if (c.isInternal())
		return true;
	if (c.mute)
		return false;
	bool hasSolos = model::get().mixer.hasSolos;
	return !hasSolos || (hasSolos && c.solo);
}

/* -------------------------------------------------------------------------- */

Peak getPeakOut()
{
	return {
	    m::model::get().mixer.state->peakOutL.load(),
	    m::model::get().mixer.state->peakOutR.load()};
}

Peak getPeakIn()
{
	return {
	    m::model::get().mixer.state->peakInL.load(),
	    m::model::get().mixer.state->peakInR.load()};
}

/* -------------------------------------------------------------------------- */

RecordInfo getRecordInfo()
{
	return {inputTracker_, recBuffer_.countFrames()};
}
} // namespace giada::m::mixer
