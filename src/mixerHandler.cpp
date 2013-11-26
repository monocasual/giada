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
#include "kernelMidi.h"
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
#include "sampleChannel.h"
#include "wave.h"


extern Mixer 		  G_Mixer;
extern Patch 		  G_Patch;
extern Conf 		  G_Conf;

#ifdef WITH_VST
extern PluginHost G_PluginHost;
#endif


void mh_stopSequencer() {
	G_Mixer.running = false;
	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		G_Mixer.channels.at(i)->stopBySeq();
}


/* ------------------------------------------------------------------ */


bool mh_uniqueSolo(Channel *ch) {
	int solos = 0;
	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		Channel *ch = G_Mixer.channels.at(i);
		if (ch->solo) solos++;
		if (solos > 1) return false;
	}
	return true;
}


/* ------------------------------------------------------------------ */


/** TODO - revision needed: mh should not call glue_addChannel */

void mh_loadPatch(bool isProject, const char *projPath) {

	G_Mixer.init();
	G_Mixer.ready = false;   // put it in wait mode

	int numChans = G_Patch.getNumChans();
	for (int i=0; i<numChans; i++) {

		Channel *ch = glue_addChannel(G_Patch.getSide(i), G_Patch.getType(i));

		char smpPath[PATH_MAX];

		/* projects < 0.6.3 version are not portable. Just use the regular
		 * samplePath */

		if (isProject && G_Patch.version >= 0.63f)
#if defined(_WIN32)
			sprintf(smpPath, "%s\\%s", gDirname(projPath).c_str(), G_Patch.getSamplePath(i).c_str());
#else
			sprintf(smpPath, "%s/%s", gDirname(projPath).c_str(), G_Patch.getSamplePath(i).c_str());
#endif
		else
			sprintf(smpPath, "%s", G_Patch.getSamplePath(i).c_str());

		ch->loadByPatch(smpPath, i);
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
	G_Mixer.ready = true;
}


/* ------------------------------------------------------------------ */


void mh_rewindSequencer() {
	if (G_Mixer.quantize > 0 && G_Mixer.running)   // quantize rewind
		G_Mixer.rewindWait = true;
	else
		G_Mixer.rewind();
}


/* ------------------------------------------------------------------ */


SampleChannel *mh_startInputRec() {

	/* search for the next available channel */

	SampleChannel *chan = NULL;
	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		if (G_Mixer.channels.at(i)->type == CHANNEL_SAMPLE)
			if (((SampleChannel*) G_Mixer.channels.at(i))->canInputRec()) {
			chan = (SampleChannel*) G_Mixer.channels.at(i);
			break;
		}
	}

	/* no chans available? */

	if (chan == NULL)
		return NULL;

	Wave *w = new Wave();
	if (!w->allocEmpty(G_Mixer.totalFrames))
		return NULL;

	/* increase lastTakeId until the sample name TAKE-[n] is unique */

	char name[32];
	sprintf(name, "TAKE-%d", G_Patch.lastTakeId);
	while (!mh_uniqueSamplename(chan, name)) {
		G_Patch.lastTakeId++;
		sprintf(name, "TAKE-%d", G_Patch.lastTakeId);
	}

	chan->allocEmpty(G_Mixer.totalFrames, G_Patch.lastTakeId);
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


SampleChannel *mh_stopInputRec() {
	printf("[mh] stop input recs\n");
	G_Mixer.mergeVirtualInput();
	SampleChannel *ch = G_Mixer.chanInput;
	G_Mixer.chanInput = NULL;
	G_Mixer.waitRec   = 0;					// if delay compensation is in use
	return ch;
}


/* ------------------------------------------------------------------ */


bool mh_uniqueSamplename(SampleChannel *ch, const char *name) {
	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		if (ch != G_Mixer.channels.at(i)) {
			if (G_Mixer.channels.at(i)->type == CHANNEL_SAMPLE) {
				SampleChannel *other = (SampleChannel*) G_Mixer.channels.at(i);
				if (other->wave != NULL)
					if (!strcmp(name, other->wave->name.c_str()))
						return false;
			}
		}
	}
	return true;
}
