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
#include "../deps/rtaudio-mod/RtAudio.h"
#include "../utils/log.h"
#include "../utils/math.h"
#include "wave.h"
#include "kernelAudio.h"
#include "recorder.h"
#include "pluginHost.h"
#include "conf.h"
#include "mixerHandler.h"
#include "clock.h"
#include "const.h"
#include "channel.h"
#include "sampleChannel.h"
#include "midiChannel.h"
#include "audioBuffer.h"
#include "action.h"
#include "mixer.h"


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

/* vChanInput
Virtual channel for input recording. */

AudioBuffer vChanInput_;

/* vChanInToOut
Virtual channel in->out bridge (hear what you're playin). */

AudioBuffer vChanInToOut_;

/* inputTracker
Frame position while recording. */

Frame inputTracker_ = 0;

std::function<void()> signalCb_ = nullptr;


/* -------------------------------------------------------------------------- */


void computePeak_(const AudioBuffer& buf, float& peak)
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
	if (!mh::hasArmedSampleChannels() || !kernelAudio::isInputEnabled() || !recording)
		return;

	for (int i=0; i<inBuf.countFrames(); i++, inputTracker_++)
		for (int j=0; j<inBuf.countChannels(); j++) {
			if (inputTracker_ >= clock::getFramesInLoop())
				inputTracker_ = 0;
			vChanInput_[inputTracker_][j] += inBuf[i][j] * inVol;  // adding: overdub!
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
	if (signalCb_ != nullptr && u::math::linearToDB(peakIn) > -10.0) // TODO
		signalCb_();

	/* "hear what you're playing" - process, copy and paste the input buffer onto 
	the output buffer. */

	if (inToOut)
		for (int i=0; i<vChanInToOut_.countFrames(); i++)
			for (int j=0; j<vChanInToOut_.countChannels(); j++)
				vChanInToOut_[i][j] = inBuf[i][j] * inVol;
}


/* -------------------------------------------------------------------------- */

/* prepareBuffers
Cleans up every buffer, both in Mixer and in channels. */

void prepareBuffers_(AudioBuffer& outBuf)
{
	outBuf.clear();
	vChanInToOut_.clear();
	for (Channel* channel : channels)
		channel->prepareBuffer(clock::isRunning());
}


/* -------------------------------------------------------------------------- */

/* doQuantize
Computes quantization on 'rewind' button and all channels. */

void doQuantize_(unsigned frame)
{
	/* Nothing to do if quantizer disabled or a quanto has not passed yet. */

	if (clock::getQuantize() == 0 || !clock::quantoHasPassed())
		return;

	if (rewindWait) {
		rewindWait = false;
		rewind();
	}
}


/* -------------------------------------------------------------------------- */

/* renderIO
Final processing stage. Take each channel and process it (i.e. copy its
content to the output buffer). Process plugins too, if any. */

void renderIO_(AudioBuffer& outBuf, const AudioBuffer& inBuf)
{
	for (Channel* channel : channels)
		channel->process(outBuf, inBuf, isChannelAudible(channel), clock::isRunning());

#ifdef WITH_VST
	pluginHost::processStack(outBuf, pluginHost::StackType::MASTER_OUT);
	pluginHost::processStack(vChanInToOut_, pluginHost::StackType::MASTER_IN);
#endif
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
	for (int i=0; i<outBuf.countFrames(); i++)
		for (int j=0; j<outBuf.countChannels(); j++) {
			if (inToOut) // Merge vChanInToOut_, if enabled
				outBuf[i][j] += vChanInToOut_[i][j];
			outBuf[i][j] *= outVol; 
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
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


std::vector<Channel*> channels;

bool  recording  = false;
bool  ready      = true;
float outVol     = G_DEFAULT_OUT_VOL;
float inVol      = G_DEFAULT_IN_VOL;
float peakOut    = 0.0f;
float peakIn     = 0.0f;
bool  metronome  = false;
int   waitRec    = 0;
bool  rewindWait = false;
bool  hasSolos   = false;
bool  inToOut    = false;

pthread_mutex_t mutex;


/* -------------------------------------------------------------------------- */


void init(Frame framesInSeq, Frame framesInBuffer)
{
	/* Allocate virtual input channels. vChanInput_ has variable size: it depends
	on how many frames there are in sequencer. */
	
	vChanInput_.alloc(framesInSeq, G_MAX_IO_CHANS);
	vChanInToOut_.alloc(framesInBuffer, G_MAX_IO_CHANS);

	gu_log("[Mixer::init] buffers ready - framesInSeq=%d, framesInBuffer=%d\n", 
		framesInSeq, framesInBuffer);	

	hasSolos = false;

	pthread_mutex_init(&mutex, nullptr);

	rewind();
}


/* -------------------------------------------------------------------------- */


void allocVirtualInput(Frame frames)
{
	vChanInput_.alloc(frames, G_MAX_IO_CHANS);
}


/* -------------------------------------------------------------------------- */


int masterPlay(void* outBuf, void* inBuf, unsigned bufferSize, 
	double streamTime, RtAudioStreamStatus status, void* userData)
{
	if (!ready)
		return 0;

#ifdef __linux__
	clock::recvJackSync();
#endif

	AudioBuffer out, in;
	out.setData((float*) outBuf, bufferSize, G_MAX_IO_CHANS);
	if (kernelAudio::isInputEnabled())
		in.setData((float*) inBuf, bufferSize, G_MAX_IO_CHANS);

	peakOut = 0.0f;  // reset peak calculator
	peakIn  = 0.0f;  // reset peak calculator

	prepareBuffers_(out);
	processLineIn_(in);

	pthread_mutex_lock(&mutex);

	if (clock::isRunning()) {

		for (unsigned j=0; j<bufferSize; j++) {
		
			FrameEvents fe;
			fe.frameLocal   = j;
			fe.frameGlobal  = clock::getCurrentFrame();
			fe.doQuantize   = clock::getQuantize() == 0 || !clock::quantoHasPassed();
			fe.onBar        = clock::isOnBar();
			fe.onFirstBeat  = clock::isOnFirstBeat();
			fe.quantoPassed = clock::quantoHasPassed();
			fe.actions      = recorder::getActionsOnFrame(clock::getCurrentFrame());

			for (Channel* channel : channels)
				channel->parseEvents(fe);

			doQuantize_(j);
			renderMetronome_(out, j);
			clock::incrCurrentFrame();
			clock::sendMIDIsync();
		}
		lineInRec_(in);
	}
	
	renderIO_(out, in);

	pthread_mutex_unlock(&mutex);

	/* Post processing */

	finalizeOutput_(out);
	limitOutput_(out);
	computePeak_(out, peakOut);

	/* Unset data in buffers. If you don't do this, buffers go out of scope and
	destroy memory allocated by RtAudio ---> havoc. */
	out.setData(nullptr, 0, 0);
	in.setData (nullptr, 0, 0);

	return 0;
}


/* -------------------------------------------------------------------------- */


void close()
{
	clock::stop();
	while (channels.size() > 0)
		mh::deleteChannel(channels.at(0));
	pthread_mutex_destroy(&mutex);
}


/* -------------------------------------------------------------------------- */


bool isSilent()
{
	for (const Channel* ch : channels)
		if (ch->isPlaying())
			return false;
	return true;
}


/* -------------------------------------------------------------------------- */


bool isChannelAudible(Channel* ch)
{
	return !hasSolos || (hasSolos && ch->solo);
}

bool isMetronomeOn() { return metronome_.running; }


/* -------------------------------------------------------------------------- */


void rewind()
{
	clock::rewind();
	if (clock::isRunning())
		for (Channel* ch : channels)
			ch->rewindBySeq();
}


/* -------------------------------------------------------------------------- */


void startInputRec()
{
	/* Start inputTracker_ from the current frame, not the beginning. */
	recording     = true;
	inputTracker_ = clock::getCurrentFrame();
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


/* -------------------------------------------------------------------------- */


void mergeVirtualInput()
{
	for (Channel* ch : channels) {
		/* TODO - move this to audioProc::*/
		if (ch->type == ChannelType::MIDI)
			continue;
		SampleChannel* sch = static_cast<SampleChannel*>(ch);
		if (sch->armed)
			sch->wave->copyData(vChanInput_[0], vChanInput_.countFrames());
	}
	vChanInput_.clear();
}
}}}; // giada::m::mixer::
