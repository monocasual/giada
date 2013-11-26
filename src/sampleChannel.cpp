/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * channel
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
#include "sampleChannel.h"
#include "patch.h"
#include "conf.h"
#include "wave.h"
#include "pluginHost.h"
#include "waveFx.h"
#include "mixerHandler.h"


extern Patch       G_Patch;
extern Mixer       G_Mixer;
extern Conf        G_Conf;
#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif


SampleChannel::SampleChannel(char side)
	: Channel    (CHANNEL_SAMPLE, STATUS_EMPTY, side),
		wave       (NULL),
		tracker    (0),
		begin      (0),
		end        (0),
		pitch      (gDEFAULT_PITCH),
		boost      (1.0f),
		mode       (DEFAULT_CHANMODE),
		qWait	     (false),
		fadein     (1.0f),
		fadeoutOn  (false),
		fadeoutVol (1.0f),
		fadeoutStep(DEFAULT_FADEOUT_STEP),
		key        (0),
	  readActions(true),
	  midiInReadActions(0x0)
{
	converter  = src_new(1, 2, NULL);
	_procChan_ = (float *) malloc(kernelAudio::realBufsize * 2 * sizeof(float));
}


/* ------------------------------------------------------------------ */


SampleChannel::~SampleChannel() {
	if (wave)
		delete wave;
	converter = src_delete(converter);
}


/* ------------------------------------------------------------------ */


void SampleChannel::clear(int bufSize) {
	memset(vChan, 0, sizeof(float) * bufSize);
	pitch=1.0f;
	printf("[sampleChannel] pitch=%f, pChan=%d\n", pitch, (int)(kernelAudio::realBufsize * pitch));
}


/* ------------------------------------------------------------------ */


void SampleChannel::calcVolumeEnv(int frame) {

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


/* ------------------------------------------------------------------ */


void SampleChannel::hardStop() {
	status = STATUS_OFF;
	reset();
}


/* ------------------------------------------------------------------ */


void SampleChannel::onBar() {
	if (mode == LOOP_REPEAT && status == STATUS_PLAY)
		setXFade();
}


/* ------------------------------------------------------------------ */


int SampleChannel::save(const char *path) {
	return wave->writeData(path);
}


/* ------------------------------------------------------------------ */


void SampleChannel::setBegin(unsigned v) {
	if (v % 2 != 0)
		v++;
	///beginTrue = v;
	///begin     = (unsigned) floorf(beginTrue / pitch);
	begin     = (unsigned) floorf(begin / pitch);
	tracker   = begin;
}


/* ------------------------------------------------------------------ */


void SampleChannel::setEnd(unsigned v) {
	if (v % 2 != 0)
		v++;
	///endTrue = v;
	///end = (unsigned) floorf(endTrue / pitch);
	end = (unsigned) floorf(end / pitch);
}


/* ------------------------------------------------------------------ */


void SampleChannel::setPitch(float v) {

	pitch = v;

	if (pitch != 1.0f) {
		begin   = begin / pitch;
		end     = end / pitch;
		///tracker = prevTracker / pitch;

		if (tracker > end) tracker = end;

		if (begin % 2 != 0)	begin++;
		if (end   % 2 != 0)	end++;
	}

	printf("[SampleChannel] new pitch=%f, begin=%d end=%d\n", pitch, begin, end);

}


/* ------------------------------------------------------------------ */

/*
void SampleChannel::processPitch() {
	if (pitch != 1.0f)
		wave->resampleProc(1, pitch);
}
*/

/* ------------------------------------------------------------------ */


void SampleChannel::rewind() {

	/* rewind LOOP_ANY or SINGLE_ANY only if it's in read-record-mode */

	if (wave != NULL) {
		if ((mode & LOOP_ANY) || (recStatus == REC_READING && (mode & SINGLE_ANY)))
			reset();
	}
}


/* ------------------------------------------------------------------ */


void SampleChannel::parseAction(recorder::action *a, int frame) {

	if (readActions == false)
		return;

	switch (a->type) {
		case ACTION_KEYPRESS:
			if (mode & SINGLE_ANY)
				start(false);
			break;
		case ACTION_KEYREL:
			if (mode & SINGLE_ANY)
				stop();
			break;
		case ACTION_KILLCHAN:
			if (mode & SINGLE_ANY)
				kill();
			break;
		case ACTION_MUTEON:
			setMute(true);   // internal mute
			break;
		case ACTION_MUTEOFF:
			unsetMute(true); // internal mute
			break;
		case ACTION_VOLUME:
			calcVolumeEnv(frame);
			break;
	}
}


/* ------------------------------------------------------------------ */


void SampleChannel::sum(int frame, bool running) {

	if (wave == NULL)
		return;

	if (status & (STATUS_PLAY | STATUS_ENDING)) {

		if (tracker <= end) {

			/* fade in */

			if (fadein <= 1.0f)
				fadein += 0.01f;		/// TODO - remove the hardcoded value

			/* volume envelope, only if seq is running */

			if (running) {
				volume_i += volume_d;
				if (volume_i < 0.0f)
					volume_i = 0.0f;
				else
				if (volume_i > 1.0f)
					volume_i = 1.0f;
			}

			/* fadeout process (both fadeout and xfade) */

			if (fadeoutOn) {
				if (fadeoutVol >= 0.0f) { // fadeout ongoing

					float v = volume_i * boost;

					if (fadeoutType == XFADE) {

						/* ftp is fadeoutTracker affected by pitch */

						vChan[frame]   += wave->data[fadeoutTracker]   * fadeoutVol * v;
						vChan[frame+1] += wave->data[fadeoutTracker+1] * fadeoutVol * v;

						vChan[frame]   += wave->data[tracker]   * v;
						vChan[frame+1] += wave->data[tracker+1] * v;

					}
					else { // FADEOUT
						vChan[frame]   += wave->data[tracker]   * fadeoutVol * v;
						vChan[frame+1] += wave->data[tracker+1] * fadeoutVol * v;
					}

					fadeoutVol     -= fadeoutStep;
					fadeoutTracker += 2;
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
							hardStop();
					}

					/* we must append another frame in the buffer when the fadeout
					 * ends: there's a gap here which would clip otherwise */

					vChan[frame]   = vChan[frame-2];
					vChan[frame+1] = vChan[frame-1];
				}
			}  // no fadeout to do
			else {
				if (!mute && !mute_i) {
					float v = volume_i * fadein * boost;
					vChan[frame]   += wave->data[tracker]   * v;
					vChan[frame+1] += wave->data[tracker+1] * v;
				}
			}

			tracker += 2;

			/* check for end of samples. SINGLE_ENDLESS runs forever unless
			 * it's in ENDING mode */

			if (tracker >= end) {

				reset();

				if (mode & (SINGLE_BASIC | SINGLE_PRESS | SINGLE_RETRIG) ||
					 (mode == SINGLE_ENDLESS && status == STATUS_ENDING))
				{
					status = STATUS_OFF;
				}

				/// FIXME - unify these
				/* stop loops when the seq is off */

				if ((mode & LOOP_ANY) && !running)
					status = STATUS_OFF;

				/* temporary stop LOOP_ONCE not in ENDING status, otherwise they
				 * would return in wait, losing the ENDING status */

				if (mode == LOOP_ONCE && status != STATUS_ENDING)
					status = STATUS_WAIT;
			}
		}
	}
}


/* ------------------------------------------------------------------ */


void SampleChannel::onZero() {

	if (wave == NULL)
		return;

	if (mode & (LOOP_ONCE | LOOP_BASIC | LOOP_REPEAT)) {

		/* do a crossfade if the sample is playing. Regular chanReset
		 * instead if it's muted, otherwise a click occurs */

		if (status == STATUS_PLAY) {
			if (mute || mute_i)
				reset();
			else
				setXFade();
		}
		else
		if (status == STATUS_ENDING)
			hardStop();
	}

	if (status == STATUS_WAIT) /// FIXME - should be inside previous if!
		status = STATUS_PLAY;

	if (recStatus == REC_ENDING) {
		recStatus = REC_STOPPED;
		recorder::disableRead(this);    // rec stop
	}
	else
	if (recStatus == REC_WAITING) {
		recStatus = REC_READING;
		recorder::enableRead(this);     // rec start
	}
}


/* ------------------------------------------------------------------ */


void SampleChannel::quantize(int index, int frame) {

	if ((mode & SINGLE_ANY) && qWait == true)	{

		/* no fadeout if the sample starts for the first time (from a STATUS_OFF), it would
		 * be meaningless. */

		if (status == STATUS_OFF) {
			status = STATUS_PLAY;
			qWait  = false;
		}
		else
			setXFade();

		/* this is the moment in which we record the keypress, if the quantizer is on.
		 * SINGLE_PRESS needs overdub */

		if (recorder::canRec(this)) {
			if (mode == SINGLE_PRESS)
				recorder::startOverdub(index, ACTION_KEYS, frame);
			else
				recorder::rec(index, ACTION_KEYPRESS, frame);
		}
	}
}


/* ------------------------------------------------------------------ */


int SampleChannel::getPosition() {
	if (status & ~(STATUS_EMPTY | STATUS_MISSING | STATUS_OFF)) // if is not (...)
		return tracker - begin;
	else
		return -1;
}


/* ------------------------------------------------------------------ */


void SampleChannel::setMute(bool internal) {

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
}


/* ------------------------------------------------------------------ */


void SampleChannel::unsetMute(bool internal) {
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
}


/* ------------------------------------------------------------------ */


void SampleChannel::calcFadeoutStep() {
	unsigned ctracker = tracker * pitch;
	if (end - ctracker < (1 / DEFAULT_FADEOUT_STEP) * 2)
		fadeoutStep = ceil((end - ctracker) / volume) * 2; /// or volume_i ???
	else
		fadeoutStep = DEFAULT_FADEOUT_STEP;
}


/* ------------------------------------------------------------------ */


void SampleChannel::setFadeIn(bool internal) {
	if (internal) mute_i = false;  // remove mute before fading in
	else          mute   = false;
	fadein = 0.0f;
}


/* ------------------------------------------------------------------ */


void SampleChannel::setFadeOut(int actionPostFadeout) {
	calcFadeoutStep();
	fadeoutOn   = true;
	fadeoutVol  = 1.0f;
	fadeoutType = FADEOUT;
	fadeoutEnd	= actionPostFadeout;
}


/* ------------------------------------------------------------------ */


void SampleChannel::setXFade() {
	calcFadeoutStep();
	fadeoutOn      = true;
	fadeoutVol     = 1.0f;
	fadeoutTracker = tracker;
	fadeoutType    = XFADE;
	reset();
}


/* ------------------------------------------------------------------ */


void SampleChannel::reset() {
	tracker = begin;
	mute_i  = false;
}



/* ------------------------------------------------------------------ */


void SampleChannel::empty() {
	status = STATUS_OFF;
	if (wave) {
		delete wave;
		wave = NULL;
	}
	status = STATUS_EMPTY;
}


/* ------------------------------------------------------------------ */


void SampleChannel::pushWave(Wave *w) {
	wave      = w;
	status    = STATUS_OFF;
	begin     = 0;
	//beginTrue = 0;
	end       = wave->size;
	//endTrue   = wave->size;
}


/* ------------------------------------------------------------------ */


bool SampleChannel::allocEmpty(int frames, int takeId) {

	Wave *w = new Wave();
	if (!w->allocEmpty(frames))
		return false;

	char wname[32];
	sprintf(wname, "TAKE-%d", takeId);

	w->pathfile = getCurrentPath()+"/"+wname;
	w->name     = wname;
	wave        = w;
	status      = STATUS_OFF;
	begin       = 0;
	//beginTrue   = 0;
	end         = wave->size;
	//endTrue     = wave->size;

	return true;
}


/* ------------------------------------------------------------------ */


void SampleChannel::process(float *buffer, int size) {

#ifdef WITH_VST
	G_PluginHost.processStack(vChan, PluginHost::CHANNEL, this);
#endif

	for (int j=0; j<size; j+=2) {
		buffer[j]   += vChan[j]   * volume * panLeft;
		buffer[j+1] += vChan[j+1] * volume * panRight;
	}
}


/* ------------------------------------------------------------------ */


void SampleChannel::kill() {
	if (wave != NULL && status != STATUS_OFF) {
		if (mute || mute_i)
			stop();   /// FIXME - hardStop() is enough
		else
		if (status == STATUS_WAIT && mode & LOOP_ANY)
			hardStop();
		else
			setFadeOut(DO_STOP);
	}
}


/* ------------------------------------------------------------------ */


void SampleChannel::stopBySeq() {

	/* kill loop channels and recs if "samplesStopOnSeqHalt" == true,
	 * else do nothing and return */

	if (!G_Conf.chansStopOnSeqHalt)
		return;

	if (mode & (LOOP_BASIC | LOOP_ONCE | LOOP_REPEAT))
		kill();

	/** FIXME - unify these */

	/* when a channel has recs in play?
	 * Recorder has events for that channel
	 * G_Mixer has at least one sample in play
	 * Recorder's channel is active (altrimenti può capitare che
	 * si stoppino i sample suonati manualmente in un canale con rec
	 * disattivate) */

	if (hasActions && readActions && status == STATUS_PLAY)
		kill();
}


/* ------------------------------------------------------------------ */


void SampleChannel::stop() {
	if (mode == SINGLE_PRESS && status == STATUS_PLAY) {
		if (mute || mute_i)
			hardStop();
		else
			setFadeOut(DO_STOP);
	}
	else  // stop a SINGLE_PRESS immediately, if the quantizer is on
	if (mode == SINGLE_PRESS && qWait == true)
		qWait = false;
}


/* ------------------------------------------------------------------ */


int SampleChannel::load(const char *file) {

	if (strcmp(file, "") == 0 || gIsDir(file)) {
		puts("[SampleChannel] file not specified");
		return SAMPLE_LEFT_EMPTY;
	}

	if (strlen(file) > FILENAME_MAX)
		return SAMPLE_PATH_TOO_LONG;

	Wave *w = new Wave();

	if (!w->open(file)) {
		printf("[SampleChannel] %s: read error\n", file);
		delete w;
		return SAMPLE_READ_ERROR;
	}

	if (w->channels() > 2) {
		printf("[SampleChannel] %s: unsupported multichannel wave\n", file);
		delete w;
		return SAMPLE_MULTICHANNEL;
	}

	if (!w->readData()) {
		delete w;
		return SAMPLE_READ_ERROR;
	}

	if (w->channels() == 1) /** FIXME: error checking  */
		wfx_monoToStereo(w);

	if (w->rate() != G_Conf.samplerate) {
		printf("[SampleChannel] input rate (%d) != system rate (%d), conversion needed\n",
				w->rate(), G_Conf.samplerate);
		w->resample(G_Conf.rsmpQuality, G_Conf.samplerate);
	}

	pushWave(w);

	/* sample name must be unique. Start from k = 1, zero is too nerdy */

	std::string oldName = wave->name;
	int k = 1;

	while (!mh_uniqueSamplename(this, wave->name.c_str())) {
		char buf[4];
		sprintf(buf, "%d", k);
		wave->name = oldName + "-" + buf;
		k++;
	}

	printf("[SampleChannel] %s loaded in channel %d\n", file, index);
	return SAMPLE_LOADED_OK;
}


/* ------------------------------------------------------------------ */


int SampleChannel::loadByPatch(const char *f, int i) {

	int res = load(f);

	if (res == SAMPLE_LOADED_OK) {
		volume      = G_Patch.getVol(i);
		key         = G_Patch.getKey(i);
		index       = G_Patch.getIndex(i);
		mode        = G_Patch.getMode(i);
		mute        = G_Patch.getMute(i);
		mute_s      = G_Patch.getMute_s(i);
		solo        = G_Patch.getSolo(i);
		boost       = G_Patch.getBoost(i);
		panLeft     = G_Patch.getPanLeft(i);
		panRight    = G_Patch.getPanRight(i);
		readActions = G_Patch.getRecActive(i);
		recStatus   = readActions ? REC_READING : REC_STOPPED;

		readPatchMidiIn(i);

		setBegin(G_Patch.getBegin(i));
		setEnd  (G_Patch.getEnd(i, wave->size));
		setPitch(G_Patch.getPitch(i));
	}
	else {
		volume = DEFAULT_VOL;
		mode   = DEFAULT_CHANMODE;
		status = STATUS_WRONG;
		key    = 0;

		if (res == SAMPLE_LEFT_EMPTY)
			status = STATUS_EMPTY;
		else
		if (res == SAMPLE_READ_ERROR)
			status = STATUS_MISSING;
	}

	return res;
}


/* ------------------------------------------------------------------ */


bool SampleChannel::canInputRec() {
	return wave == NULL;
}


/* ------------------------------------------------------------------ */


void SampleChannel::start(bool doQuantize) {

	switch (status)	{
		case STATUS_EMPTY:
		case STATUS_MISSING:
		case STATUS_WRONG:
		{
			return;
		}

		case STATUS_OFF:
		{
			if (mode & LOOP_ANY)
				status = STATUS_WAIT;
			else
				if (G_Mixer.quantize > 0 && G_Mixer.running && doQuantize)
					qWait = true;
				else
					status = STATUS_PLAY;
			break;
		}

		case STATUS_PLAY:
		{
			if (mode == SINGLE_BASIC) {
				setFadeOut(DO_STOP);
			}
			else
			if (mode == SINGLE_RETRIG) {

				if (G_Mixer.quantize > 0 && G_Mixer.running && doQuantize) {
					qWait = true;
				}
				else {

					/* do a xfade only if the mute is off. An xfade on a mute channel
					 * introduces some bad clicks */

					if (mute)
						reset();
					else
						setXFade();
				}
			}
			else
			if (mode & (LOOP_ANY | SINGLE_ENDLESS))
				status = STATUS_ENDING;

			break;
		}

		case STATUS_WAIT:
		{
			status = STATUS_OFF;
			break;
		}

		case STATUS_ENDING:
		{
			status = STATUS_PLAY;
			break;
		}
	}
}


/* ------------------------------------------------------------------ */


void SampleChannel::writePatch(FILE *fp, int i, bool isProject) {

	const char *path = "";
	if (wave != NULL) {
		path = wave->pathfile.c_str();
		if (isProject)
			path = gBasename(path).c_str();  // make it portable
	}

	fprintf(fp, "samplepath%d=%s\n",     i, path);

	fprintf(fp, "chanSide%d=%d\n",       i, side);
	fprintf(fp, "chanType%d=%d\n",       i, type);
	fprintf(fp, "chanKey%d=%d\n",        i, key);
	fprintf(fp, "chanIndex%d=%d\n",      i, index);
	fprintf(fp, "chanmute%d=%d\n",       i, mute);
	fprintf(fp, "chanMute_s%d=%d\n",     i, mute_s);
	fprintf(fp, "chanSolo%d=%d\n",       i, solo);
	fprintf(fp, "chanvol%d=%f\n",        i, volume);
	fprintf(fp, "chanmode%d=%d\n",       i, mode);
	fprintf(fp, "chanBegin%d=%d\n",      i, begin);
	fprintf(fp, "chanend%d=%d\n",        i, end);
	fprintf(fp, "chanBoost%d=%f\n",      i, boost);
	fprintf(fp, "chanPanLeft%d=%f\n",    i, panLeft);
	fprintf(fp, "chanPanRight%d=%f\n",   i, panRight);
	fprintf(fp, "chanRecActive%d=%d\n",  i, readActions);
	fprintf(fp, "chanPitch%d=%f\n",      i, pitch);

	writePatchMidiIn(fp, i);
}


/* ------------------------------------------------------------------ */

void SampleChannel::processPitch() {
	data.data_in       = wave->data + tracker;
	data.input_frames  = kernelAudio::realBufsize * pitch; /// TODO - use private var
	data.data_out      = _procChan_;
	data.output_frames = kernelAudio::realBufsize; /// TODO - use private var
	data.end_of_input  = false;
	data.src_ratio     = 1/pitch;
	data = data;
	int res = src_process(converter, &data);
	printf("[sampleChannel] process pitch --- ratio=%f, frames_used=%lu frames_gen=%lu res=%d\n", data.src_ratio, data.input_frames_used, data.output_frames_gen, res);
	memcpy(vChan, _procChan_, kernelAudio::realBufsize * 2 * sizeof(float));
	src_reset(converter);
}
