/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * mixerHandler
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
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

extern Mixer 		  G_Mixer;
extern Patch 		  G_Patch;
extern Conf 		  G_Conf;

#ifdef WITH_VST
extern PluginHost G_PluginHost;
#endif



void mh_startChan(int c, bool do_quantize) {

	switch (G_Mixer.chanStatus[c]) {

		case STATUS_EMPTY:
		case STATUS_MISSING:
		case STATUS_WRONG:
		{
			return;
		}

		case STATUS_OFF:
		{
			if (G_Mixer.chanMode[c] & LOOP_ANY)
				G_Mixer.chanStatus[c] = STATUS_WAIT;
			else
				if (G_Mixer.quantize > 0 && G_Mixer.running && do_quantize)
					G_Mixer.chanQWait[c] = true;
				else
					G_Mixer.chanStatus[c] = STATUS_PLAY;
			break;
		}

		case STATUS_PLAY:
		{
			if (G_Mixer.chanMode[c] == SINGLE_BASIC) {
				G_Mixer.fadeout(c);
			}
			else
			if (G_Mixer.chanMode[c] == SINGLE_RETRIG) {

				if (G_Mixer.quantize > 0 && G_Mixer.running && do_quantize) {
					G_Mixer.chanQWait[c] = true;
				}
				else {

					/* do a xfade only if the mute is off. An xfade on a mute channel
					 * introduces some bad clics */

					if (G_Mixer.chanMuteVol[c] == 0.0f)
						G_Mixer.chanReset(c);
					else
						G_Mixer.xfade(c);
				}
			}
			else
			if (G_Mixer.chanMode[c] & LOOP_ANY)
				G_Mixer.chanStatus[c] = STATUS_ENDING;

			break;
		}

		case STATUS_WAIT:
		{
			G_Mixer.chanStatus[c] = STATUS_OFF;
			break;
		}

		case STATUS_ENDING:
		{
			G_Mixer.chanStatus[c] = STATUS_PLAY;
			break;
		}
	}
}


/* ------------------------------------------------------------------ */


void mh_stopChan(int c) {
	if (G_Mixer.chanStatus[c] == STATUS_PLAY) {
		if (G_Mixer.chanMode[c] == SINGLE_PRESS) {

			/* no fadeout if chan is muted */

			if (G_Mixer.chanMuteVol[c] == 0.0f)
				G_Mixer.chanStop(c);
			else
				G_Mixer.fadeout(c, DO_STOP);
		}
	}

	/* stop a SINGLE_PRESS immediately, if the quantizer is on */

	else
	if (G_Mixer.chanMode[c] == SINGLE_PRESS && G_Mixer.chanQWait[c] == true)
		G_Mixer.chanQWait[c] = false;
}


/* ------------------------------------------------------------------ */


void mh_killChan(int c) {

	if (G_Mixer.chan[c] != NULL && G_Mixer.chanStatus[c] != STATUS_OFF) {
		if (G_Mixer.chanMuteVol[c] == 0.0f)
			G_Mixer.chanStop(c);
		else
			G_Mixer.fadeout(c, DO_STOP);
	}
}



/* ------------------------------------------------------------------ */


void mh_muteChan(int c) {
	G_Mixer.chanMute[c] = true;
	if (G_Mixer.chanStatus[c] == STATUS_PLAY || G_Mixer.chanStatus[c] == STATUS_ENDING)
		G_Mixer.fadeout(c, DO_MUTE);
	else
		G_Mixer.chanMuteVol[c] = 0.0f;
}
void mh_unmuteChan(int c) {
	G_Mixer.chanMute[c] = false;
	if (G_Mixer.chanStatus[c] == STATUS_PLAY || G_Mixer.chanStatus[c] == STATUS_ENDING)
		G_Mixer.fadein(c);
	else
		G_Mixer.chanMuteVol[c] = 1.0f;
}


/* ------------------------------------------------------------------ */


void mh_freeChan(int c) {

	if (G_Mixer.chanStatus[c] == STATUS_MISSING) {
		G_Mixer.chanStatus[c] = STATUS_EMPTY;  // returns to init state
	}
	else if (G_Mixer.chan[c] != NULL) {
		G_Mixer.freeWave(c);
		delete G_Mixer.chan[c];
		G_Mixer.chan[c] = NULL;
		printf("[MH] channel %d free\n", c+1);
	}
}


/* ------------------------------------------------------------------ */


int mh_loadChan(const char *file, int c) {

	if (strcmp(file, "") == 0) {
		printf("[MH] channel %d | file not specified\n", c+1);
		return SAMPLE_LEFT_EMPTY;
	}

	else if (strlen(file) > FILENAME_MAX)
		return SAMPLE_PATH_TOO_LONG;

	else {
		Wave *w = new Wave();

		if(!w->open(file)) {
			printf("[MH] channel %d | %s: read error\n", c+1, file);
			delete w;
			return SAMPLE_READ_ERROR;
		}

		if (w->inHeader.channels > 2) {
			printf("[MH] channel %d | %s: unsupported multichannel wave\n", c+1, file);
			delete w;
			return SAMPLE_MULTICHANNEL;
		}

		if (!w->readData()) {
			delete w;
			return SAMPLE_READ_ERROR;
		}

		if (w->inHeader.channels == 1) {
			/** FIXME: error checking + new constant SAMPLE_CONVERSION_ERROR */
			wfx_monoToStereo(w);
		}

		mh_freeChan(c);  // free the previous sample
		G_Mixer.loadWave(w, c);

		/* sample name must be unique */

		std::string sampleName = gBasename(stripExt(file).c_str());
		int k = 0;
		bool exists = false;
		do {
			for (int i=0; i<MAX_NUM_CHAN; i++) {
				if (G_Mixer.chan[i] != NULL && i != c) {  // skip itself
					if (G_Mixer.chan[c]->name == G_Mixer.chan[i]->name) {
						char n[32];
						sprintf(n, "%d", k);
						G_Mixer.chan[c]->name = sampleName + "-" + n;
						exists = true;
						break;
					}
				}
				exists = false;
			}
			k++;
		}
		while (exists);

		printf("[MH] channel %d | %s loaded\n", c+1, file);
		return SAMPLE_LOADED_OK;
	}
}


/* ------------------------------------------------------------------ */


void mh_loadPatch() {
	G_Mixer.init();
	int i = 0;
	while (i < MAX_NUM_CHAN) {

		/* for safety kills all channels. Useful if the patch is loaded on
		 * the fly. */

		mh_killChan(i);

		/* let's load the sample */

		int res = mh_loadChan(G_Patch.getSamplePath(i).c_str(), i);

		if (res == SAMPLE_LOADED_OK) {
			G_Mixer.chanVolume[i]    = G_Patch.getVol(i);
			G_Mixer.chanMode[i]      = G_Patch.getMode(i);
			if (G_Patch.getMute(i))	   glue_readMute(i, ACTION_MUTEON); // glue_ will activate the gui ('R' button)
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

	/*
	if (G_Mixer.running) {
		if (G_Mixer.quantize > 0)
			G_Mixer.rewindWait = true;
		else
			G_Mixer.rewind();
	}*/
}


/* ------------------------------------------------------------------ */


int mh_startInputRec() {

	if (!G_Mixer.running)
		return -1;

	/* search for the next available channel */

	int chan = -1;
	for (unsigned i=0; i<MAX_NUM_CHAN; i++) {
		if (G_Mixer.chan[i] == NULL) {
			chan = i;
			break;
		}
	}

	/* no chans available? */

	if (chan == -1)
		return -1;

	Wave *w = new Wave();
	if (!w->allocEmpty(G_Mixer.totalFrames))
		return -1;

	/* pick up the next __TAKE_(n+1)__ */

	char buf[256];
	sprintf(buf, "__TAKE_%d__", G_Patch.lastTakeId);

	for (unsigned i=0; i<MAX_NUM_CHAN; i++) {
		if (G_Mixer.chan[i] != NULL) {
			if (strcmp(buf, G_Mixer.chan[i]->name.c_str()) == 0)
				G_Patch.lastTakeId += 1;
			else
				break;
		}
	}
	sprintf(buf, "__TAKE_%d__", G_Patch.lastTakeId);

	w->pathfile = getCurrentPath()+"/"+buf;
	w->name     = buf;
	G_Patch.lastTakeId += 1;

	G_Mixer.loadWave(w, chan);
	G_Mixer.chanInput = chan;

	/* start to write from the actualFrame, not the beginning */
	/** FIXME: move this before wave allocation*/

	G_Mixer.inputTracker = G_Mixer.actualFrame;

	printf(
		"[mh] start input recs using chan %d with size %d, frame=%d\n",
		chan, G_Mixer.totalFrames, G_Mixer.inputTracker
	);

	return chan;
}


/* ------------------------------------------------------------------ */


int mh_stopInputRec() {
	printf("[mh] stop input recs\n");
	int chan = G_Mixer.chanInput;
	G_Mixer.chanInput = -1;
	G_Mixer.waitRec   = 0;					// if delay compensation is in use
	return chan;
}
