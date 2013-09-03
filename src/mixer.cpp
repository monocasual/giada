/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * mixer
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#include <math.h>
#include "mixer.h"
#include "init.h"
#include "wave.h"
#include "gui_utils.h"
#include "recorder.h"
#include "pluginHost.h"
#include "patch.h"
#include "conf.h"
#include "mixerHandler.h"
#include "channel.h"
#include "kernelMidi.h"


extern Mixer 			 G_Mixer;
extern Patch		 	 G_Patch;
extern Conf				 G_Conf;
#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif


Mixer::Mixer() 	{}
Mixer::~Mixer() {}


#define TICKSIZE 38


float Mixer::tock[TICKSIZE] = {
	 0.059033,  0.117240,  0.173807,  0.227943,  0.278890,  0.325936,
	 0.368423,  0.405755,  0.437413,  0.462951,  0.482013,  0.494333,
	 0.499738,  0.498153,  0.489598,  0.474195,  0.452159,  0.423798,
	 0.389509,  0.349771,  0.289883,  0.230617,  0.173194,  0.118739,
	 0.068260,  0.022631, -0.017423, -0.051339,	-0.078721, -0.099345,
	-0.113163, -0.120295, -0.121028, -0.115804, -0.105209, -0.089954,
	-0.070862, -0.048844
};


float Mixer::tick[TICKSIZE] = {
	  0.175860,  0.341914,  0.488904,  0.608633,  0.694426,  0.741500,
	  0.747229,  0.711293,	0.635697,  0.524656,  0.384362,  0.222636,
	  0.048496, -0.128348, -0.298035, -0.451105, -0.579021, -0.674653,
	 -0.732667, -0.749830, -0.688924, -0.594091, -0.474481, -0.340160,
	 -0.201360, -0.067752,  0.052194,  0.151746,  0.226280,  0.273493,
	  0.293425,  0.288307,  0.262252,  0.220811,  0.170435,  0.117887,
	  0.069639,  0.031320
};


/* ------------------------------------------------------------------ */


void Mixer::init() {
	quanto      = 1;
	docross     = false;
	rewindWait  = false;
	running     = false;
	ready       = true;
	waitRec     = 0;
	actualFrame = 0;
	bpm 		    = DEFAULT_BPM;
	bars		    = DEFAULT_BARS;
	beats		    = DEFAULT_BEATS;
	quantize    = DEFAULT_QUANTIZE;
	metronome   = false;

	tickTracker = 0;
	tockTracker = 0;
	tickPlay    = false;
	tockPlay    = false;

	outVol       = DEFAULT_OUT_VOL;
	inVol        = DEFAULT_IN_VOL;
	peakOut      = 0.0f;
	peakIn	     = 0.0f;
	chanInput    = NULL;
	inputTracker = 0;

	actualBeat   = 0;

	/* alloc virtual input channels. vChanInput malloc is done in
	 * updateFrameBars, because of its variable size */

	vChanInput   = NULL;
	vChanInToOut = (float *) malloc(kernelAudio::realBufsize * 2 * sizeof(float));

	pthread_mutex_init(&mutex_recs, NULL);
	pthread_mutex_init(&mutex_chans, NULL);
	pthread_mutex_init(&mutex_plugins, NULL);

	updateFrameBars();
	rewind();
}


/* ------------------------------------------------------------------ */


Channel *Mixer::addChannel(char side, int type) {

	Channel *ch;
	if (type == CHANNEL_SAMPLE)
		ch = new SampleChannel(side);
	else
		ch = new MidiChannel(side);

	while (true) {
		int lockStatus = pthread_mutex_trylock(&mutex_chans);
		if (lockStatus == 0) {
			channels.add(ch);
			pthread_mutex_unlock(&mutex_chans);
			break;
		}
	}

	ch->index = getNewIndex();
	printf("[mixer] channel index=%d added, type=%d, total=%d\n", ch->index, ch->type, channels.size);
	return ch;
}


/* ------------------------------------------------------------------ */


int Mixer::getNewIndex() {

	/* always skip last channel: it's the last one just added */

	if (channels.size == 1)
		return 0;

	int index = 0;
	for (unsigned i=0; i<channels.size-1; i++) {
		if (channels.at(i)->index > index)
			index = channels.at(i)->index;
		}
	index += 1;
	return index;
}


/* ------------------------------------------------------------------ */


int Mixer::deleteChannel(Channel *ch) {
	int lockStatus;
	while (true) {
		lockStatus = pthread_mutex_trylock(&mutex_chans);
		if (lockStatus == 0) {
			channels.del(ch);
			pthread_mutex_unlock(&mutex_chans);
			return 1;
		}
		//else
		//	puts("[mixer::deleteChannel] waiting for mutex...");
	}
}


/* ------------------------------------------------------------------ */


Channel *Mixer::getChannelByIndex(int index) {
	for (unsigned i=0; i<channels.size; i++)
		if (channels.at(i)->index == index)
			return channels.at(i);
	printf("[mixer::getChannelByIndex] channel at index %d not found!\n", index);
	return NULL;
}


/* ------------------------------------------------------------------ */


int Mixer::masterPlay(
	void *out_buf, void *in_buf, unsigned n_frames,
	double streamTime, RtAudioStreamStatus status, void *userData) {
	return G_Mixer.__masterPlay(out_buf, in_buf, n_frames);
}


/* ------------------------------------------------------------------ */


int Mixer::__masterPlay(void *out_buf, void *in_buf, unsigned bufferFrames) {

	if (!ready)
		return 0;

	float *buffer  = ((float *) out_buf);
	float *inBuf   = ((float *) in_buf);
	bufferFrames  *= 2;     // stereo
	peakOut        = 0.0f;  // reset peak calculator
	peakIn         = 0.0f;  // reset peak calculator

	/* always clean each buffer */

	memset(buffer, 0, sizeof(float) * bufferFrames);         // out
	memset(vChanInToOut, 0, sizeof(float) * bufferFrames);   // inToOut vChan

	pthread_mutex_lock(&mutex_chans);
	for (unsigned i=0; i<channels.size; i++)
		channels.at(i)->clear(bufferFrames);
	pthread_mutex_unlock(&mutex_chans);

	for (unsigned j=0; j<bufferFrames; j+=2) {

		if (kernelAudio::inputEnabled) {

			/* input peak calculation (left chan only so far). */

			if (inBuf[j] * inVol > peakIn)
				peakIn = inBuf[j] * inVol;

			/* "hear what you're playing" - process, copy and paste the input buffer
			 * onto the output buffer */

			if (inToOut) {
				vChanInToOut[j]   = inBuf[j]   * inVol;
				vChanInToOut[j+1] = inBuf[j+1] * inVol;
			}
		}

		/* operations to do if the sequencer is running:
		 * - compute quantizer
		 * - time check for LOOP_REPEAT
		 * - reset loops at beat 0
		 * - read recorded actions
		 * - reset actualFrame */

		if (running) {

			/* line in recording */

			if (chanInput != NULL && kernelAudio::inputEnabled) {

				/* delay comp: wait until waitRec reaches delayComp. WaitRec
				 * returns to 0 in mixerHandler, as soon as the recording ends */

				if (waitRec < G_Conf.delayComp)
					waitRec += 2;
				else {
					vChanInput[inputTracker]   += inBuf[j]   * inVol;
					vChanInput[inputTracker+1] += inBuf[j+1] * inVol;
					inputTracker += 2;
					if (inputTracker >= totalFrames)
						inputTracker = 0;
				}
			}

			/* quantizer computations: quantize rewind and all channels. */

			if (quantize > 0 && quanto > 0) {
				if (actualFrame % (quanto) == 0) {   // is quanto!
					if (rewindWait) {
						rewindWait = false;
						rewind();
					}
					pthread_mutex_lock(&mutex_chans);
					for (unsigned k=0; k<channels.size; k++)
						channels.at(k)->quantize(k, actualFrame);
					pthread_mutex_unlock(&mutex_chans);
				}
			}

			/* reset LOOP_REPEAT, if a bar has passed */

			if (actualFrame % framesPerBar == 0 && actualFrame != 0) {
				if (metronome)
					tickPlay = true;

				pthread_mutex_lock(&mutex_chans);
				for (unsigned k=0; k<channels.size; k++)
					channels.at(k)->onBar();
				pthread_mutex_unlock(&mutex_chans);
			}

			/* reset loops on beat 0 */

			if (actualFrame == 0) {
				pthread_mutex_lock(&mutex_chans);
				for (unsigned k=0; k<channels.size; k++)
					channels.at(k)->onZero();
				pthread_mutex_unlock(&mutex_chans);
			}

			/* reading all actions recorded */

			pthread_mutex_lock(&mutex_recs);
			for (unsigned y=0; y<recorder::frames.size; y++) {
				if (recorder::frames.at(y) == actualFrame) {
					for (unsigned z=0; z<recorder::global.at(y).size; z++) {
						int index   = recorder::global.at(y).at(z)->chan;
						Channel *ch = getChannelByIndex(index);
						ch->parseAction(recorder::global.at(y).at(z), actualFrame);
					}
					break;
				}
			}
			pthread_mutex_unlock(&mutex_recs);

			/* increase actualFrame */

			actualFrame += 2;

			/* if actualFrame > totalFrames the sequencer returns to frame 0,
			 * beat 0. This must be the last operation. */

			if (actualFrame > totalFrames) {
				actualFrame = 0;
				actualBeat  = 0;
			}
			else
			if (actualFrame % framesPerBeat == 0 && actualFrame > 0) {
				actualBeat++;

				/* avoid tick and tock to overlap when a new bar has passed (which
				 * is also a beat) */

				if (metronome && !tickPlay)
					tockPlay = true;
			}
		} // if (running)

		/* sum channels, CHANNEL_SAMPLE only */

		pthread_mutex_lock(&mutex_chans);
		for (unsigned k=0; k<channels.size; k++) {
			if (channels.at(k)->type == CHANNEL_SAMPLE)
				((SampleChannel*)channels.at(k))->sum(j, running);
		}
		pthread_mutex_unlock(&mutex_chans);

		/* metronome play */
		/** FIXME - move this one after the peak meter calculation */

		if (tockPlay) {
			buffer[j]   += tock[tockTracker];
			buffer[j+1] += tock[tockTracker];
			tockTracker++;
			if (tockTracker >= TICKSIZE-1) {
				tockPlay    = false;
				tockTracker = 0;
			}
		}
		if (tickPlay) {
			buffer[j]   += tick[tickTracker];
			buffer[j+1] += tick[tickTracker];
			tickTracker++;
			if (tickTracker >= TICKSIZE-1) {
				tickPlay    = false;
				tickTracker = 0;
			}
		}
	} // end loop J

	/* final loop: sum virtual channels and process plugins. */

	pthread_mutex_lock(&mutex_chans);
	for (unsigned k=0; k<channels.size; k++)
		channels.at(k)->process(buffer, bufferFrames);
	pthread_mutex_unlock(&mutex_chans);

	/* processing fxs master in & out, if any. */

#ifdef WITH_VST
	pthread_mutex_lock(&mutex_plugins);
	G_PluginHost.processStack(buffer, PluginHost::MASTER_OUT);
	G_PluginHost.processStack(vChanInToOut, PluginHost::MASTER_IN);
	pthread_mutex_unlock(&mutex_plugins);
#endif

	/* post processing master fx + peak calculation. */

	for (unsigned j=0; j<bufferFrames; j+=2) {

		/* merging vChanInToOut, if enabled */

		if (inToOut) {
			buffer[j]   += vChanInToOut[j];
			buffer[j+1] += vChanInToOut[j+1];
		}

		buffer[j]   *= outVol;
		buffer[j+1] *= outVol;

		/* computes the peak for the left channel (so far). */

		if (buffer[j] > peakOut)
			peakOut = buffer[j];

		if (G_Conf.limitOutput) {
			if (buffer[j] > 1.0f)
				buffer[j] = 1.0f;
			else if (buffer[j] < -1.0f)
				buffer[j] = -1.0f;

			if (buffer[j+1] > 1.0f)
				buffer[j+1] = 1.0f;
			else if (buffer[j+1] < -1.0f)
				buffer[j+1] = -1.0f;
		}
	}

	return 0;
}


/* ------------------------------------------------------------------ */


void Mixer::updateFrameBars() {

	/* seconds ....... total time of play (in seconds) of the whole
	 *                 sequencer. 60 / bpm == how many seconds lasts one bpm
	 * totalFrames ... number of frames in the whole sequencer, x2 because
	 * 								 it's stereo
	 * framesPerBar .. n. of frames within a bar
	 * framesPerBeat . n. of frames within a beat */

	float seconds  = (60.0f / bpm) * beats;
	totalFrames    = G_Conf.samplerate * seconds * 2;
	framesPerBar   = totalFrames / bars;
	framesPerBeat  = totalFrames / beats;
	framesPerBeats = framesPerBeat * beats;

	/* big troubles if frames are odd. */

	if (totalFrames % 2 != 0)
		totalFrames--;
	if (framesPerBar % 2 != 0)
		framesPerBar--;
	if (framesPerBeat % 2 != 0)
		framesPerBeat--;
	if (framesPerBeats % 2 != 0)
		framesPerBeats--;

	updateQuanto();

	/* realloc input virtual channel, if not NULL. TotalFrames is changed! */

	if (vChanInput != NULL)
		free(vChanInput);
	vChanInput = (float*) malloc(totalFrames * sizeof(float));
	if (!vChanInput)
		printf("[Mixer] vChanInput realloc error!");
}


/* ------------------------------------------------------------------ */


int Mixer::close() {
	running = false;
	while (channels.size > 0)
		deleteChannel(channels.at(0));
	free(vChanInput);
	free(vChanInToOut);
	return 1;
}


/* ------------------------------------------------------------------ */


bool Mixer::isSilent() {
	for (unsigned i=0; i<channels.size; i++)
		if (channels.at(i)->status == STATUS_PLAY)
			return false;
	return true;
}


/* ------------------------------------------------------------------ */


void Mixer::rewind() {

	actualFrame = 0;
	actualBeat  = 0;

	if (running)
		for (unsigned i=0; i<channels.size; i++)
			channels.at(i)->rewind();
}


/* ------------------------------------------------------------------ */


void Mixer::updateQuanto() {

	/* big troubles if frames are odd. */

	if (quantize != 0)
		quanto = framesPerBeat / quantize;
	if (quanto % 2 != 0)
		quanto++;
}


/* ------------------------------------------------------------------ */


bool Mixer::hasLogicalSamples() {
	for (unsigned i=0; i<channels.size; i++)
		if (channels.at(i)->type == CHANNEL_SAMPLE)
			if (((SampleChannel*)channels.at(i))->wave)
				if (((SampleChannel*)channels.at(i))->wave->isLogical)
					return true;
	return false;
}


/* ------------------------------------------------------------------ */


bool Mixer::hasEditedSamples() {
	for (unsigned i=0; i<channels.size; i++)
		if (channels.at(i)->type == CHANNEL_SAMPLE)
			if (((SampleChannel*)channels.at(i))->wave)
				if (((SampleChannel*)channels.at(i))->wave->isEdited)
					return true;
	return false;
}


/* ------------------------------------------------------------------ */


bool Mixer::mergeVirtualInput() {
	if (vChanInput == NULL) {
		puts("[Mixer] virtual input channel not alloc'd");
		return false;
	}
	else {
#ifdef WITH_VST
		G_PluginHost.processStackOffline(vChanInput, PluginHost::MASTER_IN, 0, totalFrames);
#endif
		int numFrames = totalFrames*sizeof(float);
		memcpy(chanInput->wave->data, vChanInput, numFrames);
		memset(vChanInput, 0, numFrames); // clear vchan
		return true;
	}
}
