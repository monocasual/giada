/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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
#include "kernelAudio.h"
#include "midiMapConf.h"
#include "sampleChannel.h"
#include "midiChannel.h"
#include "wave.h"


extern Patch_DEPR_ G_Patch_DEPR_;


using std::vector;
using std::string;


namespace giada {
namespace mh
{
namespace
{
#ifdef WITH_VST

int readPatchPlugins(vector<patch::plugin_t> *list, int type)
{
	int ret = 1;
	for (unsigned i=0; i<list->size(); i++) {
		patch::plugin_t *ppl = &list->at(i);
    // TODO use glue_addPlugin()
		Plugin *plugin = pluginHost::addPlugin(ppl->path.c_str(), type,
				&mixer::mutex_plugins, nullptr);
		if (plugin != nullptr) {
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


/* ------------------------------------------------------------------------ */


int getNewChanIndex()
{
	/* always skip last channel: it's the last one just added */

	if (mixer::channels.size() == 1)
		return 0;

	int index = 0;
	for (unsigned i=0; i<mixer::channels.size()-1; i++) {
		if (mixer::channels.at(i)->index > index)
			index = mixer::channels.at(i)->index;
		}
	index += 1;
	return index;
}


}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


bool uniqueSampleName(SampleChannel *ch, const string &name)
{
	for (unsigned i=0; i<mixer::channels.size(); i++) {
		if (ch == mixer::channels.at(i))  // skip itself
			continue;
		if (mixer::channels.at(i)->type != CHANNEL_SAMPLE)
			continue;
		SampleChannel *other = (SampleChannel*) mixer::channels.at(i);
		if (other->wave != nullptr && name == other->wave->name)
			return false;
	}
	return true;
}


/* -------------------------------------------------------------------------- */


Channel *addChannel(int type)
{
  Channel *ch;
	int bufferSize = kernelAudio::getRealBufSize() * 2;

	if (type == CHANNEL_SAMPLE)
		ch = new SampleChannel(bufferSize);
	else
		ch = new MidiChannel(bufferSize);

	while (true) {
		if (pthread_mutex_trylock(&mixer::mutex_chans) != 0)
      continue;
		mixer::channels.push_back(ch);
		pthread_mutex_unlock(&mixer::mutex_chans);
		break;
	}

	ch->index = getNewChanIndex();
	gu_log("[addChannel] channel index=%d added, type=%d, total=%d\n",
    ch->index, ch->type, mixer::channels.size());
	return ch;
}


/* -------------------------------------------------------------------------- */


int deleteChannel(Channel *ch)
{
	int index = -1;
	for (unsigned i=0; i<mixer::channels.size(); i++) {
		if (mixer::channels.at(i) == ch) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		gu_log("[deleteChannel] unable to find channel %d for deletion!\n", ch->index);
		return 0;
	}

	while (true) {
		if (pthread_mutex_trylock(&mixer::mutex_chans) != 0)
      continue;
		mixer::channels.erase(mixer::channels.begin() + index);
		delete ch;
		pthread_mutex_unlock(&mixer::mutex_chans);
		return 1;
	}
}


/* -------------------------------------------------------------------------- */


Channel *getChannelByIndex(int index)
{
	for (unsigned i=0; i<mixer::channels.size(); i++)
		if (mixer::channels.at(i)->index == index)
			return mixer::channels.at(i);
	gu_log("[getChannelByIndex] channel at index %d not found!\n", index);
	return nullptr;
}


/* -------------------------------------------------------------------------- */


bool hasLogicalSamples()
{
	for (unsigned i=0; i<mixer::channels.size(); i++) {
    if (mixer::channels.at(i)->type != CHANNEL_SAMPLE)
      continue;
    SampleChannel *ch = static_cast<SampleChannel*>(mixer::channels.at(i));
    if (ch->wave && ch->wave->isLogical)
      return true;
  }
	return false;
}


/* -------------------------------------------------------------------------- */


bool hasEditedSamples()
{
	for (unsigned i=0; i<mixer::channels.size(); i++)
  {
		if (mixer::channels.at(i)->type != CHANNEL_SAMPLE)
      continue;
    SampleChannel *ch = static_cast<SampleChannel*>(mixer::channels.at(i));
    if (ch->wave && ch->wave->isEdited)
      return true;
  }
	return false;
}


/* -------------------------------------------------------------------------- */


void stopSequencer()
{
  clock::stop();
	for (unsigned i=0; i<mixer::channels.size(); i++)
		mixer::channels.at(i)->stopBySeq(conf::chansStopOnSeqHalt);
}


/* -------------------------------------------------------------------------- */


bool uniqueSolo(Channel *ch)
{
	int solos = 0;
	for (unsigned i=0; i<mixer::channels.size(); i++) {
		Channel *ch = mixer::channels.at(i);
		if (ch->solo) solos++;
		if (solos > 1) return false;
	}
	return true;
}


/* -------------------------------------------------------------------------- */


/** TODO - revision needed: mh should not call glue_addChannel */

void loadPatch_DEPR_(bool isProject, const char *projPath)
{
	mixer::init(clock::getTotalFrames(), kernelAudio::getRealBufSize());
	mixer::ready = false;   // put it in wait mode

	int numChans = G_Patch_DEPR_.getNumChans();
	for (int i=0; i<numChans; i++) {
		Channel *ch = glue_addChannel(G_Patch_DEPR_.getColumn(i), G_Patch_DEPR_.getType(i));
		string projectPath = projPath;  // safe
		string samplePath  = isProject ? projectPath + G_SLASH + G_Patch_DEPR_.getSamplePath(i) : "";
		ch->readPatch_DEPR_(samplePath.c_str(), i, &G_Patch_DEPR_, conf::samplerate,
				conf::rsmpQuality);
	}

	mixer::outVol     = G_Patch_DEPR_.getOutVol();
	mixer::inVol      = G_Patch_DEPR_.getInVol();
	clock::setBpm(G_Patch_DEPR_.getBpm());
	clock::setBars(G_Patch_DEPR_.getBars());
	clock::setBeats(G_Patch_DEPR_.getBeats());
	clock::setQuantize(G_Patch_DEPR_.getQuantize());
	mixer::metronome  = G_Patch_DEPR_.getMetronome();
	G_Patch_DEPR_.lastTakeId = G_Patch_DEPR_.getLastTakeId();
	G_Patch_DEPR_.samplerate = G_Patch_DEPR_.getSamplerate();

	/* rewind and update frames in Mixer (it's vital) */

	mixer::rewind();
	clock::updateFrameBars();
	mixer::ready = true;
}


/* -------------------------------------------------------------------------- */


void readPatch()
{
	mixer::ready = false;

	mixer::outVol     = patch::masterVolOut;
	mixer::inVol      = patch::masterVolIn;
	clock::setBpm(patch::bpm);
	clock::setBars(patch::bars);
	clock::setBeats(patch::beats);
	clock::setQuantize(patch::quantize);
	mixer::metronome  = patch::metronome;

#ifdef WITH_VST

	readPatchPlugins(&patch::masterInPlugins, pluginHost::MASTER_IN);
	readPatchPlugins(&patch::masterOutPlugins, pluginHost::MASTER_OUT);

#endif

	/* rewind and update frames in Mixer (it's essential) */

	mixer::rewind();
	clock::updateFrameBars();
	mixer::ready = true;
}


/* -------------------------------------------------------------------------- */


void rewindSequencer()
{
	if (clock::getQuantize() > 0 && clock::isRunning())   // quantize rewind
		mixer::rewindWait = true;
	else
		mixer::rewind();
}


/* -------------------------------------------------------------------------- */


bool startInputRec()
{
	int channelsReady = 0;

	for (unsigned i=0; i<mixer::channels.size(); i++) {

		if (!mixer::channels.at(i)->canInputRec())
			continue;

		SampleChannel *ch = (SampleChannel*) mixer::channels.at(i);

		/* Allocate empty sample for the current channel. */

		if (!ch->allocEmpty(clock::getTotalFrames(), conf::samplerate, patch::lastTakeId))
		{
			gu_log("[startInputRec] unable to allocate new Wave in chan %d!\n",
				ch->index);
			continue;
		}

		/* Increase lastTakeId until the sample name TAKE-[n] is unique */

		while (!uniqueSampleName(ch, ch->wave->name)) {
			G_Patch_DEPR_.lastTakeId++;
			patch::lastTakeId++;
			ch->wave->name = "TAKE-" + gu_itoa(patch::lastTakeId);
		}

		gu_log("[startInputRec] start input recs using chan %d with size %d "
			"frame=%d\n", ch->index, clock::getTotalFrames(), mixer::inputTracker);

		channelsReady++;
	}

	if (channelsReady > 0) {
		mixer::recording = true;
		/* start to write from the currentFrame, not the beginning */
		/** FIXME: this should be done before wave allocation */
		mixer::inputTracker = clock::getCurrentFrame();
		return true;
	}
	return false;
}


/* -------------------------------------------------------------------------- */


void stopInputRec()
{
	mixer::mergeVirtualInput();
	mixer::recording = false;
	mixer::waitRec = 0; // in case delay compensation is in use
	gu_log("[mh] stop input recs\n");
}


/* -------------------------------------------------------------------------- */


bool hasArmedSampleChannels()
{
  for (unsigned i=0; i<mixer::channels.size(); i++) {
    Channel *ch = mixer::channels.at(i);
    if (ch->type == CHANNEL_SAMPLE && ch->armed)
      return true;
  }
  return false;
}


}}; // giada::mh::
