/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * mixerHandler
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


#if defined(__linux__)
	#include <jack/jack.h>
	#include <jack/intclient.h>
	#include <jack/transport.h>
#endif

#include <vector>
#include "../utils/utils.h"
#include "../utils/log.h"
#include "../glue/glue.h"
#include "../glue/channel.h"
#include "mixerHandler.h"
#include "kernelMidi.h"
#include "mixer.h"
#include "const.h"
#include "init.h"
#include "pluginHost.h"
#include "plugin.h"
#include "waveFx.h"
#include "conf.h"
#include "patch_DEPR_.h"
#include "patch.h"
#include "recorder.h"
#include "channel.h"
#include "sampleChannel.h"
#include "wave.h"


extern Mixer 		   G_Mixer;
extern Patch_DEPR_ G_Patch_DEPR_;
extern Patch       G_Patch;
extern Conf 		   G_Conf;

#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif


using std::vector;


#ifdef WITH_VST

static int __mh_readPatchPlugins__(vector<Patch::plugin_t> *list, int type)
{
	int ret = 1;
	for (unsigned i=0; i<list->size(); i++) {
		Patch::plugin_t *ppl = &list->at(i);
		Plugin *plugin = G_PluginHost.addPlugin(ppl->path.c_str(), type, NULL);
		if (plugin != NULL) {
			plugin->setBypass(ppl->bypass);
			for (unsigned j=0; j<ppl->params.size(); j++)
				plugin->setParameter(j, ppl->params.at(j));
			ret &= 1;
		}
		else
			ret &= 0;
	}
	return ret;
}

#endif


/* -------------------------------------------------------------------------- */


void mh_stopSequencer()
{
	G_Mixer.running = false;
	for (unsigned i=0; i<G_Mixer.channels.size(); i++)
		G_Mixer.channels.at(i)->stopBySeq();
}


/* -------------------------------------------------------------------------- */


bool mh_uniqueSolo(Channel *ch)
{
	int solos = 0;
	for (unsigned i=0; i<G_Mixer.channels.size(); i++) {
		Channel *ch = G_Mixer.channels.at(i);
		if (ch->solo) solos++;
		if (solos > 1) return false;
	}
	return true;
}


/* -------------------------------------------------------------------------- */


/** TODO - revision needed: mh should not call glue_addChannel */

void mh_loadPatch_DEPR_(bool isProject, const char *projPath)
{
	G_Mixer.init();
	G_Mixer.ready = false;   // put it in wait mode

	int numChans = G_Patch_DEPR_.getNumChans();
	for (int i=0; i<numChans; i++) {
		Channel *ch = glue_addChannel(G_Patch_DEPR_.getColumn(i), G_Patch_DEPR_.getType(i));
		string samplePath = isProject ? projPath + gGetSlash() + G_Patch_DEPR_.getSamplePath(i) : "";
		ch->readPatch_DEPR_(samplePath.c_str(), i);
	}

	G_Mixer.outVol     = G_Patch_DEPR_.getOutVol();
	G_Mixer.inVol      = G_Patch_DEPR_.getInVol();
	G_Mixer.bpm        = G_Patch_DEPR_.getBpm();
	G_Mixer.bars       = G_Patch_DEPR_.getBars();
	G_Mixer.beats      = G_Patch_DEPR_.getBeats();
	G_Mixer.quantize   = G_Patch_DEPR_.getQuantize();
	G_Mixer.metronome  = G_Patch_DEPR_.getMetronome();
	G_Patch_DEPR_.lastTakeId = G_Patch_DEPR_.getLastTakeId();
	G_Patch_DEPR_.samplerate = G_Patch_DEPR_.getSamplerate();

	/* rewind and update frames in Mixer (it's vital) */

	G_Mixer.rewind();
	G_Mixer.updateFrameBars();
	G_Mixer.ready = true;
}


/* -------------------------------------------------------------------------- */


void mh_readPatch()
{
	G_Mixer.ready = false;

	G_Mixer.outVol     = G_Patch.masterVolOut;
	G_Mixer.inVol      = G_Patch.masterVolIn;
	G_Mixer.bpm        = G_Patch.bpm;
	G_Mixer.bars       = G_Patch.bars;
	G_Mixer.beats      = G_Patch.beats;
	G_Mixer.quantize   = G_Patch.quantize;
	G_Mixer.metronome  = G_Patch.metronome;

#ifdef WITH_VST

	__mh_readPatchPlugins__(&G_Patch.masterInPlugins, PluginHost::MASTER_IN);
	__mh_readPatchPlugins__(&G_Patch.masterOutPlugins, PluginHost::MASTER_OUT);

#endif

	/* rewind and update frames in Mixer (it's essential) */

	G_Mixer.rewind();
	G_Mixer.updateFrameBars();

	G_Mixer.ready = true;
}


/* -------------------------------------------------------------------------- */


void mh_rewindSequencer()
{
	if (G_Mixer.quantize > 0 && G_Mixer.running)   // quantize rewind
		G_Mixer.rewindWait = true;
	else
		G_Mixer.rewind();
}


/* -------------------------------------------------------------------------- */


SampleChannel *mh_startInputRec()
{
	/* search for the next available channel */

	SampleChannel *chan = NULL;
	for (unsigned i=0; i<G_Mixer.channels.size(); i++) {
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
	if (!w->allocEmpty(G_Mixer.totalFrames, G_Conf.samplerate))
		return NULL;

	/* increase lastTakeId until the sample name TAKE-[n] is unique */

	char name[32];
	sprintf(name, "TAKE-%d", G_Patch_DEPR_.lastTakeId);
	while (!mh_uniqueSamplename(chan, name)) {
		G_Patch_DEPR_.lastTakeId++;
		G_Patch.lastTakeId++;
		sprintf(name, "TAKE-%d", G_Patch_DEPR_.lastTakeId);
	}

	chan->allocEmpty(G_Mixer.totalFrames, G_Patch_DEPR_.lastTakeId);
	G_Mixer.chanInput = chan;

	/* start to write from the actualFrame, not the beginning */
	/** FIXME: move this before wave allocation*/

	G_Mixer.inputTracker = G_Mixer.actualFrame;

	gLog(
		"[mh] start input recs using chan %d with size %d, frame=%d\n",
		chan->index, G_Mixer.totalFrames, G_Mixer.inputTracker
	);

	return chan;
}


/* -------------------------------------------------------------------------- */


SampleChannel *mh_stopInputRec()
{
	gLog("[mh] stop input recs\n");
	G_Mixer.mergeVirtualInput();
	SampleChannel *ch = G_Mixer.chanInput;
	G_Mixer.chanInput = NULL;
	G_Mixer.waitRec   = 0;					// if delay compensation is in use
	return ch;
}


/* -------------------------------------------------------------------------- */


bool mh_uniqueSamplename(SampleChannel *ch, const char *name)
{
	for (unsigned i=0; i<G_Mixer.channels.size(); i++) {
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
