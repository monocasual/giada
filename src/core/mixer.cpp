/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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
#include "mixer.h"


namespace giada {
namespace m {
namespace mixer
{
namespace
{
constexpr Frame TICKSIZE = 38;

float tock[TICKSIZE] = {
	0.059033,  0.117240,  0.173807,  0.227943,  0.278890,  0.325936,
	0.368423,  0.405755,  0.437413,  0.462951,  0.482013,  0.494333,
	0.499738,  0.498153,  0.489598,  0.474195,  0.452159,  0.423798,
	0.389509,  0.349771,  0.289883,  0.230617,  0.173194,  0.118739,
	0.068260,  0.022631, -0.017423, -0.051339,	-0.078721, -0.099345,
 -0.113163, -0.120295, -0.121028, -0.115804, -0.105209, -0.089954,
 -0.070862, -0.048844
};

float tick[TICKSIZE] = {
	0.175860,  0.341914,  0.488904,  0.608633,  0.694426,  0.741500,
	0.747229,  0.711293,	0.635697,  0.524656,  0.384362,  0.222636,
	0.048496, -0.128348, -0.298035, -0.451105, -0.579021, -0.674653,
 -0.732667, -0.749830, -0.688924, -0.594091, -0.474481, -0.340160,
 -0.201360, -0.067752,  0.052194,  0.151746,  0.226280,  0.273493,
	0.293425,  0.288307,  0.262252,  0.220811,  0.170435,  0.117887,
	0.069639,  0.031320
};

AudioBuffer vChanInput;   // virtual channel for recording
AudioBuffer vChanInToOut; // virtual channel in->out bridge (hear what you're playin)

Frame tickTracker = 0;
Frame tockTracker = 0;
bool tickPlay = false;
bool tockPlay = false;

/* inputTracker
Sample position while recording. */

Frame inputTracker = 0;


/* -------------------------------------------------------------------------- */

/* computePeak */

void computePeak(const AudioBuffer& buf, float& peak, Frame frame)
{
	for (int i=0; i<buf.countChannels(); i++)
		if (buf[frame][i] > peak)
			peak = buf[frame][i];
}


/* -------------------------------------------------------------------------- */

/* lineInRec
Records from line in. */

void lineInRec(const AudioBuffer& inBuf, unsigned frame)
{
	if (!mh::hasArmedSampleChannels() || !kernelAudio::isInputEnabled() || !recording)
		return;

	/* Delay comp: wait until waitRec reaches delayComp. WaitRec returns to 0 in 
	mixerHandler, as soon as the recording ends. */

	if (waitRec < conf::delayComp) {
		waitRec++;
		return;
	}

	for (int i=0; i<vChanInput.countChannels(); i++)
		vChanInput[inputTracker][i] += inBuf[frame][i] * inVol;  // adding: overdub!

	inputTracker++;
	if (inputTracker >= clock::getFramesInLoop())
		inputTracker = 0;
}


/* -------------------------------------------------------------------------- */

/* ProcessLineIn
Computes line in peaks, plus handles "hear what you're playin'" thing. */

void processLineIn(const AudioBuffer& inBuf, unsigned frame)
{
	if (!kernelAudio::isInputEnabled())
		return;

	computePeak(inBuf, peakIn, frame);

	/* "hear what you're playing" - process, copy and paste the input buffer onto 
	the output buffer. */

	if (inToOut)
		for (int i=0; i<vChanInToOut.countChannels(); i++)
			vChanInToOut[frame][i] = inBuf[frame][i] * inVol;
}


/* -------------------------------------------------------------------------- */

/* prepareBuffers
Cleans up every buffer, both in Mixer and in channels. */

void prepareBuffers(AudioBuffer& outBuf)
{
	outBuf.clear();
	vChanInToOut.clear();
	for (Channel* channel : channels)
		channel->prepareBuffer(clock::isRunning());
}


/* -------------------------------------------------------------------------- */

/* doQuantize
Computes quantization on 'rewind' button and all channels. */

void doQuantize(unsigned frame)
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

/* renderMetronome
Generates metronome when needed and pastes it to the output buffer. */

void renderMetronome(AudioBuffer& outBuf, unsigned frame)
{
	if (tockPlay) {
		for (int i=0; i<outBuf.countChannels(); i++)
			outBuf[frame][i] += tock[tockTracker];
		tockTracker++;
		if (tockTracker >= TICKSIZE-1) {
			tockPlay    = false;
			tockTracker = 0;
		}
	}
	if (tickPlay) {
		for (int i=0; i<outBuf.countChannels(); i++)
			outBuf[frame][i] += tick[tickTracker];
		tickTracker++;
		if (tickTracker >= TICKSIZE-1) {
			tickPlay    = false;
			tickTracker = 0;
		}
	}
}


/* -------------------------------------------------------------------------- */

/* renderIO
Final processing stage. Take each channel and process it (i.e. copy its
content to the output buffer). Process plugins too, if any. */

void renderIO(AudioBuffer& outBuf, const AudioBuffer& inBuf)
{
	for (Channel* channel : channels)
		channel->process(outBuf, inBuf, isChannelAudible(channel), clock::isRunning());

#ifdef WITH_VST
	pluginHost::processStack(outBuf, pluginHost::MASTER_OUT);
	pluginHost::processStack(vChanInToOut, pluginHost::MASTER_IN);
#endif
}


/* -------------------------------------------------------------------------- */

/* limitOutput
Applies a very dumb hard limiter. */

void limitOutput(AudioBuffer& outBuf, unsigned frame)
{
	for (int i=0; i<outBuf.countChannels(); i++)
		if      (outBuf[frame][i] > 1.0f)
			outBuf[frame][i] = 1.0f;
		else if (outBuf[frame][i] < -1.0f)	
			outBuf[frame][i] = -1.0f;	
}


/* -------------------------------------------------------------------------- */

/* finalizeOutput
Last touches after the output has been rendered: apply inToOut if any, apply
output volume. */

void finalizeOutput(AudioBuffer& outBuf, unsigned frame)
{
	/* Merge vChanInToOut, if enabled. */

	if (inToOut)
		outBuf.copyFrame(frame, vChanInToOut[frame]); 

	for (int i=0; i<outBuf.countChannels(); i++)
		outBuf[frame][i] *= outVol; 
}


/* -------------------------------------------------------------------------- */


void renderMetronome()
{
	if (!metronome)
		return;
	if (clock::isOnBar() || clock::isOnFirstBeat())
		tickPlay = true;
	else
	if (clock::isOnBeat())
		tockPlay = true;
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


std::vector<Channel*> channels;

bool   recording    = false;
bool   ready        = true;
float  outVol       = G_DEFAULT_OUT_VOL;
float  inVol        = G_DEFAULT_IN_VOL;
float  peakOut      = 0.0f;
float  peakIn       = 0.0f;
bool	 metronome    = false;
int    waitRec      = 0;
bool   rewindWait   = false;
bool   hasSolos     = false;
bool   inToOut      = false;

pthread_mutex_t mutex;


/* -------------------------------------------------------------------------- */


void init(Frame framesInSeq, Frame framesInBuffer)
{
	/* Allocate virtual input channels. vChanInput has variable size: it depends
	on how many frames there are in sequencer. */
	
	vChanInput.alloc(framesInSeq, G_MAX_IO_CHANS);
	vChanInToOut.alloc(framesInBuffer, G_MAX_IO_CHANS);

	gu_log("[Mixer::init] buffers ready - framesInSeq=%d, framesInBuffer=%d\n", 
		framesInSeq, framesInBuffer);	

	hasSolos = false;

	pthread_mutex_init(&mutex, nullptr);

	rewind();
}


/* -------------------------------------------------------------------------- */


void allocVirtualInput(Frame frames)
{
	vChanInput.alloc(frames, G_MAX_IO_CHANS);
}


/* -------------------------------------------------------------------------- */


int masterPlay(void* outBuf, void* inBuf, unsigned bufferSize, 
	double streamTime, RtAudioStreamStatus status, void* userData)
{
	if (!ready)
		return 0;

	pthread_mutex_lock(&mutex);

#ifdef __linux__
	clock::recvJackSync();
#endif

	AudioBuffer out, in;
	out.setData((float*) outBuf, bufferSize, G_MAX_IO_CHANS);
	if (kernelAudio::isInputEnabled())
		in.setData((float*) inBuf, bufferSize, G_MAX_IO_CHANS);

	peakOut = 0.0f;  // reset peak calculator
	peakIn  = 0.0f;  // reset peak calculator

	prepareBuffers(out);

	for (unsigned j=0; j<bufferSize; j++) {
		processLineIn(in, j);   // TODO - can go outside this loop

		if (clock::isRunning()) {
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

			lineInRec(in, j);   // TODO - can go outside this loop
			doQuantize(j);
			renderMetronome();
			clock::incrCurrentFrame();
			clock::sendMIDIsync();
		}
	}
	
	renderIO(out, in);

	/* Post processing. */
	for (unsigned j=0; j<bufferSize; j++) {
		finalizeOutput(out, j); 
		if (conf::limitOutput)
			limitOutput(out, j);
		computePeak(out, peakOut, j); 
		renderMetronome(out, j);
	}

	/* Unset data in buffers. If you don't do this, buffers go out of scope and
	destroy memory allocated by RtAudio ---> havoc. */
	out.setData(nullptr, 0, 0);
	in.setData(nullptr, 0, 0);

	pthread_mutex_unlock(&mutex);

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
	/* Start inputTracker from the current frame, not the beginning. */
	recording    = true;
	inputTracker = clock::getCurrentFrame();
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
			sch->wave->copyData(vChanInput[0], vChanInput.countFrames());
	}
	vChanInput.clear();
}
}}}; // giada::m::mixer::
