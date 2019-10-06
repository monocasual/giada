/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include "deps/rtaudio-mod/RtAudio.h"
#include "utils/log.h"
#include "utils/math.h"
#include "core/model/model.h"
#include "core/channels/channel.h"
#include "core/channels/sampleChannel.h"
#include "core/channels/midiChannel.h"
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
#include "core/mixer.h"


namespace giada {
namespace m {
namespace mixer
{
namespace
{
struct Metronome
{
	static constexpr Frame CLICK_SIZE = 38;

	float beat[CLICK_SIZE] = {
		 0.059033,  0.117240,  0.173807,  0.227943,  0.278890,  0.325936,
		 0.368423,  0.405755,  0.437413,  0.462951,  0.482013,  0.494333,
		 0.499738,  0.498153,  0.489598,  0.474195,  0.452159,  0.423798,
		 0.389509,  0.349771,  0.289883,  0.230617,  0.173194,  0.118739,
		 0.068260,  0.022631, -0.017423, -0.051339,	-0.078721, -0.099345,
		-0.113163, -0.120295, -0.121028, -0.115804, -0.105209, -0.089954,
		-0.070862, -0.048844
	};

	float bar[CLICK_SIZE] = {
		 0.175860,  0.341914,  0.488904,  0.608633,  0.694426,  0.741500,
		 0.747229,  0.711293,  0.635697,  0.524656,  0.384362,  0.222636,
		 0.048496, -0.128348, -0.298035, -0.451105, -0.579021, -0.674653,
		-0.732667, -0.749830, -0.688924, -0.594091, -0.474481, -0.340160,
	 	-0.201360, -0.067752,  0.052194,  0.151746,  0.226280,  0.273493,
		 0.293425,  0.288307,  0.262252,  0.220811,  0.170435,  0.117887,
		 0.069639,  0.031320
	};

	Frame tracker  = 0;
	bool  running  = false;
	bool  playBar  = false;
	bool  playBeat = false;

	void render(AudioBuffer& outBuf, bool& process, float* data, Frame f)
	{
		process = true;
		for (int i=0; i<outBuf.countChannels(); i++)
			outBuf[f][i] += data[tracker];
		if (++tracker > Metronome::CLICK_SIZE) {
			process = false;
			tracker = 0;
		}	
	}
} metronome_;

/* vChanInput_
Virtual channel for input recording. */

AudioBuffer vChanInput_;

/* vChanInToOut_
Virtual channel in->out bridge (hear what you're playin). */

AudioBuffer vChanInToOut_;

/* inputTracker_
Frame position while recording. */

Frame inputTracker_ = 0;

/* signalCb_
Callback triggered when the input signal level reaches a threshold. */

std::function<void()> signalCb_ = nullptr;

std::atomic<bool> processing_(false);
std::atomic<bool> active_(false);


/* -------------------------------------------------------------------------- */


void computePeak_(const AudioBuffer& buf, std::atomic<float>& peak)
{
	for (int i=0; i<buf.countFrames(); i++)
		for (int j=0; j<buf.countChannels(); j++)
			if (buf[i][j] > peak)
				peak = buf[i][j];
}


/* -------------------------------------------------------------------------- */

/* lineInRec
Records from line in. */

void lineInRec_(const AudioBuffer& inBuf)
{
	if (!recManager::isRecordingInput() || !kernelAudio::isInputEnabled())
		return;

	model::MixerLock lock(model::mixer);
	
	for (int i=0; i<inBuf.countFrames(); i++, inputTracker_++)
		for (int j=0; j<inBuf.countChannels(); j++) {
			if (inputTracker_ >= clock::getFramesInLoop())
				inputTracker_ = 0;
			vChanInput_[inputTracker_][j] += inBuf[i][j] * model::mixer.get()->inVol;  // adding: overdub!
		}
}


/* -------------------------------------------------------------------------- */

/* processLineIn
Computes line in peaks, plus handles "hear what you're playin'" thing. */

void processLineIn_(const AudioBuffer& inBuf)
{
	if (!kernelAudio::isInputEnabled())
		return;

	computePeak_(inBuf, peakIn);

	if (signalCb_ != nullptr && u::math::linearToDB(peakIn) > conf::recTriggerLevel) {
		signalCb_();
		signalCb_ = nullptr;
	}

	/* "hear what you're playing" - process, copy and paste the input buffer onto 
	the output buffer. */

	model::MixerLock lock(model::mixer);
	
	if (model::mixer.get()->inToOut)
		for (int i=0; i<vChanInToOut_.countFrames(); i++)
			for (int j=0; j<vChanInToOut_.countChannels(); j++)
				vChanInToOut_[i][j] = inBuf[i][j] * model::mixer.get()->inVol;
}


/* -------------------------------------------------------------------------- */

/* doQuantize
Computes quantization on 'rewind' button. */

void doQuantize_(unsigned frame)
{
	/* Nothing to do if quantizer disabled or a quanto has not passed yet. */

	if (clock::getQuantize() == 0 || !clock::quantoHasPassed())
		return;

	if (rewindWait) {
		rewindWait = false;
		clock::rewind();
	}
}


/* -------------------------------------------------------------------------- */


void renderMetronome_(AudioBuffer& outBuf, Frame f)
{
	if (!metronome_.running)
		return;

	if (clock::isOnBar() || metronome_.playBar)
		metronome_.render(outBuf, metronome_.playBar, metronome_.bar, f);
	else
	if (clock::isOnBeat() || metronome_.playBeat)
		metronome_.render(outBuf, metronome_.playBeat, metronome_.beat, f);
}


/* -------------------------------------------------------------------------- */


void parseEvents_(Frame f)
{
	mixer::FrameEvents fe = {
		.frameLocal   = f,
		.frameGlobal  = clock::getCurrentFrame(),
		.doQuantize   = clock::getQuantize() == 0 || !clock::quantoHasPassed(),
		.onBar        = clock::isOnBar(),
		.onFirstBeat  = clock::isOnFirstBeat(),
		.quantoPassed = clock::quantoHasPassed(),
		.actions      = recorder::getActionsOnFrame(clock::getCurrentFrame()),
	};

	model::ChannelsLock lock(model::channels);

	/* TODO - channel->parseEvents alters things in Channel (i.e. it's mutable).
	Refactoring needed ASAP. */

	for (Channel* ch : model::channels)
		ch->parseEvents(fe); 
}


/* -------------------------------------------------------------------------- */


void render_(AudioBuffer& out, const AudioBuffer& in, AudioBuffer& inToOut)
{
	bool running = clock::isRunning();

	model::ChannelsLock lock(model::channels);

	/* TODO - channel->render alters things in Channel (i.e. it's mutable).
	Refactoring needed ASAP. */

	for (const Channel* ch : model::channels) {
		if (ch == nullptr ||
			ch->id == mixer::MASTER_OUT_CHANNEL_ID ||
			ch->id == mixer::MASTER_IN_CHANNEL_ID)
			continue;
		const_cast<Channel*>(ch)->render(out, in, inToOut, isChannelAudible(ch), running);
	}

	/* Master channels are processed at the end, when the buffers have already 
	been filled. */

	model::get(model::channels, mixer::MASTER_OUT_CHANNEL_ID).render(out, in, inToOut, true, true);
	model::get(model::channels, mixer::MASTER_IN_CHANNEL_ID).render(out, in, inToOut, true, true);
}


/* -------------------------------------------------------------------------- */


void processSequencer_(AudioBuffer& out, const AudioBuffer& in)
{
	for (int j=0; j<out.countFrames(); j++) {
		if (clock::isRunning()) {
			parseEvents_(j);
			doQuantize_(j);
		}
		clock::sendMIDIsync();
		clock::incrCurrentFrame();
		renderMetronome_(out, j);
	}
	lineInRec_(in);
}


/* -------------------------------------------------------------------------- */

/* prepareBuffers
Cleans up every buffer. */

void prepareBuffers_(AudioBuffer& outBuf)
{
	outBuf.clear();
	vChanInToOut_.clear();
}


/* -------------------------------------------------------------------------- */

/* limitOutput
Applies a very dumb hard limiter. */

void limitOutput_(AudioBuffer& outBuf)
{
	if (!conf::limitOutput)
		return;
	for (int i=0; i<outBuf.countFrames(); i++)
		for (int j=0; j<outBuf.countChannels(); j++)
			if      (outBuf[i][j] > 1.0f)  outBuf[i][j] = 1.0f;
			else if (outBuf[i][j] < -1.0f) outBuf[i][j] = -1.0f;	
}


/* -------------------------------------------------------------------------- */

/* finalizeOutput
Last touches after the output has been rendered: apply inToOut if any, apply
output volume. */

void finalizeOutput_(AudioBuffer& outBuf)
{
	model::MixerLock lock(model::mixer);
	
	for (int i=0; i<outBuf.countFrames(); i++)
		for (int j=0; j<outBuf.countChannels(); j++) {
			if (model::mixer.get()->inToOut) // Merge vChanInToOut_, if enabled
				outBuf[i][j] += vChanInToOut_[i][j];
			outBuf[i][j] *= model::mixer.get()->outVol; 
		}
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


std::atomic<bool>  rewindWait(false);
std::atomic<float> peakOut(0.0);
std::atomic<float> peakIn(0.0);


/* -------------------------------------------------------------------------- */


void init(Frame framesInSeq, Frame framesInBuffer)
{
	/* Allocate virtual inputs. vChanInput_ has variable size: it depends
	on how many frames there are in sequencer. */
	
	vChanInput_.alloc(framesInSeq, G_MAX_IO_CHANS);
	vChanInToOut_.alloc(framesInBuffer, G_MAX_IO_CHANS);

	gu_log("[mixer::init] buffers ready - framesInSeq=%d, framesInBuffer=%d\n", 
		framesInSeq, framesInBuffer);	

	clock::rewind();
}


/* -------------------------------------------------------------------------- */


void enable()
{ 
	active_.store(true); 
	gu_log("[mixer::enable] enabled\n");
}


void disable() 
{ 
	active_.store(false);
	while (processing_.load() == true) 
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	gu_log("[mixer::disable] disabled\n");
}


/* -------------------------------------------------------------------------- */


void allocVirtualInput(Frame frames)
{
	vChanInput_.alloc(frames, G_MAX_IO_CHANS);
}


void clearVirtualInput()
{
	vChanInput_.clear();
}


const AudioBuffer& getVirtualInput()
{
	return vChanInput_;
}


/* -------------------------------------------------------------------------- */


int masterPlay(void* outBuf, void* inBuf, unsigned bufferSize, 
	double streamTime, RtAudioStreamStatus status, void* userData)
{
	if (!kernelAudio::isReady() || active_.load() == false)
		return 0;

	processing_.store(true);

#ifdef __linux__
	if (kernelAudio::getAPI() == G_SYS_API_JACK)
		clock::recvJackSync();
#endif

	AudioBuffer out, in;
	out.setData((float*) outBuf, bufferSize, G_MAX_IO_CHANS);
	if (kernelAudio::isInputEnabled())
		in.setData((float*) inBuf, bufferSize, G_MAX_IO_CHANS);

	/* Reset peak computation. */

	peakOut = 0.0;
	peakIn  = 0.0;

	prepareBuffers_(out);
	processLineIn_(in);

	/* Process model. */

//out[0][0] = 3.0f;

	if (clock::isActive()) 
		processSequencer_(out, in);
	render_(out, in, vChanInToOut_);

	/* Post processing. */

	finalizeOutput_(out);
	limitOutput_(out);
	computePeak_(out, peakOut);

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


bool isChannelAudible(const Channel* ch)
{
	model::MixerLock l(model::mixer);

	bool hasSolos = model::mixer.get()->hasSolos;
	return !hasSolos || (hasSolos && ch->solo);
}

bool isMetronomeOn() { return metronome_.running; }


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


void toggleMetronome()
{
	metronome_.running = !metronome_.running;
}


void setMetronome(bool v) 
{ 
	metronome_.running = v; 
}


/* -------------------------------------------------------------------------- */


void setSignalCallback(std::function<void()> f)
{
	signalCb_ = f;
}
}}}; // giada::m::mixer::
