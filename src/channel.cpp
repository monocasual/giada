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


#include "channel.h"
#include "pluginHost.h"
#include "kernelAudio.h"
#include "kernelMidi.h"
#include "patch.h"
#include "wave.h"
#include "mixer.h"
#include "mixerHandler.h"
#include "conf.h"
#include "waveFx.h"


extern Patch       G_Patch;
extern Mixer       G_Mixer;
extern Conf        G_Conf;
#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif


Channel::Channel(int type, int status, char side)
	: type       (type),
		status     (status),
		side       (side),
	  volume     (DEFAULT_VOL),
	  volume_i   (1.0f),
	  volume_d   (0.0f),
	  panLeft    (1.0f),
	  panRight   (1.0f),
	  mute_i     (false),
	  mute_s     (false),
	  mute       (false),
	  solo       (false),
	  hasActions (false),
	  recStatus  (REC_STOPPED),
	  vChan      (NULL)
{
	gVector <class Plugin *> p; // call gVector constructor with p, and using it as the real gVector
	plugins = p;                /// fixme - is it really useful?

	vChan = (float *) malloc(kernelAudio::realBufsize * 2 * sizeof(float));
	if (!vChan)
		printf("[Channel] unable to alloc memory for vChan\n");
}


/* ------------------------------------------------------------------ */


void Channel::clear(int bufSize) {
	memset(vChan, 0, sizeof(float) * bufSize);
}


/* ------------------------------------------------------------------ */


bool Channel::isPlaying() {
	return status == STATUS_PLAY || status == STATUS_ENDING;
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


MidiChannel::MidiChannel(char side)
	: Channel    (CHANNEL_MIDI, STATUS_OFF, side),
	  midiOut    (false),
	  midiOutChan(MIDI_CHANS[0])
{
#ifdef WITH_VST // init VstEvents stack
	freeVstMidiEvents(true);
#endif
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST

void MidiChannel::freeVstMidiEvents(bool init) {
	if (events.numEvents == 0 && !init)
		return;
	memset(events.events, 0, sizeof(VstEvent*) * MAX_VST_EVENTS);
	events.numEvents = 0;
	events.reserved  = 0;
}

#endif


/* ------------------------------------------------------------------ */


#ifdef WITH_VST

void MidiChannel::addVstMidiEvent(uint32_t msg) {
	addVstMidiEvent(G_PluginHost.createVstMidiEvent(msg));
}

#endif


/* ------------------------------------------------------------------ */


#ifdef WITH_VST

void MidiChannel::addVstMidiEvent(VstMidiEvent *e) {
	if (events.numEvents < MAX_VST_EVENTS) {
		events.events[events.numEvents] = (VstEvent*) e;
		events.numEvents++;
		//printf("[MidiChannel] VstMidiEvent added to channel %d, total = %d\n", index, events.numEvents);
	}
	else
		printf("[MidiChannel] channel %d VstEvents = %d > MAX_VST_EVENTS, nothing to do\n", index, events.numEvents);
}

#endif


/* ------------------------------------------------------------------ */


void MidiChannel::onBar() {}


/* ------------------------------------------------------------------ */


void MidiChannel::stop() {}


/* ------------------------------------------------------------------ */


void MidiChannel::empty() {}


/* ------------------------------------------------------------------ */


void MidiChannel::quantize(int index, int frame) {}


/* ------------------------------------------------------------------ */

#ifdef WITH_VST

VstEvents *MidiChannel::getVstEvents() {
	return (VstEvents *) &events;
}

#endif


/* ------------------------------------------------------------------ */


void MidiChannel::parseAction(recorder::action *a, int frame) {
	if (a->type == ACTION_MIDI)
		sendMidi(a);
}


/* ------------------------------------------------------------------ */


void MidiChannel::onZero() {
	if (status == STATUS_ENDING)
		status = STATUS_OFF;
	else
	if (status == STATUS_WAIT)
		status = STATUS_PLAY;
}


/* ------------------------------------------------------------------ */


void MidiChannel::setMute(bool internal) {
	// internal mute does not exist for midi (for now)
	mute = true;
	kernelMidi::send(MIDI_ALL_NOTES_OFF);
}


/* ------------------------------------------------------------------ */


void MidiChannel::unsetMute(bool internal) {
	// internal mute does not exist for midi (for now)
	mute = false;
}


/* ------------------------------------------------------------------ */


void MidiChannel::process(float *buffer, int size) {
#ifdef WITH_VST
	G_PluginHost.processStack(vChan, PluginHost::CHANNEL, this);
	freeVstMidiEvents();
#endif

	for (int j=0; j<size; j+=2) {
		buffer[j]   += vChan[j]   * volume; // * panLeft;   future?
		buffer[j+1] += vChan[j+1] * volume; // * panRight;  future?
	}
}


/* ------------------------------------------------------------------ */


void MidiChannel::start(bool doQuantize) {
	status = STATUS_WAIT;
}


/* ------------------------------------------------------------------ */


void MidiChannel::stopBySeq() {
	kill();
}


/* ------------------------------------------------------------------ */


void MidiChannel::kill() {
	if (status & (STATUS_PLAY | STATUS_ENDING))
		kernelMidi::send(MIDI_ALL_NOTES_OFF);
	status = STATUS_OFF;
}


/* ------------------------------------------------------------------ */


int MidiChannel::loadByPatch(const char *f, int i) {
	volume      = G_Patch.getVol(i);
	index       = G_Patch.getIndex(i);
	mute        = G_Patch.getMute(i);
	mute_s      = G_Patch.getMute_s(i);
	solo        = G_Patch.getSolo(i);
	panLeft     = G_Patch.getPanLeft(i);
	panRight    = G_Patch.getPanRight(i);
	midiOut     = G_Patch.getMidiOut(i);
	midiOutChan = G_Patch.getMidiOutChan(i);
	return SAMPLE_LOADED_OK;  /// TODO - change name, it's meaningless here
}


/* ------------------------------------------------------------------ */


void MidiChannel::sendMidi(recorder::action *a) {
	if (status & (STATUS_PLAY | STATUS_ENDING) && !mute) {
		if (midiOut)
			kernelMidi::send(a->iValue | MIDI_CHANS[midiOutChan]);
#ifdef WITH_VST
		addVstMidiEvent(a->event);
#endif
	}
}


void MidiChannel::sendMidi(uint32_t data) {
	if (status & (STATUS_PLAY | STATUS_ENDING) && !mute) {
		if (midiOut)
			kernelMidi::send(data | MIDI_CHANS[midiOutChan]);
#ifdef WITH_VST
		addVstMidiEvent(data);
#endif
	}
}


/* ------------------------------------------------------------------ */


void MidiChannel::rewind() {
	if (midiOut)
		kernelMidi::send(MIDI_ALL_NOTES_OFF);
#ifdef WITH_VST
		addVstMidiEvent(MIDI_ALL_NOTES_OFF);
#endif
}


/* ------------------------------------------------------------------ */


void MidiChannel::writePatch(FILE *fp, int i, bool isProject) {
	fprintf(fp, "chanSide%d=%d\n",        i, side);
	fprintf(fp, "chanType%d=%d\n",        i, type);
	fprintf(fp, "chanIndex%d=%d\n",       i, index);
	fprintf(fp, "chanmute%d=%d\n",        i, mute);
	fprintf(fp, "chanMute_s%d=%d\n",      i, mute_s);
	fprintf(fp, "chanSolo%d=%d\n",        i, solo);
	fprintf(fp, "chanvol%d=%f\n",         i, volume);
	fprintf(fp, "chanPanLeft%d=%f\n",     i, panLeft);
	fprintf(fp, "chanPanRight%d=%f\n",    i, panRight);
	fprintf(fp, "chanMidiOut%d=%d\n",     i, midiOut);
	fprintf(fp, "chanMidiOutChan%d=%d\n", i, midiOutChan);
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


SampleChannel::SampleChannel(char side)
	: Channel    (CHANNEL_SAMPLE, STATUS_EMPTY, side),
		wave       (NULL),
		tracker    (0),
		begin      (0),
		end        (0),
		beginTrue  (0),
		endTrue    (0),
		pitch      (gDEFAULT_PITCH),
		boost      (1.0f),
		mode       (DEFAULT_CHANMODE),
		qWait	     (false),
		fadein     (1.0f),
		fadeoutOn  (false),
		fadeoutVol (1.0f),
		fadeoutStep(DEFAULT_FADEOUT_STEP),
		key        (0),
	  readActions(false)
{}


/* ------------------------------------------------------------------ */


SampleChannel::~SampleChannel() {
	status = STATUS_OFF;
	if (vChan)
		free(vChan);
	if (wave)
		delete wave;
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
	beginTrue = v;
	begin     = (unsigned) floorf(beginTrue / pitch);
	tracker   = begin;
}


/* ------------------------------------------------------------------ */


void SampleChannel::setEnd(unsigned v) {
	if (v % 2 != 0)
		v++;
	endTrue = v;
	end = (unsigned) floorf(endTrue / pitch);
}


/* ------------------------------------------------------------------ */


void SampleChannel::setPitch(float v) {

	/* if the pitch changes also chanStart/chanEnd must change accordingly
	 * and to do that we need the original (or previous) chanStart/chanEnd
	 * values (chanStartTrue and chanEndTrue). Formula:
	 *
	 * chanStart{pitched} = chanStart{Original} / newpitch */

	if (v == 1.0f) {
		begin = beginTrue;
		end   = endTrue;
		pitch = 1.0f;
		return;
	}

	begin = (unsigned) floorf(beginTrue / v);
	end   = (unsigned) floorf(endTrue   / v);

	pitch = v;

	/* even values please */

	if (begin % 2 != 0)	begin++;
	if (end   % 2 != 0)	end++;

	/* avoid overflow when changing pitch during play mode */

	if (tracker > end)
		tracker = end;
}


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
			if (mode & SINGLE_ANY) {    /// FIXME: break outside if
				start(false);
				break;
			}
		case ACTION_KEYREL:
			if (mode & SINGLE_ANY) {    /// FIXME: break outside if
				stop();
				break;
			}
		case ACTION_KILLCHAN:
			if (mode & SINGLE_ANY) {    /// FIXME: break outside if
				kill();
				break;
			}
		case ACTION_MUTEON:
			setMute(true);  // internal mute
			break;
		case ACTION_MUTEOFF:
			unsetMute(true); // internal mute
			break;
		case ACTION_VOLUME:
			G_Mixer.calcVolumeEnv(this, frame);  /// REMOVE THIS!
			break;
	}
}


/* ------------------------------------------------------------------ */


void SampleChannel::sum(int frame, bool running) {

	if (wave == NULL)
		return;

	if (status & (STATUS_PLAY | STATUS_ENDING)) {

		if (tracker <= end) {

			/* ctp is chanTracker, pitch affected */

			unsigned ctp = tracker * pitch;

			/* fade in */

			if (fadein <= 1.0f)
				fadein += 0.01f;		/// FIXME - remove the hardcoded value

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

						unsigned ftp = fadeoutTracker * pitch;

						vChan[frame]   += wave->data[ftp]   * fadeoutVol * v;
						vChan[frame+1] += wave->data[ftp+1] * fadeoutVol * v;

						vChan[frame]   += wave->data[ctp]   * v;
						vChan[frame+1] += wave->data[ctp+1] * v;

					}
					else { // FADEOUT
						vChan[frame]   += wave->data[ctp]   * fadeoutVol * v;
						vChan[frame+1] += wave->data[ctp+1] * fadeoutVol * v;
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
					vChan[frame]   += wave->data[ctp]   * v;
					vChan[frame+1] += wave->data[ctp+1] * v;
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

	if (status == STATUS_WAIT)
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
	beginTrue = 0;
	end       = wave->size;
	endTrue   = wave->size;
}


/* ------------------------------------------------------------------ */


bool SampleChannel::allocEmpty(int frames, int takeId) {

	Wave *w = new Wave();
	if (!w->allocEmpty(frames))
		return false;

	char wname[32];
	sprintf(wname, "__TAKE_%d__", takeId);

	w->pathfile = getCurrentPath()+"/"+wname;
	w->name     = wname;
	wave        = w;
	status      = STATUS_OFF;
	begin       = 0;
	beginTrue   = 0;
	end         = wave->size;
	endTrue     = wave->size;

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
			stop();
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
	if (status == STATUS_PLAY && mode == SINGLE_PRESS) {
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

	if (w->inHeader.channels > 2) {
		printf("[SampleChannel] %s: unsupported multichannel wave\n", file);
		delete w;
		return SAMPLE_MULTICHANNEL;
	}

	if (!w->readData()) {
		delete w;
		return SAMPLE_READ_ERROR;
	}

	if (w->inHeader.channels == 1) /** FIXME: error checking  */
		wfx_monoToStereo(w);

	if (w->inHeader.samplerate != G_Conf.samplerate) {
		printf("[SampleChannel] input rate (%d) != system rate (%d), conversion needed\n",
				w->inHeader.samplerate, G_Conf.samplerate);
		w->resample(G_Conf.rsmpQuality, G_Conf.samplerate);
	}

	pushWave(w);

	/* sample name must be unique. Start from k = 1, zero is too nerdy */

	std::string oldName = wave->name;
	int k = 1;

	while (!mh_uniqueSamplename(this, wave->name)) {
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

		setBegin(G_Patch.getStart(i));
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

	fprintf(fp, "samplepath%d=%s\n",    i, path);
	fprintf(fp, "chanSide%d=%d\n",      i, side);
	fprintf(fp, "chanType%d=%d\n",      i, type);
	fprintf(fp, "chanKey%d=%d\n",       i, key);
	fprintf(fp, "chanIndex%d=%d\n",     i, index);
	fprintf(fp, "chanmute%d=%d\n",      i, mute);
	fprintf(fp, "chanMute_s%d=%d\n",    i, mute_s);
	fprintf(fp, "chanSolo%d=%d\n",      i, solo);
	fprintf(fp, "chanvol%d=%f\n",       i, volume);
	fprintf(fp, "chanmode%d=%d\n",      i, mode);
	fprintf(fp, "chanBegin%d=%d\n",     i, beginTrue);       // true values, not pitched
	fprintf(fp, "chanend%d=%d\n",       i, endTrue);         // true values, not pitched
	fprintf(fp, "chanBoost%d=%f\n",     i, boost);
	fprintf(fp, "chanPanLeft%d=%f\n",   i, panLeft);
	fprintf(fp, "chanPanRight%d=%f\n",  i, panRight);
	fprintf(fp, "chanRecActive%d=%d\n", i, readActions);
	fprintf(fp, "chanPitch%d=%f\n",     i, pitch);
}

