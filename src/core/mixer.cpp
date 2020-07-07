/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include <cstring>
#include "deps/rtaudio/RtAudio.h"
#include "utils/log.h"
#include "utils/math.h"
#include "core/model/model.h"
#include "core/wave.h"
#include "core/kernelAudio.h"
#include "core/recorder.h"
#include "core/recManager.h"
#include "core/pluginHost.h"
#include "core/conf.h"
#include "core/mixerHandler.h"
#include "core/clock.h"
#include "core/const.h"
#include "core/audioBuffer.h"
#include "core/action.h"
#include "core/sequencer.h"
#include "core/mixer.h"


namespace giada {
namespace m {
namespace mixer
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

std::atomic<bool> processing_(false);
std::atomic<bool> active_(false);

/* eventBuffer_
Buffer of events sent to channels for event parsing. This is filled with Events
coming from the two event queues.*/

EventBuffer eventBuffer_;


/* -------------------------------------------------------------------------- */


bool isChannelAudible_(const Channel& c)
{
    if (c.getType() == ChannelType::MASTER || c.getType() == ChannelType::PREVIEW)
        return true;
    if (c.state->mute.load() == true)
        return false;
    model::MixerLock ml(model::mixer);
    bool hasSolos = model::mixer.get()->hasSolos;
    return !hasSolos || (hasSolos && c.state->solo.load() == true);
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
			recBuffer_[inputTracker_ % framesInLoop][j] += inBuf[i][j] * inVol;  // adding: overdub!
}


/* -------------------------------------------------------------------------- */

/* processLineIn
Computes line in peaks and prepares the internal working buffer for input
recording. */

void processLineIn_(const AudioBuffer& inBuf)
{
	if (!kernelAudio::isInputEnabled())
		return;

	peakIn.store(inBuf.getPeak());

	if (signalCb_ != nullptr && u::math::linearToDB(peakIn) > conf::conf.recTriggerLevel) {
G_DEBUG("Signal > threshold!");
		signalCb_();
		signalCb_ = nullptr;
	}

	/* Prepare the working buffer for input stream, which will be processed 
	later on by the Master Input Channel with plug-ins. */
	
	assert(inBuf.countChannels() <= inBuffer_.countChannels());

	model::MixerLock lock(model::mixer);
	inBuffer_.copyData(inBuf, mh::getInVol());
}


/* -------------------------------------------------------------------------- */


void fillEventBuffer_()
{
	eventBuffer_.clear();

	Event e;
	while (UIevents.pop(e))   eventBuffer_.push_back(e);
	while (MidiEvents.pop(e)) eventBuffer_.push_back(e);

#ifdef G_DEBUG_MODE
	for (const Event& e : eventBuffer_)
		G_DEBUG("Event type=" << (int) e.type << ", channel=" << e.action.channelId 
			<< ", delta=" << e.delta << ", globalFrame=" << clock::getCurrentFrame());
#endif
}


/* -------------------------------------------------------------------------- */


void processChannels_(AudioBuffer& out, AudioBuffer& in)
{
	model::ChannelsLock lock(model::channels);

	for (const Channel* c : model::channels) {
		bool audible = isChannelAudible_(*c);	
		c->parse(eventBuffer_, audible); 
		if (c->getType() != ChannelType::MASTER) {
			c->advance(out.countFrames());
			c->render(&out, &in, audible);
		}
	}
}


/* -------------------------------------------------------------------------- */


void processSequencer_(AudioBuffer& in)
{
	sequencer::parse(eventBuffer_);
	if (clock::isActive()) {
		if (clock::isRunning())
			sequencer::run(in.countFrames());
		lineInRec_(in);
	}
}


/* -------------------------------------------------------------------------- */


void renderMasterIn_(AudioBuffer& in)
{
	model::ChannelsLock lock(model::channels);
	model::get(model::channels, mixer::MASTER_IN_CHANNEL_ID).render(nullptr, &in, true);
}

void renderMasterOut_(AudioBuffer& out)
{
	model::ChannelsLock lock(model::channels);
	model::get(model::channels, mixer::MASTER_OUT_CHANNEL_ID).render(&out, nullptr, true);
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
	for (int i=0; i<outBuf.countFrames(); i++)
		for (int j=0; j<outBuf.countChannels(); j++)
			outBuf[i][j] = std::max(-1.0f, std::min(outBuf[i][j], 1.0f));
}


/* -------------------------------------------------------------------------- */

/* finalizeOutput
Last touches after the output has been rendered: apply inToOut if any, apply
output volume, compute peak. */

void finalizeOutput_(AudioBuffer& outBuf)
{
	bool  inToOut = mh::getInToOut();
	float outVol  = mh::getOutVol();

	if (inToOut)
		outBuf.addData(inBuffer_, outVol);
	else
		outBuf.applyGain(outVol);

	if (conf::conf.limitOutput)
		limit_(outBuf);
	
	peakOut.store(outBuf.getPeak());
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


std::atomic<float> peakOut(0.0);
std::atomic<float> peakIn(0.0);

Queue<Event, G_MAX_QUEUE_EVENTS> UIevents;
Queue<Event, G_MAX_QUEUE_EVENTS> MidiEvents;


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
	active_.store(true); 
	u::log::print("[mixer::enable] enabled\n");
}


void disable() 
{ 
	active_.store(false);
	while (processing_.load() == true) 
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
	double streamTime, RtAudioStreamStatus status, void* userData)
{
	if (!kernelAudio::isReady() || active_.load() == false)
		return 0;

	processing_.store(true);

#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)
	if (kernelAudio::getAPI() == G_SYS_API_JACK)
		clock::recvJackSync();
#endif

	AudioBuffer out, in;
	out.setData(static_cast<float*>(outBuf), bufferSize, G_MAX_IO_CHANS);
	if (kernelAudio::isInputEnabled())
		in.setData(static_cast<float*>(inBuf), bufferSize, conf::conf.channelsInCount);

	/* Reset peak computation. */

	peakOut = 0.0;
	peakIn  = 0.0;

	prepareBuffers_(out);
	processLineIn_(in);

//out[0][0] = 3.0f;

	renderMasterIn_(inBuffer_);

	fillEventBuffer_();
	processSequencer_(inBuffer_);
	processChannels_(out, inBuffer_);

	renderMasterOut_(out);

	/* Advance sequencer only when rendering is done. */

	if (clock::isActive())
		sequencer::advance(out);

	/* Post processing. */

	finalizeOutput_(out);

	/* Unset data in buffers. If you don't do this, buffers go out of scope and
	destroy memory allocated by RtAudio ---> havoc. */

	out.setData(nullptr, 0, 0);
	in.setData (nullptr, 0, 0);

	processing_.store(false);

	return 0;
}


/* -------------------------------------------------------------------------- */


void close()
{
	clock::setStatus(ClockStatus::STOPPED);
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


void pumpEvent(Event e)
{
	eventBuffer_.push_back(e);
}
}}}; // giada::m::mixer::
