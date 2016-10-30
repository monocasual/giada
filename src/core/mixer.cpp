/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * mixer
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#include "../utils/log.h"
#include "wave.h"
#include "recorder.h"
#include "pluginHost.h"
#include "patch_DEPR_.h"
#include "conf.h"
#include "mixerHandler.h"
#include "channel.h"
#include "sampleChannel.h"
#include "midiChannel.h"
#include "kernelMidi.h"
#include "mixer.h"


extern Mixer 			 G_Mixer;
extern Recorder    G_Recorder;
extern KernelMidi  G_KernelMidi;
extern MidiMapConf G_MidiMap;
extern Patch_DEPR_ G_Patch_DEPR_;
extern Conf				 G_Conf;
#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif


Mixer::Mixer()
	: vChanInput(NULL),
		vChanInToOut(NULL)
{}


/* -------------------------------------------------------------------------- */


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


/* -------------------------------------------------------------------------- */


void Mixer::init()
{
	quanto      = 1;
	docross     = false;
	rewindWait  = false;
	running     = false;
	recording   = false;
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
	inputTracker = 0;

	actualBeat    = 0;

	midiTCstep    = 0;
	midiTCrate    = (G_Conf.samplerate / G_Conf.midiTCfps) * 2;  // dealing with stereo vals
	midiTCframes  = 0;
	midiTCseconds = 0;
	midiTCminutes = 0;
	midiTChours   = 0;

	/* alloc virtual input channels. vChanInput malloc is done in
	 * updateFrameBars, because of its variable size */
	/** TODO - set kernelAudio::realBufsize * 2 as private member */

	vChanInput   = NULL;
	vChanInToOut = (float *) malloc(kernelAudio::realBufsize * 2 * sizeof(float));

	pthread_mutex_init(&mutex_recs, NULL);
	pthread_mutex_init(&mutex_chans, NULL);
	pthread_mutex_init(&mutex_plugins, NULL);

	updateFrameBars();
	rewind();
}


/* -------------------------------------------------------------------------- */


Channel *Mixer::addChannel(int type)
{
	Channel *ch;
	int bufferSize = kernelAudio::realBufsize*2;

	if (type == CHANNEL_SAMPLE)
		ch = new SampleChannel(bufferSize, &G_MidiMap);
	else
		ch = new MidiChannel(bufferSize, &G_MidiMap);

#ifdef WITH_VST
	ch->setPluginHost(&G_PluginHost);
#endif

	while (true) {
		int lockStatus = pthread_mutex_trylock(&mutex_chans);
		if (lockStatus == 0) {
			channels.push_back(ch);
			pthread_mutex_unlock(&mutex_chans);
			break;
		}
	}

	ch->index = getNewIndex();
	gu_log("[mixer] channel index=%d added, type=%d, total=%d\n", ch->index, ch->type, channels.size());
	return ch;
}


/* -------------------------------------------------------------------------- */


int Mixer::getNewIndex()
{
	/* always skip last channel: it's the last one just added */

	if (channels.size() == 1)
		return 0;

	int index = 0;
	for (unsigned i=0; i<channels.size()-1; i++) {
		if (channels.at(i)->index > index)
			index = channels.at(i)->index;
		}
	index += 1;
	return index;
}


/* -------------------------------------------------------------------------- */


int Mixer::deleteChannel(Channel *ch)
{
	int index = -1;
	for (unsigned i=0; i<channels.size(); i++) {
		if (channels.at(i) == ch) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		gu_log("[Mixer::deleteChannel] unable to find Channel %d for deletion!\n", ch->index);
		return 0;
	}

	int lockStatus;
	while (true) {
		lockStatus = pthread_mutex_trylock(&mutex_chans);
		if (lockStatus == 0) {
			channels.erase(channels.begin() + index);
			delete ch;
			pthread_mutex_unlock(&mutex_chans);
			return 1;
		}
		//else
		//	gu_log("[mixer::deleteChannel] waiting for mutex...\n");
	}
}


/* -------------------------------------------------------------------------- */


Channel *Mixer::getChannelByIndex(int index)
{
	for (unsigned i=0; i<channels.size(); i++)
		if (channels.at(i)->index == index)
			return channels.at(i);
	gu_log("[mixer::getChannelByIndex] channel at index %d not found!\n", index);
	return NULL;
}


/* -------------------------------------------------------------------------- */


void Mixer::sendMIDIsync()
{
	if (G_Conf.midiSync == MIDI_SYNC_CLOCK_M) {
		if (actualFrame % (framesPerBeat/24) == 0)
			G_KernelMidi.send(MIDI_CLOCK, -1, -1);
	}
	else
	if (G_Conf.midiSync == MIDI_SYNC_MTC_M) {

		/* check if a new timecode frame has passed. If so, send MIDI TC
		 * quarter frames. 8 quarter frames, divided in two branches:
		 * 1-4 and 5-8. We check timecode frame's parity: if even, send
		 * range 1-4, if odd send 5-8. */

		if (actualFrame % midiTCrate == 0) {

			/* frame low nibble
			 * frame high nibble
			 * seconds low nibble
			 * seconds high nibble */

			if (midiTCframes % 2 == 0) {
				G_KernelMidi.send(MIDI_MTC_QUARTER, (midiTCframes & 0x0F)  | 0x00, -1);
				G_KernelMidi.send(MIDI_MTC_QUARTER, (midiTCframes >> 4)    | 0x10, -1);
				G_KernelMidi.send(MIDI_MTC_QUARTER, (midiTCseconds & 0x0F) | 0x20, -1);
				G_KernelMidi.send(MIDI_MTC_QUARTER, (midiTCseconds >> 4)   | 0x30, -1);
			}

			/* minutes low nibble
			 * minutes high nibble
			 * hours low nibble
			 * hours high nibble SMPTE frame rate */

			else {
				G_KernelMidi.send(MIDI_MTC_QUARTER, (midiTCminutes & 0x0F) | 0x40, -1);
				G_KernelMidi.send(MIDI_MTC_QUARTER, (midiTCminutes >> 4)   | 0x50, -1);
				G_KernelMidi.send(MIDI_MTC_QUARTER, (midiTChours & 0x0F)   | 0x60, -1);
				G_KernelMidi.send(MIDI_MTC_QUARTER, (midiTChours >> 4)     | 0x70, -1);
			}

			midiTCframes++;

			/* check if total timecode frames are greater than timecode fps:
			 * if so, a second has passed */

			if (midiTCframes > G_Conf.midiTCfps) {
				midiTCframes = 0;
				midiTCseconds++;
				if (midiTCseconds >= 60) {
					midiTCminutes++;
					midiTCseconds = 0;
					if (midiTCminutes >= 60) {
						midiTChours++;
						midiTCminutes = 0;
					}
				}
				//gu_log("%d:%d:%d:%d\n", midiTChours, midiTCminutes, midiTCseconds, midiTCframes);
			}
		}
	}
}


/* -------------------------------------------------------------------------- */


void Mixer::sendMIDIrewind()
{
	midiTCframes  = 0;
	midiTCseconds = 0;
	midiTCminutes = 0;
	midiTChours   = 0;

	/* For cueing the slave to a particular start point, Quarter Frame
	 * messages are not used. Instead, an MTC Full Frame message should
	 * be sent. The Full Frame is a SysEx message that encodes the entire
	 * SMPTE time in one message */

	if (G_Conf.midiSync == MIDI_SYNC_MTC_M) {
		G_KernelMidi.send(MIDI_SYSEX, 0x7F, 0x00);  // send msg on channel 0
		G_KernelMidi.send(0x01, 0x01, 0x00);        // hours 0
		G_KernelMidi.send(0x00, 0x00, 0x00);        // mins, secs, frames 0
		G_KernelMidi.send(MIDI_EOX, -1, -1);        // end of sysex
	}
}

/* -------------------------------------------------------------------------- */


int Mixer::masterPlay(
	void *out_buf, void *in_buf, unsigned n_frames,
	double streamTime, RtAudioStreamStatus status, void *userData)
{
	return G_Mixer.__masterPlay(out_buf, in_buf, n_frames);
}


/* -------------------------------------------------------------------------- */


int Mixer::__masterPlay(void *out_buf, void *in_buf, unsigned bufferSize)
{
	if (!ready)
		return 0;

	float *outBuf = ((float *) out_buf);
	float *inBuf  = ((float *) in_buf);
	bufferSize   *= 2;     // stereo
	peakOut       = 0.0f;  // reset peak calculator
	peakIn        = 0.0f;  // reset peak calculator

	clearAllBuffers(outBuf, bufferSize);

	for (unsigned j=0; j<bufferSize; j+=2) {
		processLineIn(inBuf, j);
		if (running) {
			lineInRec(inBuf, j);
			doQuantize(j);
			testBar(j);
			testFirstBeat(j);
			readActions(j);
			actualFrame += 2;
			testLastBeat();  // this test must be the last one
			sendMIDIsync();
		}
		sumChannels(j);
		renderMetronome(outBuf, j); // FIXME - move this one after the peak meter calculation
	}

	renderIO(outBuf, inBuf);

	/* post processing */

	for (unsigned j=0; j<bufferSize; j+=2) {
		finalizeOutput(outBuf, j);
		computePeak(outBuf, j);
		if (G_Conf.limitOutput)
			limitOutput(outBuf, j);
	}

	return 0;
}


/* -------------------------------------------------------------------------- */


void Mixer::updateFrameBars()
{
	/* seconds ....... total time of play (in seconds) of the whole
	 *                 sequencer. 60 / bpm == how many seconds lasts one bpm
	 * totalFrames ... number of frames in the whole sequencer, x2 because
	 * 								 it's stereo
	 * framesPerBar .. n. of frames within a bar
	 * framesPerBeat . n. of frames within a beat */

	float seconds     = (60.0f / bpm) * beats;
	totalFrames       = G_Conf.samplerate * seconds * 2;
	framesPerBar      = totalFrames / bars;
	framesPerBeat     = totalFrames / beats;
	framesInSequencer = framesPerBeat * MAX_BEATS;

	/* big troubles if frames are odd. */

	if (totalFrames % 2 != 0)
		totalFrames--;
	if (framesPerBar % 2 != 0)
		framesPerBar--;
	if (framesPerBeat % 2 != 0)
		framesPerBeat--;

	updateQuanto();

	/* realloc input virtual channel, if not NULL. TotalFrames is changed! */

	if (vChanInput != NULL)
		free(vChanInput);
	vChanInput = (float*) malloc(totalFrames * sizeof(float));
	if (!vChanInput)
		gu_log("[Mixer] vChanInput realloc error!\n");
}


/* -------------------------------------------------------------------------- */


int Mixer::close()
{
	running = false;
	while (channels.size() > 0)
		deleteChannel(channels.at(0));

	if (vChanInput) {
		free(vChanInput);
		vChanInput = NULL;
	}
	if (vChanInToOut) {
		free(vChanInToOut);
		vChanInToOut = NULL;
	}
	return 1;
}


/* -------------------------------------------------------------------------- */


bool Mixer::isSilent()
{
	for (unsigned i=0; i<channels.size(); i++)
		if (channels.at(i)->status == STATUS_PLAY)
			return false;
	return true;
}


/* -------------------------------------------------------------------------- */


void Mixer::rewind()
{
	actualFrame = 0;
	actualBeat  = 0;

	if (running)
		for (unsigned i=0; i<channels.size(); i++)
			channels.at(i)->rewind();

	sendMIDIrewind();
}


/* -------------------------------------------------------------------------- */


void Mixer::updateQuanto()
{
	/* big troubles if frames are odd. */

	if (quantize != 0)
		quanto = framesPerBeat / quantize;
	if (quanto % 2 != 0)
		quanto++;
}


/* -------------------------------------------------------------------------- */


bool Mixer::hasLogicalSamples()
{
	for (unsigned i=0; i<channels.size(); i++)
		if (channels.at(i)->type == CHANNEL_SAMPLE)
			if (((SampleChannel*)channels.at(i))->wave)
				if (((SampleChannel*)channels.at(i))->wave->isLogical)
					return true;
	return false;
}


/* -------------------------------------------------------------------------- */


bool Mixer::hasEditedSamples()
{
	for (unsigned i=0; i<channels.size(); i++)
		if (channels.at(i)->type == CHANNEL_SAMPLE)
			if (((SampleChannel*)channels.at(i))->wave)
				if (((SampleChannel*)channels.at(i))->wave->isEdited)
					return true;
	return false;
}


/* -------------------------------------------------------------------------- */


void Mixer::mergeVirtualInput()
{
	for (unsigned i=0; i<channels.size(); i++) {
		if (channels.at(i)->type == CHANNEL_MIDI)
			continue;
		SampleChannel *ch = (SampleChannel*) channels.at(i);
		if (ch->armed)
			memcpy(ch->wave->data, vChanInput, totalFrames * sizeof(float));
	}
	memset(vChanInput, 0, totalFrames * sizeof(float)); // clear vchan
}


/* -------------------------------------------------------------------------- */


void Mixer::lineInRec(float *inBuf, unsigned frame)
{
	if (/* TODO - !hasArmedChannels() || */ !kernelAudio::inputEnabled)
	 	return;

	/* delay comp: wait until waitRec reaches delayComp. WaitRec
	 * returns to 0 in mixerHandler, as soon as the recording ends */

	if (waitRec < G_Conf.delayComp) {
		waitRec += 2;
		return;
	}

	vChanInput[inputTracker]   += inBuf[frame]   * inVol;
	vChanInput[inputTracker+1] += inBuf[frame+1] * inVol;
	inputTracker += 2;
	if (inputTracker >= totalFrames)
		inputTracker = 0;
}


/* -------------------------------------------------------------------------- */


void Mixer::processLineIn(float *inBuf, unsigned frame)
{
	if (!kernelAudio::inputEnabled)
		return;

	/* input peak calculation (left chan only so far). */

	if (inBuf[frame] * inVol > peakIn)
		peakIn = inBuf[frame] * inVol;

	/* "hear what you're playing" - process, copy and paste the input buffer
	 * onto the output buffer */

	if (inToOut) {
		vChanInToOut[frame]   = inBuf[frame]   * inVol;
		vChanInToOut[frame+1] = inBuf[frame+1] * inVol;
	}
}


/* -------------------------------------------------------------------------- */


void Mixer::readActions(unsigned frame)
{
	pthread_mutex_lock(&mutex_recs);
	for (unsigned i=0; i<G_Recorder.frames.size(); i++) {
		if (G_Recorder.frames.at(i) == actualFrame) {
			for (unsigned j=0; j<G_Recorder.global.at(i).size(); j++) {
				int index   = G_Recorder.global.at(i).at(j)->chan;
				Channel *ch = getChannelByIndex(index);
				ch->parseAction(G_Recorder.global.at(i).at(j), frame, actualFrame, quantize, running);
			}
			break;
		}
	}
	pthread_mutex_unlock(&mutex_recs);
}


/* -------------------------------------------------------------------------- */


void Mixer::doQuantize(unsigned frame)
{
	if (quantize < 0 || quanto <= 0) // if quantizer disabled
		return;
	if (actualFrame % (quanto) != 0) // if a quanto has not passed yet
		return;

	if (rewindWait) {
		rewindWait = false;
		rewind();
	}
	pthread_mutex_lock(&mutex_chans);
	for (unsigned i=0; i<channels.size(); i++)
		channels.at(i)->quantize(i, frame, actualFrame);  // j == localFrame
	pthread_mutex_unlock(&mutex_chans);
}


/* -------------------------------------------------------------------------- */


void Mixer::testBar(unsigned frame)
{
	if (actualFrame % framesPerBar != 0 || actualFrame == 0)
		return;

	if (metronome)
		tickPlay = true;

	pthread_mutex_lock(&mutex_chans);
	for (unsigned k=0; k<channels.size(); k++)
		channels.at(k)->onBar(frame);
	pthread_mutex_unlock(&mutex_chans);
}


/* -------------------------------------------------------------------------- */


void Mixer::testFirstBeat(unsigned frame)
{
	if (actualFrame != 0)
		return;
	pthread_mutex_lock(&mutex_chans);
	for (unsigned k=0; k<channels.size(); k++)
		channels.at(k)->onZero(frame, G_Conf.recsStopOnChanHalt);
	pthread_mutex_unlock(&mutex_chans);
}


/* -------------------------------------------------------------------------- */


void Mixer::testLastBeat()
{
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
}


/* -------------------------------------------------------------------------- */


void Mixer::sumChannels(unsigned frame)
{
	pthread_mutex_lock(&mutex_chans);
	for (unsigned k=0; k<channels.size(); k++) {
		if (channels.at(k)->type == CHANNEL_SAMPLE)
			((SampleChannel*)channels.at(k))->sum(frame, running);
	}
	pthread_mutex_unlock(&mutex_chans);
}


/* -------------------------------------------------------------------------- */


void Mixer::renderMetronome(float *outBuf, unsigned frame)
{
	if (tockPlay) {
		outBuf[frame]   += tock[tockTracker];
		outBuf[frame+1] += tock[tockTracker];
		tockTracker++;
		if (tockTracker >= TICKSIZE-1) {
			tockPlay    = false;
			tockTracker = 0;
		}
	}
	if (tickPlay) {
		outBuf[frame]   += tick[tickTracker];
		outBuf[frame+1] += tick[tickTracker];
		tickTracker++;
		if (tickTracker >= TICKSIZE-1) {
			tickPlay    = false;
			tickTracker = 0;
		}
	}
}


/* -------------------------------------------------------------------------- */


void Mixer::renderIO(float *outBuf, float *inBuf)
{
	pthread_mutex_lock(&mutex_chans);
	for (unsigned k=0; k<channels.size(); k++)
		channels.at(k)->process(outBuf, inBuf);
	pthread_mutex_unlock(&mutex_chans);

#ifdef WITH_VST
	pthread_mutex_lock(&mutex_plugins);
	G_PluginHost.processStack(outBuf, PluginHost::MASTER_OUT);
	G_PluginHost.processStack(vChanInToOut, PluginHost::MASTER_IN);
	pthread_mutex_unlock(&mutex_plugins);
#endif
}


/* -------------------------------------------------------------------------- */


void Mixer::computePeak(float *outBuf, unsigned frame)
{
	/* TODO it takes into account only left channel so far! */
	if (outBuf[frame] > peakOut)
		peakOut = outBuf[frame];
}


/* -------------------------------------------------------------------------- */


void Mixer::limitOutput(float *outBuf, unsigned frame)
{
	if (outBuf[frame] > 1.0f)
		outBuf[frame] = 1.0f;
	else
	if (outBuf[frame] < -1.0f)
		outBuf[frame] = -1.0f;

	if (outBuf[frame+1] > 1.0f)
		outBuf[frame+1] = 1.0f;
	else
	if (outBuf[frame+1] < -1.0f)
		outBuf[frame+1] = -1.0f;
}


/* -------------------------------------------------------------------------- */


void Mixer::finalizeOutput(float *outBuf, unsigned frame)
{
	/* merge vChanInToOut, if enabled */

	if (inToOut) {
		outBuf[frame]   += vChanInToOut[frame];
		outBuf[frame+1] += vChanInToOut[frame+1];
	}
	outBuf[frame]   *= outVol;
	outBuf[frame+1] *= outVol;
}


/* -------------------------------------------------------------------------- */


void Mixer::clearAllBuffers(float *outBuf, unsigned bufferSize)
{
	memset(outBuf, 0, sizeof(float) * bufferSize);         // out
	memset(vChanInToOut, 0, sizeof(float) * bufferSize);   // inToOut vChan

	pthread_mutex_lock(&mutex_chans);
	for (unsigned i=0; i<channels.size(); i++)
		channels.at(i)->clear();
	pthread_mutex_unlock(&mutex_chans);
}
