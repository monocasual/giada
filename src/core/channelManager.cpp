/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../utils/fs.h"
#include "const.h"
#include "channel.h"
#include "patch.h"
#include "mixer.h"
#include "wave.h"
#include "waveManager.h"
#include "sampleChannel.h"
#include "midiChannel.h"
#include "pluginHost.h"
#include "plugin.h"
#include "channelManager.h"


using std::string;


namespace giada {
namespace m {
namespace channelManager
{
namespace
{
void writeActions_(int chanIndex, patch::channel_t& pch)
{
	recorder::forEachAction([&] (const recorder::action* a) {
		if (a->chan != chanIndex) 
			return;
		pch.actions.push_back(patch::action_t { 
			a->type, a->frame, a->fValue, a->iValue 
		});
	});
}


/* -------------------------------------------------------------------------- */


void writePlugins_(const Channel* ch, patch::channel_t& pch)
{
#ifdef WITH_VST

	pluginHost::forEachPlugin(pluginHost::CHANNEL, ch, [&] (const Plugin* p) {
		patch::plugin_t pp;
		pp.path   = p->getUniqueId();
		pp.bypass = p->isBypassed();
		for (int k=0; k<p->getNumParameters(); k++)
			pp.params.push_back(p->getParameter(k));
		for (uint32_t param : p->midiInParams)
			pp.midiInParams.push_back(param);
		pch.plugins.push_back(pp);
	});

#endif
}
} // {anonymous}


/* -------------------------------------------------------------------------- */


void readActions_(Channel* ch, const patch::channel_t& pch)
{
	for (const patch::action_t& ac : pch.actions) {
		recorder::rec(ch->index, ac.type, ac.frame, ac.iValue, ac.fValue);
		ch->hasActions = true;
	}
}


/* -------------------------------------------------------------------------- */


void readPlugins_(Channel* ch, const patch::channel_t& pch)
{
#ifdef WITH_VST

	for (const patch::plugin_t& ppl : pch.plugins) {
		Plugin* plugin = pluginHost::addPlugin(ppl.path, pluginHost::CHANNEL,
			&mixer::mutex, ch);
		if (plugin == nullptr)
			continue;

		plugin->setBypass(ppl.bypass);
		for (unsigned j=0; j<ppl.params.size(); j++)
			plugin->setParameter(j, ppl.params.at(j));

		/* Don't fill Channel::midiInParam if Patch::midiInParams are 0: it would
		wipe out the current default 0x0 values. */

		if (!ppl.midiInParams.empty()) {
			plugin->midiInParams.clear();
			for (uint32_t midiInParam : ppl.midiInParams)
				plugin->midiInParams.push_back(midiInParam);
		}
	}

#endif
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


int create(ChannelType type, int bufferSize, bool inputMonitorOn, Channel** out)
{
	if (type == ChannelType::SAMPLE)
		*out = new SampleChannel(inputMonitorOn, bufferSize);
	else
		*out = new MidiChannel(bufferSize);
	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


int writePatch(const Channel* ch, bool isProject)
{
	patch::channel_t pch;
	pch.type            = static_cast<int>(ch->type);
	pch.index           = ch->index;
	pch.size            = ch->guiChannel->getSize();
	pch.name            = ch->name;
	pch.key             = ch->key;
	pch.armed           = ch->armed;
	pch.column          = ch->guiChannel->getColumnIndex();
	pch.mute            = ch->mute;
	pch.solo            = ch->solo;
	pch.volume          = ch->volume;
	pch.pan             = ch->pan;
	pch.midiIn          = ch->midiIn;
	pch.midiInKeyPress  = ch->midiInKeyPress;
	pch.midiInKeyRel    = ch->midiInKeyRel;
	pch.midiInKill      = ch->midiInKill;
	pch.midiInArm       = ch->midiInArm;
	pch.midiInVolume    = ch->midiInVolume;
	pch.midiInMute      = ch->midiInMute;
	pch.midiInFilter    = ch->midiInFilter;
	pch.midiInSolo      = ch->midiInSolo;
	pch.midiOutL        = ch->midiOutL;
	pch.midiOutLplaying = ch->midiOutLplaying;
	pch.midiOutLmute    = ch->midiOutLmute;
	pch.midiOutLsolo    = ch->midiOutLsolo;

	writeActions_(ch->index, pch);
	writePlugins_(ch, pch);

	patch::channels.push_back(pch);

	return patch::channels.size() - 1;
}


/* -------------------------------------------------------------------------- */


void writePatch(const MidiChannel* ch, bool isProject, int index)
{
	patch::channel_t& pch = patch::channels.at(index);
	pch.midiOut     = ch->midiOut;
	pch.midiOutChan = ch->midiOutChan;
}


/* -------------------------------------------------------------------------- */


void writePatch(const SampleChannel* ch, bool isProject, int index)
{
	patch::channel_t& pch = patch::channels.at(index);

	if (ch->wave != nullptr) {
		pch.samplePath = ch->wave->getPath();
		if (isProject)
			pch.samplePath = gu_basename(ch->wave->getPath());  // make it portable
	}
	else
		pch.samplePath = "";

	pch.mode              = static_cast<int>(ch->mode);
	pch.begin             = ch->getBegin();
	pch.end               = ch->getEnd();
	pch.boost             = ch->getBoost();
	pch.recActive         = ch->readActions;
	pch.pitch             = ch->getPitch();
	pch.inputMonitor      = ch->inputMonitor;
	pch.midiInReadActions = ch->midiInReadActions;
	pch.midiInPitch       = ch->midiInPitch;	
}


/* -------------------------------------------------------------------------- */


void readPatch(Channel* ch, int i)
{
	const patch::channel_t& pch = patch::channels.at(i);

	ch->key             = pch.key;
	ch->armed           = pch.armed;
	ch->type            = static_cast<ChannelType>(pch.type);
	ch->name            = pch.name;
	ch->index           = pch.index;
	ch->mute            = pch.mute;
	ch->solo            = pch.solo;
	ch->volume          = pch.volume;
	ch->pan             = pch.pan;
	ch->midiIn          = pch.midiIn;
	ch->midiInKeyPress  = pch.midiInKeyPress;
	ch->midiInKeyRel    = pch.midiInKeyRel;
	ch->midiInKill      = pch.midiInKill;
	ch->midiInVolume    = pch.midiInVolume;
	ch->midiInMute      = pch.midiInMute;
	ch->midiInFilter    = pch.midiInFilter;
	ch->midiInSolo      = pch.midiInSolo;
	ch->midiOutL        = pch.midiOutL;
	ch->midiOutLplaying = pch.midiOutLplaying;
	ch->midiOutLmute    = pch.midiOutLmute;
	ch->midiOutLsolo    = pch.midiOutLsolo;

	readActions_(ch, pch);
	readPlugins_(ch, pch);
}


/* -------------------------------------------------------------------------- */


void readPatch(SampleChannel* ch, const string& basePath, int i)
{
	const patch::channel_t& pch = patch::channels.at(i);

	ch->mode              = static_cast<ChannelMode>(pch.mode);
	ch->readActions       = pch.recActive;
	ch->recStatus         = pch.recActive ? ChannelStatus::PLAY : ChannelStatus::OFF;
	ch->midiInVeloAsVol   = pch.midiInVeloAsVol;
	ch->midiInReadActions = pch.midiInReadActions;
	ch->midiInPitch       = pch.midiInPitch;
  ch->inputMonitor      = pch.inputMonitor;
	ch->setBoost(pch.boost);

  Wave* w = nullptr;
  int res = waveManager::create(basePath + pch.samplePath, &w); 

	if (res == G_RES_OK) {
		ch->pushWave(w);
		ch->setBegin(pch.begin);
		ch->setEnd(pch.end);
		ch->setPitch(pch.pitch);
	}
	else {
		if (res == G_RES_ERR_NO_DATA)
			ch->status = ChannelStatus::EMPTY;
		else
		if (res == G_RES_ERR_IO)
			ch->status = ChannelStatus::MISSING;
		ch->sendMidiLstatus();  // FIXME - why sending MIDI lightning if sample status is wrong?
	}
}


/* -------------------------------------------------------------------------- */


void readPatch(MidiChannel* ch, int i)
{
	const patch::channel_t& pch = patch::channels.at(i);

	ch->midiOut     = pch.midiOut;
	ch->midiOutChan = pch.midiOutChan;	
}
}}}; // giada::m::channelManager
