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


channel *Mixer::addChannel(char side, int type) {
	channel *ch = (channel*) malloc(sizeof(channel));
	if (!ch) {
		printf("[mixer] unable to alloc memory for channel struct\n");
		return NULL;
	}

	/* virtual channel malloc. We use kernelAudio::realBufsize, the real
	 * buffer size from the soundcard. G_Conf.bufferSize may be wrong or
	 * useless, especially when JACK is running. */

	ch->vChan = (float *) malloc(kernelAudio::realBufsize * 2 * sizeof(float));
	if (!ch->vChan) {
		printf("[mixer] unable to alloc memory for this vChan\n");
		return NULL;
	}

	while (true) {
		int lockStatus = pthread_mutex_trylock(&mutex_chans);
		if (lockStatus == 0) {
			channels.add(ch);
			pthread_mutex_unlock(&mutex_chans);
			break;
		}
	}

	initChannel(ch);

	ch->index = getNewIndex();
	ch->side  = side;
	ch->type  = type;

	if (type == CHANNEL_MIDI) {
		ch->status = STATUS_OFF;  // for SAMPLES, init status is EMPTY
		ch->readActions = true;
	}

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


void Mixer::pushChannel(Wave *w, channel *ch) {
	ch->wave = w;
	ch->status = STATUS_OFF;
	ch->start     = 0;
	ch->startTrue = 0;
	ch->end       = ch->wave->size;
	ch->endTrue   = ch->wave->size;
}


/* ------------------------------------------------------------------ */


void Mixer::initChannel(channel *ch) {
	ch->wave        = NULL;
	ch->key         = 0;
	ch->tracker     = 0;
	ch->status      = STATUS_EMPTY;
	ch->start       = 0;
	ch->startTrue   = 0;
	ch->end         = 0;
	ch->endTrue     = 0;
	ch->solo        = false;
	ch->mute        = false;
	ch->mute_i      = false;
	ch->mute_s      = false;
	ch->mode        = DEFAULT_CHANMODE;
	ch->volume      = DEFAULT_VOL;
	ch->volume_i    = 1.0f;
	ch->volume_d    = 0.0f;
	ch->pitch       = gDEFAULT_PITCH;
	ch->boost       = 1.0f;
	ch->panLeft     = 1.0f;
	ch->panRight    = 1.0f;
	ch->qWait	      = false;
	ch->recStatus   = REC_STOPPED;
	ch->fadein      = 1.0f;
	ch->fadeoutOn   = false;
	ch->fadeoutVol  = 1.0f;
	ch->fadeoutStep = DEFAULT_FADEOUT_STEP;

	ch->readActions = false;
	ch->hasActions  = false;

	/* call gVector constructor with p, and using it as the real gVector */
	/** FIXME - is it really useful??? */

	gVector <class Plugin *> p;
	ch->plugins     = p;

	ch->midiOut     = false;
	ch->midiOutChan = MIDI_CHANS[0];

#ifdef WITH_VST // init VstEvents stack
	G_PluginHost.freeVstMidiEvents(ch, true);
#endif
}


/* ------------------------------------------------------------------ */


int Mixer::deleteChannel(channel *ch) {
	int lockStatus;
	while (true) {
		lockStatus = pthread_mutex_trylock(&mutex_chans);
		if (lockStatus == 0) {
			ch->status = STATUS_OFF;
			int i = getChannelIndex(ch);
			if (ch->wave) {
				delete ch->wave;
				ch->wave = NULL;
			}
			free(ch->vChan);
			free(ch);
			channels.del(i);
			pthread_mutex_unlock(&mutex_chans);
			return 1;
		}
		//else
		//	puts("[mixer::deleteChannel] waiting for mutex...");
	}
}


/* ------------------------------------------------------------------ */


void Mixer::freeChannel(channel *ch) {
	ch->status = STATUS_OFF;
	if (ch->wave != NULL) {
		delete ch->wave;
		ch->wave   = NULL;
	}
	ch->status = STATUS_EMPTY;
}



/* ------------------------------------------------------------------ */


void Mixer::chanStop(channel *ch) {
	ch->status = STATUS_OFF;
	chanReset(ch);
}


/* ------------------------------------------------------------------ */


int Mixer::getChannelIndex(channel *ch) {
	for (unsigned i=0; i<channels.size; i++)
		if (channels.at(i)->index == ch->index)
			return i;
	return -1;
}


/* ------------------------------------------------------------------ */


channel *Mixer::getChannelByIndex(int index) {
	for (unsigned i=0; i<channels.size; i++)
		if (channels.at(i)->index == index)
			return channels.at(i);
	printf("[mixer::getChannelByIndex] channel at index %d not found!\n", index);
	return NULL;
}


/* ------------------------------------------------------------------ */


void Mixer::chanReset(channel *ch)	{
	ch->tracker = ch->start;
	ch->mute_i  = false;
}


/* ------------------------------------------------------------------ */


void Mixer::fadein(channel *ch, bool internal) {

	/* remove mute before fading in */

	if (internal) ch->mute_i = false;
	else          ch->mute   = false;
	ch->fadein = 0.0f;
}


/* ------------------------------------------------------------------ */


void Mixer::fadeout(channel *ch, int actionPostFadeout) {
	calcFadeoutStep(ch);
	ch->fadeoutOn   = true;
	ch->fadeoutVol  = 1.0f;
	ch->fadeoutType = FADEOUT;
	ch->fadeoutEnd	 = actionPostFadeout;
}


/* ------------------------------------------------------------------ */


void Mixer::xfade(channel *ch) {
	calcFadeoutStep(ch);
	ch->fadeoutOn      = true;
	ch->fadeoutVol     = 1.0f;
	ch->fadeoutTracker = ch->tracker;
	ch->fadeoutType    = XFADE;
	chanReset(ch);
}


/* ------------------------------------------------------------------ */


int Mixer::getChanPos(channel *ch)	{
	if (ch->status & ~(STATUS_EMPTY | STATUS_MISSING | STATUS_OFF))  // if chanStatus[ch] is not (...)
		return ch->tracker - ch->start;
	else
		return -1;
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
		memset(channels.at(i)->vChan, 0, sizeof(float) * bufferFrames);     // vchans
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

			/* quantizer computations */

			if (quantize > 0 && quanto > 0) {

				bool isQuanto = actualFrame % (quanto) == 0 ? true : false;

				/* rewind quantization */

				if (isQuanto && rewindWait) {
					rewindWait = false;
					rewind();
				}

				pthread_mutex_lock(&mutex_chans);
				for (unsigned k=0; k<channels.size; k++) {
					channel *ch = channels.at(k);
					if (ch->wave == NULL)
						continue;
					if (isQuanto && (ch->mode & SINGLE_ANY) && ch->qWait == true)	{

						/* no fadeout if the sample starts for the first time (from a STATUS_OFF), it would
						 * be meaningless. */

						if (ch->status == STATUS_OFF) {
							ch->status = STATUS_PLAY;
							ch->qWait  = false;
						}
						else
							xfade(ch);

						/* this is the moment in which we record the keypress, if the quantizer is on.
						 * SINGLE_PRESS needs overdub */

						if (recorder::canRec(ch)) {
							if (ch->mode == SINGLE_PRESS)
								recorder::startOverdub(k, ACTION_KEYS, actualFrame);
							else
								recorder::rec(k, ACTION_KEYPRESS, actualFrame);
						}
					}
				}
				pthread_mutex_unlock(&mutex_chans);
			}

			/* reset LOOP_REPEAT, if a bar has passed */

			if (actualFrame % framesPerBar == 0 && actualFrame != 0) {
				if (metronome)
					tickPlay = true;

				pthread_mutex_lock(&mutex_chans);
				for (unsigned k=0; k<channels.size; k++) {
					channel *ch = channels.at(k);
					if (ch->wave == NULL)
						continue;
					if (ch->mode == LOOP_REPEAT && ch->status == STATUS_PLAY)
						xfade(ch);
				}
				pthread_mutex_unlock(&mutex_chans);
			}

			/* reset sample on beat 0 */

			if (actualFrame == 0)
				updateChansOnSampleZero();

			/* reading all actions recorded */

			readActions();

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

	/* sum channels */

		pthread_mutex_lock(&mutex_chans);
		for (unsigned k=0; k<channels.size; k++) {
			channel *ch = channels.at(k);

			/** TODO - distinguish between CHANNEL_MIDI and CHANNEL_SAMPLE */

			if (ch->wave == NULL)
				continue;

			if (ch->status & (STATUS_PLAY | STATUS_ENDING)) {

				if (ch->tracker <= channels.at(k)->end) {

					/* ctp is chanTracker, pitch affected */

					unsigned ctp = ch->tracker * ch->pitch;

					/* fade in */

					if (ch->fadein <= 1.0f)
						ch->fadein += 0.01f;		/// FIXME - remove the hardcoded value

					/* volume envelope, only if seq is running */

					if (running) {
						ch->volume_i += ch->volume_d;
						if (ch->volume_i < 0.0f)
							ch->volume_i = 0.0f;
						else
						if (ch->volume_i > 1.0f)
							ch->volume_i = 1.0f;
					}

					/* fadeout process (both fadeout and xfade) */

					if (ch->fadeoutOn) {
						if (ch->fadeoutVol >= 0.0f) { // fadeout ongoing

							float v = ch->volume_i * ch->boost;

							if (ch->fadeoutType == XFADE) {

								/* ftp is fadeoutTracker affected by pitch */

								unsigned ftp = ch->fadeoutTracker * ch->pitch;

								ch->vChan[j]   += ch->wave->data[ftp]   * ch->fadeoutVol * v;
								ch->vChan[j+1] += ch->wave->data[ftp+1] * ch->fadeoutVol * v;

								ch->vChan[j]   += ch->wave->data[ctp]   * v;
								ch->vChan[j+1] += ch->wave->data[ctp+1] * v;

							}
							else { // FADEOUT
								ch->vChan[j]   += ch->wave->data[ctp]   * ch->fadeoutVol * v;
								ch->vChan[j+1] += ch->wave->data[ctp+1] * ch->fadeoutVol * v;
							}

							ch->fadeoutVol     -= ch->fadeoutStep;
							ch->fadeoutTracker += 2;
						}
						else {  // fadeout end
							ch->fadeoutOn  = false;
							ch->fadeoutVol = 1.0f;

							/* QWait ends with the end of the xfade */

							if (ch->fadeoutType == XFADE) {
								ch->qWait = false;
							}
							else {
								if (ch->fadeoutEnd == DO_MUTE)
									ch->mute = true;
								else
								if (ch->fadeoutEnd == DO_MUTE_I)
									ch->mute_i = true;
								else              // DO_STOP
									chanStop(ch);
							}

							/* we must append another frame in the buffer when the fadeout
							 * ends: there's a gap here which would clip otherwise */

							ch->vChan[j]   = ch->vChan[j-2];
							ch->vChan[j+1] = ch->vChan[j-1];
						}
					}  // no fadeout to do
					else {
						if (!ch->mute && !ch->mute_i) {
							float v = ch->volume_i * ch->fadein * ch->boost;
							ch->vChan[j]   += ch->wave->data[ctp]   * v;
							ch->vChan[j+1] += ch->wave->data[ctp+1] * v;
						}
					}

					ch->tracker += 2;

					/* check for end of samples. SINGLE_ENDLESS runs forever unless
					 * it's in ENDING mode */

					if (ch->tracker >= ch->end) {

						chanReset(ch);

						if (ch->mode & (SINGLE_BASIC | SINGLE_PRESS | SINGLE_RETRIG) ||
						   (ch->mode == SINGLE_ENDLESS && ch->status == STATUS_ENDING))
						{
							ch->status = STATUS_OFF;
						}

						/// FIXME - unify these
						/* stop loops when the seq is off */

						if ((ch->mode & LOOP_ANY) && !running)
							ch->status = STATUS_OFF;

						/* temporary stop LOOP_ONCE not in ENDING status, otherwise they
						 * would return in wait, losing the ENDING status */

						if (ch->mode == LOOP_ONCE && ch->status != STATUS_ENDING)
							ch->status = STATUS_WAIT;
					}
				}
			}
		} // end loop K
		pthread_mutex_unlock(&mutex_chans);

		/* metronome. FIXME - move this one after the peak meter calculation */

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

	/* final loop: sum virtual channels and process plugins, only if channel
	 * is CHANNEL_SAMPLE. MIDI events have been already processed before. */

	pthread_mutex_lock(&mutex_chans);
	for (unsigned k=0; k<channels.size; k++) {
		channel *ch = channels.at(k);
#ifdef WITH_VST
		G_PluginHost.processStack(ch->vChan, PluginHost::CHANNEL, ch);
		G_PluginHost.freeVstMidiEvents(ch);
#endif
		for (unsigned j=0; j<bufferFrames; j+=2) {
			buffer[j]   += ch->vChan[j]   * ch->volume * ch->panLeft;
			buffer[j+1] += ch->vChan[j+1] * ch->volume * ch->panRight;
		}
	}
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


void Mixer::readActions() {

	/* MIDI to VST: we must pass an array of vstEvents structs, spanning
	 * through the whole frame */

	pthread_mutex_lock(&mutex_recs);

	for (unsigned y=0; y<recorder::frames.size; y++) {

		if (recorder::frames.at(y) == actualFrame) {

			for (unsigned z=0; z<recorder::global.at(y).size; z++) {

				int index   = recorder::global.at(y).at(z)->chan;
				channel *ch = getChannelByIndex(index);

				if (ch->type == CHANNEL_SAMPLE && ch->readActions == false)
					continue;

				recorder::action *a = recorder::global.at(y).at(z);

				switch (a->type) {
					case ACTION_KEYPRESS:
						if (ch->mode & SINGLE_ANY) {
							mh_startChan(ch, false);
							break;
						}
					case ACTION_KEYREL:
						if (ch->mode & SINGLE_ANY) {
							mh_stopChan(ch);
							break;
						}
					case ACTION_KILLCHAN:
						if (ch->mode & SINGLE_ANY) {
							mh_killChan(ch);
							break;
						}
					case ACTION_MIDI:
						mh_sendMidi(a, ch);
						break;
					case ACTION_MUTEON:
						mh_muteChan(ch, true);   // internal mute
						break;
					case ACTION_MUTEOFF:
						mh_unmuteChan(ch, true); // internal mute
						break;
					case ACTION_VOLUME:
						calcVolumeEnv(ch, actualFrame);
						break;
				}
			}
			break;
		}
	}

	pthread_mutex_unlock(&mutex_recs);
}


/* ------------------------------------------------------------------ */


void Mixer::updateFrameBars() {

	/* seconds ....... total time of play (in seconds) of the whole
	 *                 sequencer. 60 / bpm == how many seconds lasts one bpm
	 * totalFrames ... number of frames in the whole sequencer, x2 because
	 * 								 it's stereo
	 * framesPerBar .. n. of frames within a bar
	 * framesPerBeat . n. of frames within a beat */

	float seconds   = (60.0f / bpm) * beats;
	totalFrames     = G_Conf.samplerate * seconds * 2;
	framesPerBar    = totalFrames / bars;
	framesPerBeat   = totalFrames / beats;

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
		printf("[Mixer] vChanInput realloc error!");
}


/* ------------------------------------------------------------------ */


int Mixer::close() {
	//printf("[mixer::close] total channels %d\n", channels.size);
	running = false;
	while (channels.size > 0)
		deleteChannel(channels.at(0));
	//printf("[mixer::close] total channels %d\n", channels.size);
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
		for (unsigned i=0; i<channels.size; i++) {
			channel *ch = channels.at(i);
			if (ch->wave != NULL) {

				/* don't rewind a SINGLE_ANY, unless it's in read-record-mode */

				if ((ch->mode & LOOP_ANY) || (ch->recStatus == REC_READING && (ch->mode & SINGLE_ANY)))
					chanReset(ch);
			}
		}
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


void Mixer::calcFadeoutStep(channel *ch) {

	/* how many frames are left before the end of the sample? Is there
	 * enough room for a complete fadeout? Should we shorten it? */

	unsigned ctracker = ch->tracker * ch->pitch;

	if (ch->end - ctracker < (1 / DEFAULT_FADEOUT_STEP) * 2)
		ch->fadeoutStep = ceil((ch->end - ctracker) / ch->volume) * 2; /// or ch->volume_i ???
	else
		ch->fadeoutStep = DEFAULT_FADEOUT_STEP;
}


/* ------------------------------------------------------------------ */


bool Mixer::hasLogicalSamples() {
	for (unsigned i=0; i<channels.size; i++)
		if (channels.at(i)->wave != NULL)
			if (channels.at(i)->wave->isLogical)
				return true;
	return false;
}


/* ------------------------------------------------------------------ */


bool Mixer::hasEditedSamples() {
	for (unsigned i=0; i<channels.size; i++)
		if (channels.at(i)->wave != NULL)
			if (channels.at(i)->wave->isEdited)
				return true;
	return false;
}


/* ------------------------------------------------------------------ */


void Mixer::setPitch(channel *ch, float val) {

	/* if the pitch changes also chanStart/chanEnd must change accordingly
	 * and to do that we need the original (or previous) chanStart/chanEnd
	 * values (chanStartTrue and chanEndTrue). Formula:
	 *
	 * chanStart{pitched} = chanStart{Original} / newpitch */

	if (val == 1.0f) {
		ch->start = ch->startTrue;
		ch->end   = ch->endTrue;
		ch->pitch = 1.0f;
		return;
	}

	ch->start = (unsigned) floorf(ch->startTrue / val);
	ch->end   = (unsigned) floorf(ch->endTrue   / val);

	ch->pitch = val;

	/* even values please */

	if (ch->start % 2 != 0)	ch->start++;
	if (ch->end   % 2 != 0)	ch->end++;

	/* avoid overflow when changing pitch during play mode */

	if (ch->tracker > ch->end)
		ch->tracker = ch->end;
}


/* ------------------------------------------------------------------ */


void Mixer::setChanStart(struct channel *ch, unsigned val) {
	if (val % 2 != 0)
		val++;
	ch->startTrue = val;
	ch->start     = (unsigned) floorf(ch->startTrue / ch->pitch);
	ch->tracker   = ch->start;
}


/* ------------------------------------------------------------------ */


void Mixer::setChanEnd(struct channel *ch, unsigned val) {
	if (val % 2 != 0)
		val++;
	ch->endTrue = val;
	ch->end = (unsigned) floorf(ch->endTrue / ch->pitch);
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


/* ------------------------------------------------------------------ */


bool Mixer::isPlaying(channel *ch) {
	return ch->status == STATUS_PLAY || ch->status == STATUS_ENDING;
}


/* ------------------------------------------------------------------ */


void Mixer::updateChansOnSampleZero() {

	pthread_mutex_lock(&mutex_chans);

	for (unsigned k=0; k<channels.size; k++) {

		channel *ch = channels.at(k);

		if (ch->type == CHANNEL_SAMPLE) {

			if (ch->wave == NULL)
				continue;

			if (ch->mode & (LOOP_ONCE | LOOP_BASIC | LOOP_REPEAT)) {

				/* do a crossfade if the sample is playing. Regular chanReset
				 * instead if it's muted, otherwise a click occurs */

				if (ch->status == STATUS_PLAY) {
					if (ch->mute || ch->mute_i)
						chanReset(ch);
					else
						xfade(ch);
				}
				else
				if (ch->status == STATUS_ENDING)
					chanStop(ch);
			}

			if (ch->status == STATUS_WAIT)
				ch->status = STATUS_PLAY;

			if (ch->recStatus == REC_ENDING) {
				ch->recStatus = REC_STOPPED;
				recorder::disableRead(ch);    // rec stop
			}
			else
			if (ch->recStatus == REC_WAITING) {
				ch->recStatus = REC_READING;
				recorder::enableRead(ch);     // rec start
			}
		}
		else {
			if (ch->status == STATUS_ENDING)
				ch->status = STATUS_OFF;
			else
			if (ch->status == STATUS_WAIT)
				ch->status = STATUS_PLAY;
		}
	}
	pthread_mutex_unlock(&mutex_chans);
}


/* ------------------------------------------------------------------ */


void Mixer::calcVolumeEnv(struct channel *ch, int frame) {

	/* method: check this frame && next frame, then calculate delta */

	recorder::action *a0 = NULL;
	recorder::action *a1 = NULL;
	int res;

	res = recorder::getAction(ch->index, ACTION_VOLUME, frame, &a0);
	if (res == 0) {
		//printf("[mixer::calcVolumeEnv] a0 not found\n");
		return;
	}

	//printf("[mixer::calcVolumeEnv] a0 found, frame=%d value=%f\n", a0->frame, a0->fValue);

	res = recorder::getNextAction(ch->index, ACTION_VOLUME, frame, &a1);

	/* res == -1: a1 not found, this is the last one. Rewind the search
	 * and use action at frame number 0 (actions[0]) */

	if (res == -1) {
		//puts("[mixer::calcVolumeEnv] a1 not found, using actions[0]");
		res = recorder::getAction(ch->index, ACTION_VOLUME, 0, &a1);
	}
	else

	/* res == -2 ACTION_VOLUME not found. This should never happen */

	//if (res == -2)
	//	puts("[mixer::calcVolumeEnv] WARNING: action1 as ACTION_VOLUME not found!");
	//else
	//	printf("[mixer::calcVolumeEnv] a1 found, at frame %d value=%f\n", a1->frame, a1->fValue);

	ch->volume_i = a0->fValue;
	ch->volume_d = ((a1->fValue - a0->fValue) / ((a1->frame - a0->frame) / 2)) * 1.003f;
}
