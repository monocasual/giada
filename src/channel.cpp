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
#include "conf.h"
#include "waveFx.h"


extern Patch       G_Patch;
extern Mixer       G_Mixer;
extern Conf        G_Conf;
#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif


channel::channel(int type, int status, char side)
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
	  vChan      (NULL),
	  recStatus  (REC_STOPPED),
	  readActions(false),
	  hasActions (false)
{
	gVector <class Plugin *> p; // call gVector constructor with p, and using it as the real gVector
	plugins = p;                /// fixme - is it really useful?
}


/* ------------------------------------------------------------------ */


void channel::clear(int bufSize) {
	memset(vChan, 0, sizeof(float) * bufSize);
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


MidiChannel::MidiChannel(char side)
	: channel    (CHANNEL_MIDI, STATUS_OFF, side),
	  midiOut    (false),
	  midiOutChan(MIDI_CHANS[0])
{
	readActions = true;

#ifdef WITH_VST // init VstEvents stack
	G_PluginHost.freeVstMidiEvents(this, true);
#endif
}


/* ------------------------------------------------------------------ */


void MidiChannel::process(float *buffer, int size) {
#ifdef WITH_VST
	G_PluginHost.processStack(vChan, PluginHost::CHANNEL, this);
	G_PluginHost.freeVstMidiEvents(this);
#endif
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
		kernelMidi::send(MIDI_ALL_NOTES_OFF, this);
	status = STATUS_OFF;
}


/* ------------------------------------------------------------------ */


int MidiChannel::load(const char *f) {
	status = STATUS_OFF;
	return SAMPLE_LOADED_OK;
}


/* ------------------------------------------------------------------ */


void MidiChannel::stop() {
	// nothing to do
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
	return SAMPLE_LOADED_OK;  /// TODO - change name, is meaningless here
}


/* ------------------------------------------------------------------ */


bool MidiChannel::canInputRec() {
	return false;  // midi channel can't record input
}


/* ------------------------------------------------------------------ */


Wave *MidiChannel::getWave() {
	return NULL;
}


/* ------------------------------------------------------------------ */


void MidiChannel::sendMidi(recorder::action *a) {
	if (status & (STATUS_PLAY | STATUS_ENDING) && !mute) {
		kernelMidi::send(a->iValue | MIDI_CHANS[midiOutChan], this);
#ifdef WITH_VST
		G_PluginHost.addVstMidiEvent(a->event, this);
#endif
	}
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
	: channel    (CHANNEL_SAMPLE, STATUS_EMPTY, side),
		wave       (NULL),
		index      (0),
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
		key        (0)
{
	vChan = (float *) malloc(kernelAudio::realBufsize * 2 * sizeof(float));
	if (!vChan)
		printf("[sampleChannel] unable to alloc memory for this vChan\n");
}


/* ------------------------------------------------------------------ */


SampleChannel::~SampleChannel() {
	status = STATUS_OFF;
	if (vChan)
		free(vChan);
	if (wave)
		delete wave;
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


Wave *SampleChannel::getWave() {
	return wave;
}


/* ------------------------------------------------------------------ */


void SampleChannel::kill() {
	if (wave != NULL && status != STATUS_OFF) {
		if (mute || mute_i)
			G_Mixer.chanStop(this);  /// remove!
		else
			G_Mixer.fadeout(this, Mixer::DO_STOP);
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
			G_Mixer.chanStop(this);  /// remove!
		else
			G_Mixer.fadeout(this, Mixer::DO_STOP);
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
		printf("[SampleChannel] input rate (%d) != system rate (%d), conversion needed\n", w->inHeader.samplerate, G_Conf.samplerate);
		w->resample(G_Conf.rsmpQuality, G_Conf.samplerate);
	}

	pushWave(w);

	/* sample name must be unique */
	/** TODO - use mh_uniqueSamplename */

	std::string sampleName = gBasename(stripExt(file).c_str());
	int k = 0;
	bool exists = false;
	do {
		for (unsigned i=0; i<G_Mixer.channels.size; i++) {
			channel *thatCh = G_Mixer.channels.at(i);
			if (thatCh->getWave() && thatCh->index != index) {  // skip itself
				if (wave->name == thatCh->getWave()->name) {
					char n[32];
					sprintf(n, "%d", k);
					wave->name = sampleName + "-" + n;
					exists = true;
					break;
				}
			}
			exists = false;
		}
		k++;
	}
	while (exists);

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

		G_Mixer.setChanStart(this, G_Patch.getStart(i));
		G_Mixer.setChanEnd  (this, G_Patch.getEnd(i, wave->size));
		G_Mixer.setPitch    (this, G_Patch.getPitch(i));
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


void SampleChannel::sendMidi(recorder::action *a) {
	return;
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
				G_Mixer.fadeout(this);
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
						G_Mixer.chanReset(this);
					else
						G_Mixer.xfade(this);
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

