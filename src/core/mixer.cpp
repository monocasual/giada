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
#include "core/action.h"
#include "core/audioBuffer.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/kernelAudio.h"
#include "core/mixerHandler.h"
#include "core/model/model.h"
#include "core/plugins/pluginHost.h"
#include "core/recManager.h"
#include "core/recorder.h"
#include "core/sequencer.h"
#include "core/swapper.h"
#include "core/wave.h"
#include "deps/rtaudio/RtAudio.h"
#include "utils/log.h"
#include "utils/math.h"
#include <cassert>
#include <cstring>

namespace giada::m::mixer
{
namespace
{
/* recBuffer_
Working buffer for audio recording. */

AudioBuffer recBuffer_;

/* inBuffer_
Working buffer for input channel. Used for the in->out bridge. */

AudioBuffer inBuffer_;

/* inputTracker_
Frame position while recording. */

Frame inputTracker_ = 0;

/* signalCb_
Callback triggered when the input signal level reaches a threshold. */

std::function<void()> signalCb_ = nullptr;

/* -------------------------------------------------------------------------- */

/* invokeSignalCb_
Invokes the signal callback. This is done by pumping a FUNCTION event to the
event dispatcher, rather than invoking the callback directly. This is done on
purpose: the callback might (and surely will) contain blocking stuff from 
model:: that the realtime thread cannot perform directly. */

void invokeSignalCb_()
{
	eventDispatcher::pumpEvent({eventDispatcher::EventType::FUNCTION, 0, 0, []() {
		                            signalCb_();
		                            signalCb_ = nullptr;
	                            }});
}

/* -------------------------------------------------------------------------- */

/* lineInRec
Records from line in. */

void lineInRec_(const AudioBuffer& inBuf)
{
	if (!recManager::isRecordingInput() || !kernelAudio::isInputEnabled())
		return;

	float inVol        = mh::getInVol();
	int   framesInLoop = clock::getFramesInLoop();

	for (int i = 0; i < inBuf.countFrames(); i++, inputTracker_++)
		for (int j = 0; j < inBuf.countChannels(); j++)
			recBuffer_[inputTracker_ % framesInLoop][j] += inBuf[i][j] * inVol; // adding: overdub!
}

/* -------------------------------------------------------------------------- */

/* processLineIn
Computes line in peaks and prepares the internal working buffer for input
recording. */

void processLineIn_(const model::Mixer& mixer, const AudioBuffer& inBuf)
{
	if (!kernelAudio::isInputEnabled())
		return;

	float peak = inBuf.getPeak();

	if (signalCb_ != nullptr && u::math::linearToDB(peak) > conf::conf.recTriggerLevel)
	{
		G_DEBUG("Signal > threshold!");
		invokeSignalCb_();
	}

	mixer.state->peakIn.store(peak);

	/* Prepare the working buffer for input stream, which will be processed 
	later on by the Master Input Channel with plug-ins. */

	assert(inBuf.countChannels() <= inBuffer_.countChannels());

	inBuffer_.copyData(inBuf, mh::getInVol());
}

/* -------------------------------------------------------------------------- */

void processChannels_(const model::Layout& layout, AudioBuffer& out, AudioBuffer& in)
{
	/* No channel processing if layout is locked: another thread is changing
	data (e.g. Plugins or Waves). */

	if (layout.locked)
		return;

	for (const channel::Data& c : layout.channels)
		if (!c.isInternal())
			channel::render(c, &out, &in, isChannelAudible(c));
}

/* -------------------------------------------------------------------------- */

void processSequencer_(const model::Layout& layout, AudioBuffer& out, const AudioBuffer& in)
{
	lineInRec_(in);

	if (!clock::isRunning())
		return;

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

void renderMasterIn_(const model::Layout& layout, AudioBuffer& in)
{
	channel::render(layout.getChannel(mixer::MASTER_IN_CHANNEL_ID), nullptr, &in, true);
}

void renderMasterOut_(const model::Layout& layout, AudioBuffer& out)
{
	channel::render(layout.getChannel(mixer::MASTER_OUT_CHANNEL_ID), &out, nullptr, true);
}

void renderPreview_(const model::Layout& layout, AudioBuffer& out)
{
	channel::render(layout.getChannel(mixer::PREVIEW_CHANNEL_ID), &out, nullptr, true);
}

/* -------------------------------------------------------------------------- */

/* prepareBuffers
Cleans up every buffer. */

void prepareBuffers_(AudioBuffer& outBuf)
{
	outBuf.clear();
	inBuffer_.clear();
}

/* -------------------------------------------------------------------------- */

/* limit_
Applies a very dumb hard limiter. */

void limit_(AudioBuffer& outBuf)
{
	for (int i = 0; i < outBuf.countFrames(); i++)
		for (int j = 0; j < outBuf.countChannels(); j++)
			outBuf[i][j] = std::max(-1.0f, std::min(outBuf[i][j], 1.0f));
}

/* -------------------------------------------------------------------------- */

/* finalizeOutput
Last touches after the output has been rendered: apply inToOut if any, apply
output volume, compute peak. */

void finalizeOutput_(const model::Mixer& mixer, AudioBuffer& outBuf)
{
	bool  inToOut = mh::getInToOut();
	float outVol  = mh::getOutVol();

	if (inToOut)
		outBuf.addData(inBuffer_, outVol);
	else
		outBuf.applyGain(outVol);

	if (conf::conf.limitOutput)
		limit_(outBuf);

	mixer.state->peakOut.store(outBuf.getPeak());
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void init(Frame framesInSeq, Frame framesInBuffer)
{
	/* Allocate virtual inputs. recBuffer_ has variable size: it depends
	on how many frames there are in sequencer. */

	recBuffer_.alloc(framesInSeq, G_MAX_IO_CHANS);
	inBuffer_.alloc(framesInBuffer, G_MAX_IO_CHANS);

	u::log::print("[mixer::init] buffers ready - framesInSeq=%d, framesInBuffer=%d\n",
	    framesInSeq, framesInBuffer);
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
	while (model::get().mixer.state->processing.load() == true)
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

const AudioBuffer& getRecBuffer()
{
	return recBuffer_;
}

/* -------------------------------------------------------------------------- */

int masterPlay(void* outBuf, void* inBuf, unsigned bufferSize,
    double /*streamTime*/, RtAudioStreamStatus /*status*/, void* /*userData*/)
{
	model::Lock          rtLock = model::get_RT();
	const model::Mixer&  mixer  = rtLock.get().mixer;
	const model::Kernel& kernel = rtLock.get().kernel;

	if (!kernel.audioReady || mixer.state->active.load() == false)
		return 0;

	mixer.state->processing.store(true);

#ifdef WITH_AUDIO_JACK
	if (kernelAudio::getAPI() == G_SYS_API_JACK)
		clock::recvJackSync();
#endif

	AudioBuffer out, in;
	out.setData(static_cast<float*>(outBuf), bufferSize, G_MAX_IO_CHANS);
	if (kernelAudio::isInputEnabled())
		in.setData(static_cast<float*>(inBuf), bufferSize, conf::conf.channelsInCount);

	/* Reset peak computation. */

	mixer.state->peakOut.store(0.0);
	mixer.state->peakIn.store(0.0);

	prepareBuffers_(out);
	processLineIn_(mixer, in);

	//out[0][0] = 3.0f;

	renderMasterIn_(rtLock.get(), inBuffer_);

	if (clock::isActive())
		processSequencer_(rtLock.get(), out, inBuffer_);
	processChannels_(rtLock.get(), out, inBuffer_);

	renderMasterOut_(rtLock.get(), out);
	renderPreview_(rtLock.get(), out);

	/* Post processing. */

	finalizeOutput_(mixer, out);

	/* Unset data in buffers. If you don't do this, buffers go out of scope and
	destroy memory allocated by RtAudio ---> havoc. */

	out.setData(nullptr, 0, 0);
	in.setData(nullptr, 0, 0);

	mixer.state->processing.store(false);

	return 0;
}

/* -------------------------------------------------------------------------- */

void startInputRec()
{
	/* Start inputTracker_ from the current frame, not the beginning. */
	inputTracker_ = clock::getCurrentFrame();
}

void stopInputRec()
{
	inputTracker_ = 0;
}

/* -------------------------------------------------------------------------- */

void setSignalCallback(std::function<void()> f)
{
	signalCb_ = f;
}

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

float getPeakOut() { return m::model::get().mixer.state->peakOut.load(); }
float getPeakIn() { return m::model::get().mixer.state->peakIn.load(); }
} // namespace giada::m::mixer
