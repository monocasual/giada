/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * mixerHandler
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include <vector>
#include "../utils/fs.h"
#include "../utils/string.h"
#include "../utils/log.h"
#include "../glue/main.h"
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
#include "clock.h"
#include "channel.h"
#include "sampleChannel.h"
#include "wave.h"


extern Mixer 		   G_Mixer;
extern Patch_DEPR_ G_Patch_DEPR_;
extern Patch       G_Patch;
extern Conf 		   G_Conf;
extern Clock       G_Clock;

#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif


using std::vector;
using std::string;


#ifdef WITH_VST

static int __mh_readPatchPlugins__(vector<Patch::plugin_t> *list, int type)
{
	int ret = 1;
	for (unsigned i=0; i<list->size(); i++) {
		Patch::plugin_t *ppl = &list->at(i);
    // TODO use glue_addPlugin()
		Plugin *plugin = G_PluginHost.addPlugin(ppl->path.c_str(), type,
				&G_Mixer.mutex_plugins, NULL);
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
  G_Clock.stop();
	for (unsigned i=0; i<G_Mixer.channels.size(); i++)
		G_Mixer.channels.at(i)->stopBySeq(G_Conf.chansStopOnSeqHalt);
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
		string projectPath = projPath;  // safe
		string samplePath  = isProject ? projectPath + G_SLASH + G_Patch_DEPR_.getSamplePath(i) : "";
		ch->readPatch_DEPR_(samplePath.c_str(), i, &G_Patch_DEPR_, G_Conf.samplerate,
				G_Conf.rsmpQuality);
	}

	G_Mixer.outVol     = G_Patch_DEPR_.getOutVol();
	G_Mixer.inVol      = G_Patch_DEPR_.getInVol();
	G_Clock.setBpm(G_Patch_DEPR_.getBpm());
	G_Clock.setBars(G_Patch_DEPR_.getBars());
	G_Clock.setBeats(G_Patch_DEPR_.getBeats());
	G_Clock.setQuantize(G_Patch_DEPR_.getQuantize());
	G_Mixer.metronome  = G_Patch_DEPR_.getMetronome();
	G_Patch_DEPR_.lastTakeId = G_Patch_DEPR_.getLastTakeId();
	G_Patch_DEPR_.samplerate = G_Patch_DEPR_.getSamplerate();

	/* rewind and update frames in Mixer (it's vital) */

	G_Mixer.rewind();
	G_Clock.updateFrameBars();
	G_Mixer.ready = true;
}


/* -------------------------------------------------------------------------- */


void mh_readPatch()
{
	G_Mixer.ready = false;

	G_Mixer.outVol     = G_Patch.masterVolOut;
	G_Mixer.inVol      = G_Patch.masterVolIn;
	G_Clock.setBpm(G_Patch.bpm);
	G_Clock.setBars(G_Patch.bars);
	G_Clock.setBeats(G_Patch.beats);
	G_Clock.setQuantize(G_Patch.quantize);
	G_Mixer.metronome  = G_Patch.metronome;

#ifdef WITH_VST

	__mh_readPatchPlugins__(&G_Patch.masterInPlugins, PluginHost::MASTER_IN);
	__mh_readPatchPlugins__(&G_Patch.masterOutPlugins, PluginHost::MASTER_OUT);

#endif

	/* rewind and update frames in Mixer (it's essential) */

	G_Mixer.rewind();
	G_Clock.updateFrameBars();
	G_Mixer.ready = true;
}


/* -------------------------------------------------------------------------- */


void mh_rewindSequencer()
{
	if (G_Clock.getQuantize() > 0 && G_Clock.isRunning())   // quantize rewind
		G_Mixer.rewindWait = true;
	else
		G_Mixer.rewind();
}


/* -------------------------------------------------------------------------- */


bool mh_startInputRec()
{
	int channelsReady = 0;

	for (unsigned i=0; i<G_Mixer.channels.size(); i++) {

		if (!G_Mixer.channels.at(i)->canInputRec())
			continue;

		SampleChannel *ch = (SampleChannel*) G_Mixer.channels.at(i);

		/* Allocate empty sample for the current channel. */

		if (!ch->allocEmpty(G_Clock.getTotalFrames(), G_Conf.samplerate, G_Patch.lastTakeId))
		{
			gu_log("[mh_startInputRec] unable to allocate new Wave in chan %d!\n",
				ch->index);
			continue;
		}

		/* Increase lastTakeId until the sample name TAKE-[n] is unique */

		while (!mh_uniqueSampleName(ch, ch->wave->name)) {
			G_Patch_DEPR_.lastTakeId++;
			G_Patch.lastTakeId++;
			ch->wave->name = "TAKE-" + gu_itoa(G_Patch.lastTakeId);
		}

		gu_log("[mh_startInputRec] start input recs using chan %d with size %d "
			"frame=%d\n", ch->index, G_Clock.getTotalFrames(), G_Mixer.inputTracker);

		channelsReady++;
	}

	if (channelsReady > 0) {
		G_Mixer.recording = true;
		/* start to write from the currentFrame, not the beginning */
		/** FIXME: this should be done before wave allocation */
		G_Mixer.inputTracker = G_Clock.getCurrentFrame();
		return true;
	}
	return false;
}


/* -------------------------------------------------------------------------- */


void mh_stopInputRec()
{
	G_Mixer.mergeVirtualInput();
	G_Mixer.recording = false;
	G_Mixer.waitRec = 0; // in case delay compensation is in use
	gu_log("[mh] stop input recs\n");
}


/* -------------------------------------------------------------------------- */


bool mh_hasArmedSampleChannels()
{
  for (unsigned i=0; i<G_Mixer.channels.size(); i++) {
    Channel *ch = G_Mixer.channels.at(i);
    if (ch->type == CHANNEL_SAMPLE && ch->armed)
      return true;
  }
  return false;
}


/* -------------------------------------------------------------------------- */


bool mh_uniqueSampleName(SampleChannel *ch, const string &name)
{
	for (unsigned i=0; i<G_Mixer.channels.size(); i++) {
		if (ch == G_Mixer.channels.at(i))  // skip itself
			continue;
		if (G_Mixer.channels.at(i)->type != CHANNEL_SAMPLE)
			continue;
		SampleChannel *other = (SampleChannel*) G_Mixer.channels.at(i);
		if (other->wave != NULL && name == other->wave->name)
			return false;
	}
	return true;
}
