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


extern Mixer 		  G_Mixer;
extern Patch 		  G_Patch;
extern Conf 		  G_Conf;

#ifdef WITH_VST
extern PluginHost G_PluginHost;
#endif


void mh_startChan(int c, bool do_quantize) {

	channel *ch = G_Mixer.channels.at(c);

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
				G_Mixer.fadeout(c);
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
						G_Mixer.chanReset(c);
					else
						G_Mixer.xfade(c);
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
			G_Mixer.chanStop(ch->index);
		else
			G_Mixer.fadeout(ch->index, Mixer::DO_STOP);
	}

	/* stop a SINGLE_PRESS immediately, if the quantizer is on */

	else
	if (ch->mode == SINGLE_PRESS && ch->qWait == true)
		ch->qWait = false;
}


/* ------------------------------------------------------------------ */


void mh_killChan(int c) {
	channel *ch = G_Mixer.channels.at(c);
	if (ch->wave != NULL && ch->status != STATUS_OFF) {
		if (ch->mute || ch->mute_i)
			G_Mixer.chanStop(c);
		else
			G_Mixer.fadeout(c, Mixer::DO_STOP);
	}
}


/* ------------------------------------------------------------------ */


void mh_muteChan(int c, bool internal) {
	channel *ch = G_Mixer.channels.at(c);
	if (internal) {
		if (ch->mute)          // global mute? don't waste time with fadeout,
			ch->mute_i = true;   // just mute it internally
		else
			if (G_Mixer.isPlaying(c))
				G_Mixer.fadeout(c, Mixer::DO_MUTE_I);
			else
				ch->mute_i = true;
	}
	else {
		if (ch->mute_i)        // internal mute? don't waste time with fadeout,
			ch->mute = true;     // just mute it globally
		else
			if (G_Mixer.isPlaying(c))              // sample in play? fadeout needed. Else,
				G_Mixer.fadeout(c, Mixer::DO_MUTE);  // just mute it globally
			else
				ch->mute = true;
	}
}


/* ------------------------------------------------------------------ */


void mh_unmuteChan(int c, bool internal) {
	channel *ch = G_Mixer.channels.at(c);
	if (internal) {
		if (ch->mute)
			ch->mute_i = false;
		else
			if (G_Mixer.isPlaying(c))
				G_Mixer.fadein(c, internal);
			else
				ch->mute_i = false;
	}
	else {
		if (ch->mute_i)
			ch->mute = false;
		else
			if (G_Mixer.isPlaying(c))
				G_Mixer.fadein(c, internal);
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


int mh_loadChan(const char *file, struct channel *ch, bool push) {

	if (strcmp(file, "") == 0) {
		puts("[MH] file not specified");
		return SAMPLE_LEFT_EMPTY;
	}

	else if (strlen(file) > FILENAME_MAX)
		return SAMPLE_PATH_TOO_LONG;

	else {
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

		channel *thisCh;

		if (push) {
			G_Mixer.pushChannel(w, ch);
			thisCh = ch;
		}
		else {
			thisCh = G_Mixer.loadChannel(w, 0);
		}

		/* sample name must be unique */

		std::string sampleName = gBasename(stripExt(file).c_str());
		int k = 0;
		bool exists = false;
		do {
			for (unsigned i=0; i<G_Mixer.channels.size; i++) {
				channel *thatCh = G_Mixer.channels.at(i);
				if (thatCh->wave != NULL && thatCh->index != thisCh->index) {  // skip itself
					if (thisCh->wave->name == thatCh->wave->name) {
						char n[32];
						sprintf(n, "%d", k);
						thisCh->wave->name = sampleName + "-" + n;
						exists = true;
						break;
					}
				}
				exists = false;
			}
			k++;
		}
		while (exists);

		printf("[MH] %s loaded in channel %d\n", file, thisCh->index);
		return SAMPLE_LOADED_OK;
	}
}


/* ------------------------------------------------------------------ */


void mh_loadPatch() {
	G_Mixer.init();

#if 0

	int i = 0;

	while (i < MAX_NUM_CHAN) {

		/* kills all channels for safety. Useful if the patch is loaded on
		 * the fly. */

		mh_killChan(i);

		/* let's load the sample */

		int res = mh_loadChan(G_Patch.getSamplePath(i).c_str(), i);

		if (res == SAMPLE_LOADED_OK) {
			G_Mixer.chanVolume[i]    = G_Patch.getVol(i);
			G_Mixer.chanMode[i]      = G_Patch.getMode(i);
			G_Mixer.chanMute[i]      = G_Patch.getMute(i);
			//if (G_Patch.getMute(i))	   glue_readMute(i, ACTION_MUTEON); // glue_ will activate the gui ('R' button)
			G_Mixer.chanBoost[i]     = G_Patch.getBoost(i);
			G_Mixer.chanPanLeft[i]   = G_Patch.getPanLeft(i);
			G_Mixer.chanPanRight[i]  = G_Patch.getPanRight(i);
			G_Mixer.chanTracker[i]   = G_Mixer.chanStart[i];
			recorder::chanActive[i]  = G_Patch.getRecActive(i); // gu_update_controls will activate the gui (bg color)
			G_Mixer.chanRecStatus[i] = recorder::chanActive[i] ? REC_READING : REC_STOPPED;
			G_Mixer.setChanStart(i, G_Patch.getStart(i));
			G_Mixer.setChanEnd  (i, G_Patch.getEnd(i, G_Mixer.chan[i]->size));
			G_Mixer.setPitch    (i, G_Patch.getPitch(i));
		}
		else {
			G_Mixer.chanVolume[i] = DEFAULT_VOL;
			G_Mixer.chanMode[i]   = DEFAULT_CHANMODE;
			G_Mixer.chanStatus[i] = STATUS_WRONG;

			if (res == SAMPLE_LEFT_EMPTY)
				G_Mixer.chanStatus[i] = STATUS_EMPTY;
			else
			if (res == SAMPLE_READ_ERROR)
				G_Mixer.chanStatus[i] = STATUS_MISSING;
		}
		i++;
	}

#endif

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
	G_Mixer.chanInput = chan->index;

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


int mh_stopInputRec() {
	printf("[mh] stop input recs\n");
	G_Mixer.mergeVirtualInput();
	int chan = G_Mixer.chanInput;
	G_Mixer.chanInput = -1;
	G_Mixer.waitRec   = 0;					// if delay compensation is in use
	return chan;
}
