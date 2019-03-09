/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include "utils/fs.h"
#include "core/channels/sampleChannel.h"
#include "core/channels/midiChannel.h"
#include "core/channels/masterChannel.h"
#include "core/channels/channel.h"
#include "core/const.h"
#include "core/patch.h"
#include "core/mixer.h"
#include "core/wave.h"
#include "core/waveManager.h"
#include "core/pluginHost.h"
#include "core/pluginManager.h"
#include "core/plugin.h"
#include "core/action.h"
#include "core/recorderHandler.h"
#include "channelManager.h"


namespace giada {
namespace m {
namespace channelManager
{
namespace
{
void writePlugins_(const Channel* ch, patch::channel_t& pch)
{
#ifdef WITH_VST
#if 0
	pluginHost::forEachPlugin({pluginHost::StackType::CHANNEL, ch->index}, [&] (const Plugin* p) {
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
#endif
}
} // {anonymous}


/* -------------------------------------------------------------------------- */


void readActions_(Channel* ch, const patch::channel_t& pch)
{
	recorderHandler::readPatch(pch.actions);
	ch->hasActions = pch.actions.size() > 0;
}


/* -------------------------------------------------------------------------- */


void readPlugins_(Channel* ch, const patch::channel_t& pch)
{
#if 0
#ifdef WITH_VST

	for (const patch::plugin_t& ppl : pch.plugins) {

		std::unique_ptr<Plugin> plugin = pluginManager::makePlugin(ppl.path);
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

		pluginHost::addPlugin(std::move(plugin), {pluginHost::StackType::CHANNEL, ch->index});
	}
#endif
#endif
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


std::unique_ptr<Channel> create(ChannelType type, int bufferSize, bool inputMonitorOn, size_t column)
{
	if (type == ChannelType::SAMPLE)
		return std::make_unique<SampleChannel>(inputMonitorOn, bufferSize, column);
	else
	if (type == ChannelType::MIDI)
		return std::make_unique<MidiChannel>(bufferSize, column);
	else
		return std::make_unique<MasterChannel>(bufferSize, column);
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Channel> create(const Channel& ch)
{
	if (ch.type == ChannelType::SAMPLE)
		return std::make_unique<SampleChannel>(static_cast<const SampleChannel&>(ch));
	else
	if (ch.type == ChannelType::MIDI)
		return std::make_unique<MidiChannel>(static_cast<const MidiChannel&>(ch));
	else
		return std::make_unique<MasterChannel>(static_cast<const MasterChannel&>(ch));
}


/* -------------------------------------------------------------------------- */


int writePatch(const Channel* ch, bool isProject)
{
#if 0
	patch::channel_t pch;
	pch.type            = static_cast<int>(ch->type);
	pch.index           = ch->index;
	//pch.size            = ch->guiChannel->getSize();
	pch.name            = ch->name;
	pch.key             = ch->key;
	pch.armed           = ch->armed;
	pch.column          = ch->columnIndex;
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

	recorderHandler::writePatch(ch->index, pch.actions);
	writePlugins_(ch, pch);

	patch::channels.push_back(pch);

	return patch::channels.size() - 1;
#endif
}


/* -------------------------------------------------------------------------- */


void writePatch(const MidiChannel* ch, bool isProject, int index)
{
#if 0
	patch::channel_t& pch = patch::channels.at(index);
	pch.midiOut     = ch->midiOut;
	pch.midiOutChan = ch->midiOutChan;
#endif
}


/* -------------------------------------------------------------------------- */


void writePatch(const SampleChannel* ch, bool isProject, int index)
{
#if 0
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
	pch.readActions       = ch->readActions;
	pch.pitch             = ch->getPitch();
	pch.inputMonitor      = ch->inputMonitor;
	pch.midiInReadActions = ch->midiInReadActions;
	pch.midiInPitch       = ch->midiInPitch;	
#endif
}


/* -------------------------------------------------------------------------- */


void readPatch(Channel* ch, const patch::channel_t& pch)
{
#if 0
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
#endif
}


/* -------------------------------------------------------------------------- */


void readPatch(SampleChannel* ch, const std::string& basePath, const patch::channel_t& pch)
{
#if 0
	ch->mode              = static_cast<ChannelMode>(pch.mode);
	ch->readActions       = pch.readActions;
	ch->recStatus         = pch.readActions ? ChannelStatus::PLAY : ChannelStatus::OFF;
	ch->midiInVeloAsVol   = pch.midiInVeloAsVol;
	ch->midiInReadActions = pch.midiInReadActions;
	ch->midiInPitch       = pch.midiInPitch;
	ch->inputMonitor      = pch.inputMonitor;
	ch->setBoost(pch.boost);

	waveManager::Result res = waveManager::createFromFile(basePath + pch.samplePath);

	if (res.status == G_RES_OK) {
		ch->pushWave(std::move(res.wave));
		ch->setBegin(pch.begin);
		ch->setEnd(pch.end);
		ch->setPitch(pch.pitch);
	}
	else {
		if (res.status == G_RES_ERR_NO_DATA)
			ch->status = ChannelStatus::EMPTY;
		else
		if (res.status == G_RES_ERR_IO)
			ch->status = ChannelStatus::MISSING;
		ch->sendMidiLstatus();  // FIXME - why sending MIDI lightning if sample status is wrong?
	}
#endif
}


/* -------------------------------------------------------------------------- */


void readPatch(MidiChannel* ch, const patch::channel_t& pch)
{
#if 0
	ch->midiOut     = pch.midiOut;
	ch->midiOutChan = pch.midiOutChan;	
#endif
}
}}}; // giada::m::channelManager
