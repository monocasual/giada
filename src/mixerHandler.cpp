/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * mixerHandler
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


#include "mixerHandler.h"
#include "mixer.h"
#include "const.h"
#include "utils.h"
#include "init.h"
#include "pluginHost.h"
#include "plugin.h"
#include "waveFx.h"
#include "glue.h"
#include "conf.h"
#include "patch.h"
#include "recorder.h"
#include "channel.h"


extern Mixer 		  G_Mixer;
extern Patch 		  G_Patch;
extern Conf 		  G_Conf;

#ifdef WITH_VST
extern PluginHost G_PluginHost;
#endif


void mh_startChan(channel *ch, bool do_quantize) {

	switch (ch->status) {
		case STATUS_EMPTY:
		case STATUS_MISSING:
		case STATUS_WRONG:
		{
			return;
		}

		case STATUS_OFF:
		{
			if (ch->mode & LOOP_ANY)
				ch->status = STATUS_WAIT;
			else
				if (G_Mixer.quantize > 0 && G_Mixer.running && do_quantize)
					ch->qWait = true;
				else
					ch->status = STATUS_PLAY;
			break;
		}

		case STATUS_PLAY:
		{
			if (ch->mode == SINGLE_BASIC) {
				G_Mixer.fadeout(ch);
			}
			else
			if (ch->mode == SINGLE_RETRIG) {

				if (G_Mixer.quantize > 0 && G_Mixer.running && do_quantize) {
					ch->qWait = true;
				}
				else {

					/* do a xfade only if the mute is off. An xfade on a mute channel
					 * introduces some bad clics */

					if (ch->mute)
						G_Mixer.chanReset(ch);
					else
						G_Mixer.xfade(ch);
				}
			}
			else
			if (ch->mode & LOOP_ANY)
				ch->status = STATUS_ENDING;

			break;
		}

		case STATUS_WAIT:
		{
			ch->status = STATUS_OFF;
			break;
		}

		case STATUS_ENDING:
		{
			ch->status = STATUS_PLAY;
			break;
		}
	}
}


/* ------------------------------------------------------------------ */


void mh_stopChan(channel *ch) {
	if (ch == NULL)
		return;

	if (ch->status == STATUS_PLAY && ch->mode == SINGLE_PRESS) {
		if (ch->mute || ch->mute_i)
			G_Mixer.chanStop(ch);
		else
			G_Mixer.fadeout(ch, Mixer::DO_STOP);
	}

	/* stop a SINGLE_PRESS immediately, if the quantizer is on */

	else
	if (ch->mode == SINGLE_PRESS && ch->qWait == true)
		ch->qWait = false;
}


/* ------------------------------------------------------------------ */


void mh_killChan(channel *ch) {
	if (ch->wave != NULL && ch->status != STATUS_OFF) {
		if (ch->mute || ch->mute_i)
			G_Mixer.chanStop(ch);
		else
			G_Mixer.fadeout(ch, Mixer::DO_STOP);
	}
}


/* ------------------------------------------------------------------ */


void mh_muteChan(channel *ch, bool internal) {
	if (internal) {
		if (ch->mute)          // global mute? don't waste time with fadeout,
			ch->mute_i = true;   // just mute it internally
		else
			if (G_Mixer.isPlaying(ch))
				G_Mixer.fadeout(ch, Mixer::DO_MUTE_I);
			else
				ch->mute_i = true;
	}
	else {
		if (ch->mute_i)        // internal mute? don't waste time with fadeout,
			ch->mute = true;     // just mute it globally
		else
			if (G_Mixer.isPlaying(ch))              // sample in play? fadeout needed. Else,
				G_Mixer.fadeout(ch, Mixer::DO_MUTE);  // just mute it globally
			else
				ch->mute = true;
	}
}


/* ------------------------------------------------------------------ */


void mh_unmuteChan(channel *ch, bool internal) {
	if (internal) {
		if (ch->mute)
			ch->mute_i = false;
		else
			if (G_Mixer.isPlaying(ch))
				G_Mixer.fadein(ch, internal);
			else
				ch->mute_i = false;
	}
	else {
		if (ch->mute_i)
			ch->mute = false;
		else
			if (G_Mixer.isPlaying(ch))
				G_Mixer.fadein(ch, internal);
			else
				ch->mute = false;
	}
}


/* ------------------------------------------------------------------ */


void mh_deleteChannel(channel *ch) {
	/*
	if (ch == NULL)
		return;
	if (ch->wave == NULL)
		return;
	if (ch->status == STATUS_MISSING) {
		ch->status = STATUS_EMPTY;  // returns to init state
	}
	else {
		int i = ch->index;
		G_Mixer.deleteChannel(ch);
		printf("[MH] channel %d freed\n", i);
	}
	*/
	int i = ch->index;
	G_Mixer.deleteChannel(ch);
	printf("[MH] channel %d freed\n", i);
}


/* ------------------------------------------------------------------ */


void mh_freeChannel(channel *ch) {
	G_Mixer.freeChannel(ch);
}


/* ------------------------------------------------------------------ */


int mh_loadChan(const char *file, struct channel *ch) {

	if (strcmp(file, "") == 0) {
		puts("[MH] file not specified");
		return SAMPLE_LEFT_EMPTY;
	}

	if (strlen(file) > FILENAME_MAX)
		return SAMPLE_PATH_TOO_LONG;

	Wave *w = new Wave();

	if(!w->open(file)) {
		printf("[MH] %s: read error\n", file);
		delete w;
		return SAMPLE_READ_ERROR;
	}

	if (w->inHeader.channels > 2) {
		printf("[MH] %s: unsupported multichannel wave\n", file);
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
		printf("[MH] input rate (%d) != system rate (%d), conversion needed\n", w->inHeader.samplerate, G_Conf.samplerate);
		w->resample(G_Conf.rsmpQuality, G_Conf.samplerate);
	}

	G_Mixer.pushChannel(w, ch);

	/* sample name must be unique */

	std::string sampleName = gBasename(stripExt(file).c_str());
	int k = 0;
	bool exists = false;
	do {
		for (unsigned i=0; i<G_Mixer.channels.size; i++) {
			channel *thatCh = G_Mixer.channels.at(i);
			if (thatCh->wave != NULL && thatCh->index != ch->index) {  // skip itself
				if (ch->wave->name == thatCh->wave->name) {
					char n[32];
					sprintf(n, "%d", k);
					ch->wave->name = sampleName + "-" + n;
					exists = true;
					break;
				}
			}
			exists = false;
		}
		k++;
	}
	while (exists);

	printf("[MH] %s loaded in channel %d\n", file, ch->index);
	return SAMPLE_LOADED_OK;
}


/* ------------------------------------------------------------------ */


void mh_loadPatch() {
	G_Mixer.init();

	int numChans = G_Patch.getNumChans();
	for (int i=0; i<numChans; i++) {
		channel *ch = glue_addChannel(G_Patch.getSide(i));
		int res = mh_loadChan(G_Patch.getSamplePath(i).c_str(), ch);

		if (res == SAMPLE_LOADED_OK) {
			ch->volume      = G_Patch.getVol(i);
			ch->index       = G_Patch.getIndex(i);
			ch->mode        = G_Patch.getMode(i);
			ch->mute        = G_Patch.getMute(i);
			ch->boost       = G_Patch.getBoost(i);
			ch->panLeft     = G_Patch.getPanLeft(i);
			ch->panRight    = G_Patch.getPanRight(i);
			ch->tracker     = ch->start;
			ch->readActions = G_Patch.getRecActive(i);
			ch->recStatus   = ch->readActions ? REC_READING : REC_STOPPED;
			G_Mixer.setChanStart(ch, G_Patch.getStart(i));
			G_Mixer.setChanEnd  (ch, G_Patch.getEnd(i, ch->wave->size));
			G_Mixer.setPitch    (ch, G_Patch.getPitch(i));
		}
		else {
			ch->volume = DEFAULT_VOL;
			ch->mode   = DEFAULT_CHANMODE;
			ch->status = STATUS_WRONG;

			if (res == SAMPLE_LEFT_EMPTY)
				ch->status = STATUS_EMPTY;
			else
			if (res == SAMPLE_READ_ERROR)
				ch->status = STATUS_MISSING;
		}
	}

	G_Mixer.outVol     = G_Patch.getOutVol();
	G_Mixer.inVol      = G_Patch.getInVol();
	G_Mixer.bpm        = G_Patch.getBpm();
	G_Mixer.bars       = G_Patch.getBars();
	G_Mixer.beats      = G_Patch.getBeats();
	G_Mixer.quantize   = G_Patch.getQuantize();
	G_Mixer.metronome  = G_Patch.getMetronome();
	G_Patch.lastTakeId = G_Patch.getLastTakeId();
	G_Patch.samplerate = G_Patch.getSamplerate();

	/* rewind and update frames in Mixer (it's vital) */

	G_Mixer.rewind();
	G_Mixer.updateFrameBars();
}


/* ------------------------------------------------------------------ */


void mh_rewind() {

	if (G_Mixer.quantize > 0 && G_Mixer.running)
		G_Mixer.rewindWait = true;
	else
		G_Mixer.rewind();
}


/* ------------------------------------------------------------------ */


channel *mh_startInputRec() {

	if (!G_Mixer.running)
		return NULL;

	/* search for the next available channel */

	channel *chan = NULL;
	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		if (G_Mixer.channels.at(i)->wave == NULL) {
			chan = G_Mixer.channels.at(i);
			break;
		}
	}

	/* no chans available? */

	if (chan == NULL)
		return NULL;

	Wave *w = new Wave();
	if (!w->allocEmpty(G_Mixer.totalFrames))
		return NULL;

	/* pick up the next __TAKE_(n+1)__ */

	char buf[256];
	sprintf(buf, "__TAKE_%d__", G_Patch.lastTakeId);

	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		channel *ch = G_Mixer.channels.at(i);
		if (ch->wave != NULL) {
			if (strcmp(buf, ch->wave->name.c_str()) == 0)
				G_Patch.lastTakeId += 1;
			else
				break;
		}
	}
	sprintf(buf, "__TAKE_%d__", G_Patch.lastTakeId);

	w->pathfile = getCurrentPath()+"/"+buf;
	w->name     = buf;
	G_Patch.lastTakeId += 1;

	G_Mixer.pushChannel(w, chan);
	G_Mixer.chanInput = chan;

	/* start to write from the actualFrame, not the beginning */
	/** FIXME: move this before wave allocation*/

	G_Mixer.inputTracker = G_Mixer.actualFrame;

	printf(
		"[mh] start input recs using chan %d with size %d, frame=%d\n",
		chan->index, G_Mixer.totalFrames, G_Mixer.inputTracker
	);

	return chan;
}


/* ------------------------------------------------------------------ */


channel *mh_stopInputRec() {
	printf("[mh] stop input recs\n");
	G_Mixer.mergeVirtualInput();
	channel *ch = G_Mixer.chanInput;
	G_Mixer.chanInput = NULL;
	G_Mixer.waitRec   = 0;					// if delay compensation is in use
	return ch;
}
