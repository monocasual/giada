/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * channel
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


#include "../utils/log.h"
#include "../gui/elems/ge_channel.h"
#include "channel.h"
#include "pluginHost.h"
#include "plugin.h"
#include "kernelMidi.h"
#include "patch_DEPR_.h"
#include "patch.h"
#include "wave.h"
#include "mixer.h"
#include "mixerHandler.h"
#include "conf.h"
#include "patch.h"
#include "waveFx.h"
#include "midiMapConf.h"


extern Patch_DEPR_ G_Patch_DEPR_;
extern Patch       G_Patch;
extern Mixer       G_Mixer;
extern Conf        G_Conf;
extern MidiMapConf G_MidiMap;
#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif


Channel::Channel(int type, int status, int bufferSize)
: bufferSize(bufferSize),
  type      (type),
	status    (status),
	key       (0),
  volume    (DEFAULT_VOL),
  volume_i  (1.0f),
  volume_d  (0.0f),
  panLeft   (1.0f),
  panRight  (1.0f),
  mute_i    (false),
  mute_s    (false),
  mute      (false),
  solo      (false),
  hasActions(false),
  recStatus (REC_STOPPED),
  vChan     (NULL),
  guiChannel(NULL),
  midiIn         (true),
  midiInKeyPress (0x0),
  midiInKeyRel   (0x0),
  midiInKill     (0x0),
  midiInVolume   (0x0),
  midiInMute     (0x0),
  midiInSolo     (0x0),
  midiOutL       (false),
  midiOutLplaying(0x0),
  midiOutLmute   (0x0),
  midiOutLsolo   (0x0)
{
  vChan = (float *) malloc(bufferSize * sizeof(float));
	if (!vChan)
		gLog("[Channel::allocVchan] unable to alloc memory for vChan\n");
	memset(vChan, 0, bufferSize * sizeof(float));
}


/* -------------------------------------------------------------------------- */


Channel::~Channel()
{
	status = STATUS_OFF;
	if (vChan)
		free(vChan);
}


/* -------------------------------------------------------------------------- */


void Channel::copy(const Channel *src)
{
  key             = src->key;
  volume          = src->volume;
  volume_i        = src->volume_i;
  volume_d        = src->volume_d;
  panLeft         = src->panLeft;
  panRight        = src->panRight;
  mute_i          = src->mute_i;
  mute_s          = src->mute_s;
  mute            = src->mute;
  solo            = src->solo;
  hasActions      = src->hasActions;
  recStatus       = src->recStatus;
  midiIn          = src->midiIn;
  midiInKeyPress  = src->midiInKeyPress;
  midiInKeyRel    = src->midiInKeyRel;
  midiInKill      = src->midiInKill;
  midiInVolume    = src->midiInVolume;
  midiInMute      = src->midiInMute;
  midiInSolo      = src->midiInSolo;
  midiOutL        = src->midiOutL;
  midiOutLplaying = src->midiOutLplaying;
  midiOutLmute    = src->midiOutLmute;
  midiOutLsolo    = src->midiOutLsolo;

  /* clone plugins */

#ifdef WITH_VST
  for (unsigned i=0; i<src->plugins.size(); i++)
    G_PluginHost.clonePlugin(src->plugins.at(i), PluginHost::CHANNEL,
      &G_Mixer.mutex_plugins, this);
#endif

  /* clone actions */

  for (unsigned i=0; i<recorder::global.size(); i++) {
    for (unsigned k=0; k<recorder::global.at(i).size(); k++) {
      recorder::action *a = recorder::global.at(i).at(k);
      if (a->chan == src->index)
        recorder::rec(index, a->type, a->frame, a->iValue, a->fValue);
    }
  }
}


/* -------------------------------------------------------------------------- */


void Channel::sendMidiLmessage(uint32_t learn, const MidiMapConf::message_t &msg)
{
	gLog("[channel::sendMidiLmessage] learn=%#X, chan=%d, msg=%#X, offset=%d\n",
		learn, msg.channel, msg.value, msg.offset);

	/* isolate 'channel' from learnt message and offset it as requested by 'nn'
	 * in the midimap configuration file. */

		uint32_t out = ((learn & 0x00FF0000) >> 16) << msg.offset;

	/* merge the previously prepared channel into final message, and finally
	 * send it. */

	out |= msg.value | (msg.channel << 24);
	kernelMidi::send(out);
}


/* -------------------------------------------------------------------------- */


void Channel::readPatchMidiIn_DEPR_(int i)
{
	midiIn         = G_Patch_DEPR_.getMidiValue(i, "In");
	midiInKeyPress = G_Patch_DEPR_.getMidiValue(i, "InKeyPress");
	midiInKeyRel   = G_Patch_DEPR_.getMidiValue(i, "InKeyRel");
  midiInKill     = G_Patch_DEPR_.getMidiValue(i, "InKill");
  midiInVolume   = G_Patch_DEPR_.getMidiValue(i, "InVolume");
  midiInMute     = G_Patch_DEPR_.getMidiValue(i, "InMute");
  midiInSolo     = G_Patch_DEPR_.getMidiValue(i, "InSolo");
}

void Channel::readPatchMidiOut_DEPR_(int i)
{
	midiOutL        = G_Patch_DEPR_.getMidiValue(i, "OutL");
	midiOutLplaying = G_Patch_DEPR_.getMidiValue(i, "OutLplaying");
	midiOutLmute    = G_Patch_DEPR_.getMidiValue(i, "OutLmute");
	midiOutLsolo    = G_Patch_DEPR_.getMidiValue(i, "OutLsolo");
}


/* -------------------------------------------------------------------------- */


bool Channel::isPlaying()
{
	return status & (STATUS_PLAY | STATUS_ENDING);
}


/* -------------------------------------------------------------------------- */


int Channel::writePatch(int i, bool isProject)
{
	Patch::channel_t pch;
	pch.type            = type;
	pch.key             = key;
	pch.index           = index;
	pch.column          = guiChannel->getColumnIndex();
	pch.mute            = mute;
	pch.mute_s          = mute_s;
	pch.solo            = solo;
	pch.volume          = volume;
	pch.panLeft         = panLeft;
	pch.panRight        = panRight;
	pch.midiIn          = midiIn;
	pch.midiInKeyPress  = midiInKeyPress;
	pch.midiInKeyRel    = midiInKeyRel;
	pch.midiInKill      = midiInKill;
	pch.midiInVolume    = midiInVolume;
	pch.midiInMute      = midiInMute;
	pch.midiInSolo      = midiInSolo;
	pch.midiOutL        = midiOutL;
	pch.midiOutLplaying = midiOutLplaying;
	pch.midiOutLmute    = midiOutLmute;
	pch.midiOutLsolo    = midiOutLsolo;

	for (unsigned i=0; i<recorder::global.size(); i++) {
		for (unsigned k=0; k<recorder::global.at(i).size(); k++) {
			recorder::action *action = recorder::global.at(i).at(k);
			if (action->chan == index) {
				Patch::action_t pac;
				pac.type   = action->type;
		    pac.frame  = action->frame;
		    pac.fValue = action->fValue;
		    pac.iValue = action->iValue;
				pch.actions.push_back(pac);
			}
		}
	}

#ifdef WITH_VST

	unsigned numPlugs = G_PluginHost.countPlugins(PluginHost::CHANNEL, this);
	for (unsigned i=0; i<numPlugs; i++) {
		Plugin *pPlugin = G_PluginHost.getPluginByIndex(i, PluginHost::CHANNEL, this);
		if (pPlugin->getStatus()) {
			Patch::plugin_t pp;
			pp.path   = pPlugin->getUniqueId();
	    pp.bypass = pPlugin->isBypassed();
			for (int k=0; k<pPlugin->getNumParameters(); k++)
				pp.params.push_back(pPlugin->getParameter(k));
			pch.plugins.push_back(pp);
		}
	}

#endif

	G_Patch.channels.push_back(pch);

	return G_Patch.channels.size() - 1;
}


/* -------------------------------------------------------------------------- */


int Channel::readPatch(const string &path, int i)
{
	int ret = 1;
	Patch::channel_t *pch = &G_Patch.channels.at(i);
	key             = pch->key;
	type            = pch->type;
	index           = pch->index;
	mute            = pch->mute;
	mute_s          = pch->mute_s;
	solo            = pch->solo;
	volume          = pch->volume;
	panLeft         = pch->panLeft;
	panRight        = pch->panRight;
	midiIn          = pch->midiIn;
	midiInKeyPress  = pch->midiInKeyPress;
	midiInKeyRel    = pch->midiInKeyRel;
  midiInKill      = pch->midiInKill;
  midiInVolume    = pch->midiInVolume;
  midiInMute      = pch->midiInMute;
  midiInSolo      = pch->midiInSolo;
	midiOutL        = pch->midiOutL;
	midiOutLplaying = pch->midiOutLplaying;
	midiOutLmute    = pch->midiOutLmute;
	midiOutLsolo    = pch->midiOutLsolo;

	for (unsigned k=0; k<pch->actions.size(); k++) {
		Patch::action_t *ac = &pch->actions.at(k);
		recorder::rec(index, ac->type, ac->frame, ac->iValue, ac->fValue);
	}

#ifdef WITH_VST

	for (unsigned k=0; k<pch->plugins.size(); k++) {
		Patch::plugin_t *ppl = &pch->plugins.at(k);
		Plugin *plugin = G_PluginHost.addPlugin(ppl->path, PluginHost::CHANNEL,
      &G_Mixer.mutex_plugins, this);
		if (plugin != NULL) {
			plugin->setBypass(ppl->bypass);
			for (unsigned j=0; j<ppl->params.size(); j++)
				plugin->setParameter(j, ppl->params.at(j));
			ret &= 1;
		}
		else
			ret &= 0;
	}

#endif

	return ret;
}


/* -------------------------------------------------------------------------- */


void Channel::sendMidiLmute()
{
	if (!midiOutL || midiOutLmute == 0x0)
		return;
	if (mute)
		sendMidiLmessage(midiOutLsolo, G_MidiMap.muteOn);
	else
		sendMidiLmessage(midiOutLsolo, G_MidiMap.muteOff);
}


/* -------------------------------------------------------------------------- */


void Channel::sendMidiLsolo()
{
	if (!midiOutL || midiOutLsolo == 0x0)
		return;
	if (solo)
		sendMidiLmessage(midiOutLsolo, G_MidiMap.soloOn);
	else
		sendMidiLmessage(midiOutLsolo, G_MidiMap.soloOff);
}


/* -------------------------------------------------------------------------- */


void Channel::sendMidiLplay()
{
	if (!midiOutL || midiOutLplaying == 0x0)
		return;
	switch (status) {
		case STATUS_OFF:
			sendMidiLmessage(midiOutLplaying, G_MidiMap.stopped);
			break;
		case STATUS_PLAY:
			sendMidiLmessage(midiOutLplaying, G_MidiMap.playing);
			break;
		case STATUS_WAIT:
			sendMidiLmessage(midiOutLplaying, G_MidiMap.waiting);
			break;
		case STATUS_ENDING:
			sendMidiLmessage(midiOutLplaying, G_MidiMap.stopping);
	}
}
