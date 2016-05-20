/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * channel
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


#include <math.h>
#include "../utils/log.h"
#include "sampleChannel.h"
#include "patch_DEPR_.h"
#include "patch.h"
#include "conf.h"
#include "wave.h"
#include "pluginHost.h"
#include "waveFx.h"
#include "mixerHandler.h"
#include "kernelMidi.h"


using std::string;


SampleChannel::SampleChannel(int bufferSize, MidiMapConf *midiMapConf)
	: Channel          (CHANNEL_SAMPLE, STATUS_EMPTY, bufferSize, midiMapConf),
		frameRewind      (-1),
		wave             (NULL),
		tracker          (0),
		begin            (0),
		end              (0),
		pitch            (gDEFAULT_PITCH),
		boost            (1.0f),
		mode             (DEFAULT_CHANMODE),
		qWait	           (false),
		fadeinOn         (false),
		fadeinVol        (1.0f),
		fadeoutOn        (false),
		fadeoutVol       (1.0f),
		fadeoutTracker   (0),
		fadeoutStep      (DEFAULT_FADEOUT_STEP),
	  readActions      (true),
	  midiInReadActions(0x0),
	  midiInPitch      (0x0)
{
	rsmp_state = src_new(SRC_LINEAR, 2, NULL);
	pChan      = (float *) malloc(kernelAudio::realBufsize * 2 * sizeof(float));
}


/* -------------------------------------------------------------------------- */


SampleChannel::~SampleChannel()
{
	if (wave)
		delete wave;
	src_delete(rsmp_state);
	free(pChan);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::copy(const Channel *_src, pthread_mutex_t *pluginMutex)
{
	Channel::copy(_src, pluginMutex);
	SampleChannel *src = (SampleChannel *) _src;
	tracker         = src->tracker;
	begin           = src->begin;
	end             = src->end;
	boost           = src->boost;
	mode            = src->mode;
	qWait           = src->qWait;
	fadeinOn        = src->fadeinOn;
	fadeinVol       = src->fadeinVol;
	fadeoutOn       = src->fadeoutOn;
	fadeoutVol      = src->fadeoutVol;
	fadeoutTracker  = src->fadeoutTracker;
	fadeoutStep     = src->fadeoutStep;
	fadeoutType     = src->fadeoutType;
	fadeoutEnd      = src->fadeoutEnd;
	setPitch(src->pitch);

	if (src->wave) {
		Wave *w = new Wave(*src->wave); // invoke Wave's copy constructor
		pushWave(w);
		generateUniqueSampleName();
	}
}


/* -------------------------------------------------------------------------- */


void SampleChannel::generateUniqueSampleName()
{
	string oldName = wave->name;
	int k = 1; // Start from k = 1, zero is too nerdy
	while (!mh_uniqueSamplename(this, wave->name.c_str())) {
		wave->updateName((oldName + "-" + gItoa(k)).c_str());
		k++;
	}
}


/* -------------------------------------------------------------------------- */


void SampleChannel::clear()
{
	/** TODO - these memsets can be done only if status PLAY (if below),
	 * but it would require extra clearPChan calls when samples stop */

		memset(vChan, 0, sizeof(float) * bufferSize);
		memset(pChan, 0, sizeof(float) * bufferSize);

	if (status & (STATUS_PLAY | STATUS_ENDING)) {
		tracker = fillChan(vChan, tracker, 0);
		if (fadeoutOn && fadeoutType == XFADE) {
			gLog("[clear] filling pChan fadeoutTracker=%d\n", fadeoutTracker);
			fadeoutTracker = fillChan(pChan, fadeoutTracker, 0);
		}
	}
}


/* -------------------------------------------------------------------------- */


void SampleChannel::calcVolumeEnv(int frame)
{
	/* method: check this frame && next frame, then calculate delta */

	recorder::action *a0 = NULL;
	recorder::action *a1 = NULL;
	int res;

	/* get this action on frame 'frame'. It's unlikely that the action
	 * is not found. */

	res = recorder::getAction(index, ACTION_VOLUME, frame, &a0);
	if (res == 0)
		return;

	/* get the action next to this one.
	 * res == -1: a1 not found, this is the last one. Rewind the search
	 * and use action at frame number 0 (actions[0]).
	 * res == -2 ACTION_VOLUME not found. This should never happen */

	res = recorder::getNextAction(index, ACTION_VOLUME, frame, &a1);

	if (res == -1)
		res = recorder::getAction(index, ACTION_VOLUME, 0, &a1);

	volume_i = a0->fValue;
	volume_d = ((a1->fValue - a0->fValue) / ((a1->frame - a0->frame) / 2)) * 1.003f;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::hardStop(int frame)
{
	if (frame != 0)        // clear data in range [frame, bufferSize-1]
		clearChan(vChan, frame);
	status = STATUS_OFF;
	sendMidiLplay();
	reset(frame);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::onBar(int frame)
{
	///if (mode == LOOP_REPEAT && status == STATUS_PLAY)
	///	//setXFade(frame);
	///	reset(frame);

	if (mode == LOOP_REPEAT) {
		if (status == STATUS_PLAY)
			//setXFade(frame);
			reset(frame);
	}
	else
	if (mode == LOOP_ONCE_BAR) {
		if (status == STATUS_WAIT) {
			status  = STATUS_PLAY;
			tracker = fillChan(vChan, tracker, frame);
			sendMidiLplay();
		}
	}
}


/* -------------------------------------------------------------------------- */


int SampleChannel::save(const char *path)
{
	return wave->writeData(path);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setBegin(unsigned v)
{
	begin   = v;
	tracker = begin;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setEnd(unsigned v)
{
	end = v;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setPitch(float v)
{
	pitch = v;
	rsmp_data.src_ratio = 1/pitch;

	/* if status is off don't slide between frequencies */

	if (status & (STATUS_OFF | STATUS_WAIT))
		src_set_ratio(rsmp_state, 1/pitch);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::rewind()
{
	/* rewind LOOP_ANY or SINGLE_ANY only if it's in read-record-mode */

	if (wave != NULL) {
		if ((mode & LOOP_ANY) || (recStatus == REC_READING && (mode & SINGLE_ANY)))
			reset(0);  // rewind is user-generated events, always on frame 0
	}
}


/* -------------------------------------------------------------------------- */


void SampleChannel::parseAction(recorder::action *a, int localFrame,
		int globalFrame, int quantize, bool mixerIsRunning)
{
	if (readActions == false)
		return;

	switch (a->type) {
		case ACTION_KEYPRESS:
			if (mode & SINGLE_ANY)
				start(localFrame, false, quantize, mixerIsRunning);
			break;
		case ACTION_KEYREL:
			if (mode & SINGLE_ANY)
				stop();
			break;
		case ACTION_KILLCHAN:
			if (mode & SINGLE_ANY)
				kill(localFrame);
			break;
		case ACTION_MUTEON:
			setMute(true);   // internal mute
			break;
		case ACTION_MUTEOFF:
			unsetMute(true); // internal mute
			break;
		case ACTION_VOLUME:
			calcVolumeEnv(globalFrame);
			break;
	}
}


/* -------------------------------------------------------------------------- */


void SampleChannel::sum(int frame, bool running)
{
	if (wave == NULL || status & ~(STATUS_PLAY | STATUS_ENDING))
		return;

	if (frame != frameRewind) {

		/* volume envelope, only if seq is running */

		if (running) {
			volume_i += volume_d;
			if (volume_i < 0.0f)
				volume_i = 0.0f;
			else
			if (volume_i > 1.0f)
				volume_i = 1.0f;
		}

		/* fadein or fadeout processes. If mute, delete any signal. */

		/** TODO - big issue: fade[in/out]Vol * internal_volume might be a
		 * bad choice: it causes glitches when muting on and off during a
		 * volume envelope. */

		if (mute || mute_i) {
			vChan[frame]   = 0.0f;
			vChan[frame+1] = 0.0f;
		}
		else
		if (fadeinOn) {
			if (fadeinVol < 1.0f) {
				vChan[frame]   *= fadeinVol * volume_i;
				vChan[frame+1] *= fadeinVol * volume_i;
				fadeinVol += 0.01f;
			}
			else {
				fadeinOn  = false;
				fadeinVol = 0.0f;
			}
		}
		else
		if (fadeoutOn) {
			if (fadeoutVol > 0.0f) { // fadeout ongoing
				if (fadeoutType == XFADE) {
					vChan[frame]   *= volume_i;
					vChan[frame+1] *= volume_i;
					vChan[frame]    = pChan[frame]   * fadeoutVol * volume_i;
					vChan[frame+1]  = pChan[frame+1] * fadeoutVol * volume_i;
				}
				else {
					vChan[frame]   *= fadeoutVol * volume_i;
					vChan[frame+1] *= fadeoutVol * volume_i;
				}
				fadeoutVol -= fadeoutStep;
			}
			else {  // fadeout end
				fadeoutOn  = false;
				fadeoutVol = 1.0f;

				/* QWait ends with the end of the xfade */

				if (fadeoutType == XFADE) {
					qWait = false;
				}
				else {
					if (fadeoutEnd == DO_MUTE)
						mute = true;
					else
					if (fadeoutEnd == DO_MUTE_I)
						mute_i = true;
					else             // DO_STOP
						hardStop(frame);
				}
			}
		}
		else {
			vChan[frame]   *= volume_i;
			vChan[frame+1] *= volume_i;
		}
	}
	else { // at this point the sample has reached the end */

		if (mode & (SINGLE_BASIC | SINGLE_PRESS | SINGLE_RETRIG) ||
			 (mode == SINGLE_ENDLESS && status == STATUS_ENDING)   ||
			 (mode & LOOP_ANY && !running))     // stop loops when the seq is off
		{
			status = STATUS_OFF;
			sendMidiLplay();
		}

		/* LOOP_ONCE or LOOP_ONCE_BAR: if ending (i.e. the user requested their
		 * termination), kill 'em. Let them wait otherwise. But don't put back in
		 * wait mode those already stopped by the conditionals above. */

		if (mode & (LOOP_ONCE | LOOP_ONCE_BAR)) {
			if (status == STATUS_ENDING)
				status = STATUS_OFF;
			else
			if (status != STATUS_OFF)
				status = STATUS_WAIT;
		}

		/* check for end of samples. SINGLE_ENDLESS runs forever unless
		 * it's in ENDING mode. */

		reset(frame);
	}
}


/* -------------------------------------------------------------------------- */


void SampleChannel::onZero(int frame, bool recsStopOnChanHalt)
{
	if (wave == NULL)
		return;

	if (mode & LOOP_ANY) {

		/* do a crossfade if the sample is playing. Regular chanReset
		 * instead if it's muted, otherwise a click occurs */

		if (status == STATUS_PLAY) {
			/*
			if (mute || mute_i)
				reset(frame);
			else
				setXFade(frame);
			*/
			reset(frame);
		}
		else
		if (status == STATUS_ENDING)
			hardStop(frame);
	}

	if (status == STATUS_WAIT) { /// FIXME - should be inside previous if!
		status  = STATUS_PLAY;
		sendMidiLplay();
		tracker = fillChan(vChan, tracker, frame);
	}

	if (recStatus == REC_ENDING) {
		recStatus = REC_STOPPED;
		setReadActions(false, recsStopOnChanHalt);  // rec stop
	}
	else
	if (recStatus == REC_WAITING) {
		recStatus = REC_READING;
		setReadActions(true, recsStopOnChanHalt);   // rec start
	}
}


/* -------------------------------------------------------------------------- */


void SampleChannel::quantize(int index, int localFrame, int globalFrame)
{
	/* skip if LOOP_ANY or not in quantizer-wait mode */

	if ((mode & LOOP_ANY) || !qWait)
		return;

	/* no fadeout if the sample starts for the first time (from a
	 * STATUS_OFF), it would be meaningless. */

	if (status == STATUS_OFF) {
		status  = STATUS_PLAY;
		sendMidiLplay();
		qWait   = false;
		tracker = fillChan(vChan, tracker, localFrame); /// FIXME: ???
	}
	else
		//setXFade(localFrame);
		reset(localFrame);

	/* this is the moment in which we record the keypress, if the
	 * quantizer is on. SINGLE_PRESS needs overdub */

	if (recorder::canRec(this)) {
		if (mode == SINGLE_PRESS)
			recorder::startOverdub(index, ACTION_KEYS, globalFrame);
		else
			recorder::rec(index, ACTION_KEYPRESS, globalFrame);
	}
}


/* -------------------------------------------------------------------------- */


int SampleChannel::getPosition()
{
	if (status & ~(STATUS_EMPTY | STATUS_MISSING | STATUS_OFF)) // if is not (...)
		return tracker - begin;
	else
		return -1;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setMute(bool internal)
{
	if (internal) {

		/* global mute is on? don't waste time with fadeout, just mute it
		 * internally */

		if (mute)
			mute_i = true;
		else {
			if (isPlaying())
				setFadeOut(DO_MUTE_I);
			else
				mute_i = true;
		}
	}
	else {

		/* internal mute is on? don't waste time with fadeout, just mute it
		 * globally */

		if (mute_i)
			mute = true;
		else {

			/* sample in play? fadeout needed. Else, just mute it globally */

			if (isPlaying())
				setFadeOut(DO_MUTE);
			else
				mute = true;
		}
	}

	sendMidiLmute();
}


/* -------------------------------------------------------------------------- */


void SampleChannel::unsetMute(bool internal)
{
	if (internal) {
		if (mute)
			mute_i = false;
		else {
			if (isPlaying())
				setFadeIn(internal);
			else
				mute_i = false;
		}
	}
	else {
		if (mute_i)
			mute = false;
		else {
			if (isPlaying())
				setFadeIn(internal);
			else
				mute = false;
		}
	}

	sendMidiLmute();
}


/* -------------------------------------------------------------------------- */


void SampleChannel::calcFadeoutStep()
{
	if (end - tracker < (1 / DEFAULT_FADEOUT_STEP) * 2)
		fadeoutStep = ceil((end - tracker) / volume) * 2; /// or volume_i ???
	else
		fadeoutStep = DEFAULT_FADEOUT_STEP;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setReadActions(bool v, bool recsStopOnChanHalt)
{
	if (v)
		readActions = true;
	else {
		readActions = false;
		if (recsStopOnChanHalt)
			kill(0);  /// FIXME - wrong frame value
	}
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setFadeIn(bool internal)
{
	if (internal) mute_i = false;  // remove mute before fading in
	else          mute   = false;
	fadeinOn  = true;
	fadeinVol = 0.0f;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setFadeOut(int actionPostFadeout)
{
	calcFadeoutStep();
	fadeoutOn   = true;
	fadeoutVol  = 1.0f;
	fadeoutType = FADEOUT;
	fadeoutEnd	= actionPostFadeout;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::setXFade(int frame)
{
	gLog("[xFade] frame=%d tracker=%d\n", frame, tracker);

	calcFadeoutStep();
	fadeoutOn      = true;
	fadeoutVol     = 1.0f;
	fadeoutType    = XFADE;
	fadeoutTracker = fillChan(pChan, tracker, 0, false);
	reset(frame);
}


/* -------------------------------------------------------------------------- */


/* on reset, if frame > 0 and in play, fill again pChan to create
 * something like this:
 *
 * |abcdefabcdefab*abcdefabcde|
 * [old data-----]*[new data--]
 *
 * */

void SampleChannel::reset(int frame)
{
	//fadeoutTracker = tracker;   // store old frame number for xfade
	tracker = begin;
	mute_i  = false;
	if (frame > 0 && status & (STATUS_PLAY | STATUS_ENDING))
		tracker = fillChan(vChan, tracker, frame);
}


/* -------------------------------------------------------------------------- */


void SampleChannel::empty()
{
	status = STATUS_OFF;
	if (wave) {
		delete wave;
		wave = NULL;
	}
	status = STATUS_EMPTY;
	sendMidiLplay();
}


/* -------------------------------------------------------------------------- */


void SampleChannel::pushWave(Wave *w)
{
	wave   = w;
	status = STATUS_OFF;
	sendMidiLplay();
	begin  = 0;
	end    = wave->size;
}


/* -------------------------------------------------------------------------- */


bool SampleChannel::allocEmpty(int frames, int samplerate, int takeId)
{
	Wave *w = new Wave();
	if (!w->allocEmpty(frames, samplerate))
		return false;

	char wname[32];
	sprintf(wname, "TAKE-%d", takeId);

	w->pathfile = gGetCurrentPath() + "/" + wname; // FIXME - use gGetSlash() in utils.h
	w->name     = wname;
	wave        = w;
	status      = STATUS_OFF;
	begin       = 0;
	end         = wave->size;

	sendMidiLplay();

	return true;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::process(float *buffer)
{
#ifdef WITH_VST
	pluginHost->processStack(vChan, PluginHost::CHANNEL, this);
#endif

	for (int j=0; j<bufferSize; j+=2) {
		buffer[j]   += vChan[j]   * volume * panLeft  * boost;
		buffer[j+1] += vChan[j+1] * volume * panRight * boost;
	}
}


/* -------------------------------------------------------------------------- */


void SampleChannel::kill(int frame)
{
	if (wave != NULL && status != STATUS_OFF) {
		if (mute || mute_i || (status == STATUS_WAIT && mode & LOOP_ANY))
			hardStop(frame);
		else
			setFadeOut(DO_STOP);
	}
}


/* -------------------------------------------------------------------------- */


void SampleChannel::stopBySeq(bool chansStopOnSeqHalt)
{
  /* Loop-mode samples in wait status get stopped right away. */

	if (mode & LOOP_ANY && status == STATUS_WAIT) {
		status = STATUS_OFF;
    return;
  }

  /* When to kill samples on StopSeq:
   *  - when chansStopOnSeqHalt == true (run the sample to end otherwise)
   *  - when a channel has recs in play (1)
   *
   * Always kill at frame=0, this is a user-generated event.
   *
   * (1) a channel has recs in play when:
   *  - Recorder has events for that channel
   *  - G_Mixer has at least one sample in play
   *  - Recorder's channel is active (altrimenti può capitare che si stoppino i
   *    sample suonati manualmente in un canale con rec disattivate) */

	if (chansStopOnSeqHalt) {
    if ((mode & LOOP_ANY) || (hasActions && readActions && status == STATUS_PLAY))
      kill(0);
  }
}


/* -------------------------------------------------------------------------- */


void SampleChannel::stop()
{
	if (mode == SINGLE_PRESS && status == STATUS_PLAY) {
		if (mute || mute_i)
			hardStop(0);  /// FIXME - wrong frame value
		else
			setFadeOut(DO_STOP);
	}
	else  // stop a SINGLE_PRESS immediately, if the quantizer is on
	if (mode == SINGLE_PRESS && qWait == true)
		qWait = false;
}


/* -------------------------------------------------------------------------- */


int SampleChannel::load(const char *file, int samplerate, int rsmpQuality)
{
	if (strcmp(file, "") == 0 || gIsDir(file)) {
		gLog("[SampleChannel] file not specified\n");
		return SAMPLE_LEFT_EMPTY;
	}

	if (strlen(file) > FILENAME_MAX)
		return SAMPLE_PATH_TOO_LONG;

	Wave *w = new Wave();

	if (!w->open(file)) {
		gLog("[SampleChannel] %s: read error\n", file);
		delete w;
		return SAMPLE_READ_ERROR;
	}

	if (w->channels() > 2) {
		gLog("[SampleChannel] %s: unsupported multichannel wave\n", file);
		delete w;
		return SAMPLE_MULTICHANNEL;
	}

	if (!w->readData()) {
		delete w;
		return SAMPLE_READ_ERROR;
	}

	if (w->channels() == 1) /** FIXME: error checking  */
		wfx_monoToStereo(w);

	if (w->rate() != samplerate) {
		gLog("[SampleChannel] input rate (%d) != system rate (%d), conversion needed\n",
				w->rate(), samplerate);
		w->resample(rsmpQuality, samplerate);
	}

	pushWave(w);
	generateUniqueSampleName();

	gLog("[SampleChannel] %s loaded in channel %d\n", file, index);
	return SAMPLE_LOADED_OK;
}


/* -------------------------------------------------------------------------- */


int SampleChannel::readPatch_DEPR_(const char *f, int i, Patch_DEPR_ *patch,
		int samplerate, int rsmpQuality)
{
	int res = load(f, samplerate, rsmpQuality);

		volume      = patch->getVol(i);
		key         = patch->getKey(i);
		index       = patch->getIndex(i);
		mode        = patch->getMode(i);
		mute        = patch->getMute(i);
		mute_s      = patch->getMute_s(i);
		solo        = patch->getSolo(i);
		boost       = patch->getBoost(i);
		panLeft     = patch->getPanLeft(i);
		panRight    = patch->getPanRight(i);
		readActions = patch->getRecActive(i);
		recStatus   = readActions ? REC_READING : REC_STOPPED;

		readPatchMidiIn_DEPR_(i, *patch);
		midiInReadActions = patch->getMidiValue(i, "InReadActions");
		midiInPitch       = patch->getMidiValue(i, "InPitch");
		readPatchMidiOut_DEPR_(i, *patch);

	if (res == SAMPLE_LOADED_OK) {
		setBegin(patch->getBegin(i));
		setEnd  (patch->getEnd(i, wave->size));
		setPitch(patch->getPitch(i));
	}
	else {
		// volume = DEFAULT_VOL;
		// mode   = DEFAULT_CHANMODE;
		// status = STATUS_WRONG;
		// key    = 0;

		if (res == SAMPLE_LEFT_EMPTY)
			status = STATUS_EMPTY;
		else
		if (res == SAMPLE_READ_ERROR)
			status = STATUS_MISSING;
		sendMidiLplay();
	}

	return res;
}


/* -------------------------------------------------------------------------- */


int SampleChannel::readPatch(const string &basePath, int i, Patch *patch,
		pthread_mutex_t *pluginMutex, int samplerate, int rsmpQuality)
{
	/* load channel's data first: if the sample is missing or wrong, the channel
	 * is not completely blank. */

	Channel::readPatch("", i, patch, pluginMutex, samplerate, rsmpQuality);

	Patch::channel_t *pch = &patch->channels.at(i);

	mode              = pch->mode;
	boost             = pch->boost;
	readActions       = pch->recActive;
	recStatus         = readActions ? REC_READING : REC_STOPPED;
	midiInReadActions = pch->midiInReadActions;
	midiInPitch       = pch->midiInPitch;

	int res = load((basePath + pch->samplePath).c_str(), samplerate, rsmpQuality);
	if (res == SAMPLE_LOADED_OK) {
		setBegin(pch->begin);
		setEnd  (pch->end);
		setPitch(pch->pitch);
	}
	else {
		if (res == SAMPLE_LEFT_EMPTY)
			status = STATUS_EMPTY;
		else
		if (res == SAMPLE_READ_ERROR)
			status = STATUS_MISSING;
		sendMidiLplay();  // FIXME - why sending MIDI lightning if sample status is wrong?
	}

	return res;
}


/* -------------------------------------------------------------------------- */


bool SampleChannel::canInputRec()
	{
	return wave == NULL;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::start(int frame, bool doQuantize, int quantize,
		bool mixerIsRunning)
{
	switch (status)	{
		case STATUS_EMPTY:
		case STATUS_MISSING:
		case STATUS_WRONG:
		{
			return;
		}

		case STATUS_OFF:
		{
			if (mode & LOOP_ANY) {
				status = STATUS_WAIT;
				sendMidiLplay();
			}
			else {
				if (quantize > 0 && mixerIsRunning && doQuantize)
					qWait = true;
				else {

					/* fillChan only if frame != 0. If you call fillChan on frame == 0
					 * a duplicate call to fillChan occurs with loss of data. */

					status = STATUS_PLAY;
					sendMidiLplay();
					if (frame != 0)
						tracker = fillChan(vChan, tracker, frame);
				}
			}
			break;
		}

		case STATUS_PLAY:
		{
			if (mode == SINGLE_BASIC)
				setFadeOut(DO_STOP);
			else
			if (mode == SINGLE_RETRIG) {
				if (quantize > 0 && mixerIsRunning && doQuantize)
					qWait = true;
				else
					reset(frame);
			}
			else
			if (mode & (LOOP_ANY | SINGLE_ENDLESS)) {
				status = STATUS_ENDING;
				sendMidiLplay();
			}
			break;
		}

		case STATUS_WAIT:
		{
			status = STATUS_OFF;
			sendMidiLplay();
			break;
		}

		case STATUS_ENDING:
		{
			status = STATUS_PLAY;
			sendMidiLplay();
			break;
		}
	}
}


/* -------------------------------------------------------------------------- */


int SampleChannel::writePatch(int i, bool isProject, Patch *patch)
{
	int pchIndex = Channel::writePatch(i, isProject, patch);
	Patch::channel_t *pch = &patch->channels.at(pchIndex);

	if (wave != NULL) {
		pch->samplePath = wave->pathfile;
		if (isProject)
			pch->samplePath = gBasename(wave->pathfile);  // make it portable
	}
	else
		pch->samplePath = "";

	pch->mode              = mode;
	pch->begin             = begin;
	pch->end               = end;
	pch->boost             = boost;
	pch->recActive         = readActions;
	pch->pitch             = pitch;
	pch->midiInReadActions = midiInReadActions;
	pch->midiInPitch       = midiInPitch;

	return 0;
}


/* -------------------------------------------------------------------------- */


void SampleChannel::clearChan(float *dest, int start)
{
	memset(dest+start, 0, sizeof(float)*(bufferSize-start));
}


/* -------------------------------------------------------------------------- */


int SampleChannel::fillChan(float *dest, int start, int offset, bool rewind)
{
	int position;  // return value: the new position

	if (pitch == 1.0f) {

		/* case 1: 'dest' lies within the original sample boundaries (start-
		 * end) */

		if (start+bufferSize-offset <= end) {
			memcpy(dest+offset, wave->data+start, (bufferSize-offset)*sizeof(float));
			position = start+bufferSize-offset;
			if (rewind)
				frameRewind = -1;
		}

		/* case2: 'dest' lies outside the end of the sample, OR the sample
		 * is smaller than 'dest' */

		else {
			memcpy(dest+offset, wave->data+start, (end-start)*sizeof(float));
			position = end;
			if (rewind)
				frameRewind = end-start+offset;
		}
	}
	else {

		rsmp_data.data_in       = wave->data+start;         // source data
		rsmp_data.input_frames  = (end-start)/2;            // how many readable bytes
		rsmp_data.data_out      = dest+offset;              // destination (processed data)
		rsmp_data.output_frames = (bufferSize-offset)/2;    // how many bytes to process
		rsmp_data.end_of_input  = false;

		src_process(rsmp_state, &rsmp_data);
		int gen = rsmp_data.output_frames_gen*2;            // frames generated by this call

		position = start + rsmp_data.input_frames_used*2;   // position goes forward of frames_used (i.e. read from wave)

		if (rewind) {
			if (gen == bufferSize-offset)
				frameRewind = -1;
			else
				frameRewind = gen+offset;
		}
	}
	return position;
}
