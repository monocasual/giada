/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include "utils/fs.h"
#include "core/channels/sampleChannel.h"
#include "core/channels/midiChannel.h"
#include "core/channels/masterChannel.h"
#include "core/channels/channel.h"
#include "core/const.h"
#include "core/patch.h"
#include "core/mixer.h"
#include "core/idManager.h"
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
IdManager channelId_;
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init()
{
	channelId_ = IdManager();
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Channel> create(ChannelType type, int bufferSize,
	bool inputMonitorOn, ID columnId)
{
	std::unique_ptr<Channel> ch = nullptr;

	if (type == ChannelType::SAMPLE)
		ch = std::make_unique<SampleChannel>(inputMonitorOn, bufferSize, columnId, channelId_.get());
	else
	if (type == ChannelType::MIDI)
		ch = std::make_unique<MidiChannel>(bufferSize, columnId, channelId_.get());
	else
	if (type == ChannelType::MASTER)
		ch = std::make_unique<MasterChannel>(bufferSize, channelId_.get());
	else
	if (type == ChannelType::PREVIEW)
		ch = std::make_unique<MasterChannel>(bufferSize, channelId_.get()); // TODO - temporary placeholder
	
	assert(ch != nullptr);
	return ch;
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Channel> create(const Channel& o)
{
	std::unique_ptr<Channel> ch = nullptr;
	
	if (o.type == ChannelType::SAMPLE)
		ch = std::make_unique<SampleChannel>(static_cast<const SampleChannel&>(o));
	else
	if (o.type == ChannelType::MIDI)
		ch = std::make_unique<MidiChannel>(static_cast<const MidiChannel&>(o));
	else
	if (o.type == ChannelType::MASTER)
		ch = std::make_unique<MasterChannel>(static_cast<const MasterChannel&>(o));

	assert(ch != nullptr);

	if (o.type != ChannelType::MASTER)
		ch->id = channelId_.get();

	return ch;
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Channel> deserializeChannel(const patch::Channel& pch, int bufferSize)
{
	std::unique_ptr<Channel> ch = nullptr;

	if (pch.type == ChannelType::SAMPLE)
		ch = std::make_unique<SampleChannel>(pch, bufferSize);
	else
	if (pch.type == ChannelType::MIDI)
		ch = std::make_unique<MidiChannel>(pch, bufferSize);

	assert(ch != nullptr);

	channelId_.set(pch.id);

	return ch;
}


/* -------------------------------------------------------------------------- */


const patch::Channel serializeChannel(const Channel& c)
{
	patch::Channel pc;

	pc.id   = c.id;
	pc.type = c.type;

#ifdef WITH_VST
	for (ID pid : c.pluginIds)
		pc.pluginIds.push_back(pid);
#endif	

	if (c.type != ChannelType::MASTER) {
		pc.size            = 20; // c->size;  // TODO
		pc.name            = c.name.c_str();
		pc.columnId        = c.columnId;
		pc.key             = c.key;
		pc.mute            = c.mute;
		pc.solo            = c.solo;
		pc.volume          = c.volume;
		pc.pan             = c.pan;
		pc.hasActions      = c.hasActions;
		pc.armed           = c.armed;
		pc.midiIn          = c.midiIn;
		pc.midiInKeyPress  = c.midiInKeyRel;
		pc.midiInKeyRel    = c.midiInKeyPress;
		pc.midiInKill      = c.midiInKill;
		pc.midiInArm       = c.midiInArm;
		pc.midiInVolume    = c.midiInVolume;
		pc.midiInMute      = c.midiInMute;
		pc.midiInSolo      = c.midiInSolo;
		pc.midiInFilter    = c.midiInFilter;
		pc.midiOutL        = c.midiOutL;
		pc.midiOutLplaying = c.midiOutLplaying;
		pc.midiOutLmute    = c.midiOutLmute;
		pc.midiOutLsolo    = c.midiOutLsolo;
	}

	if (c.type == ChannelType::SAMPLE) {
		const SampleChannel& sc = static_cast<const SampleChannel&>(c);
		pc.waveId            = sc.waveId;
		pc.mode              = sc.mode;
		pc.begin             = sc.begin;
		pc.end               = sc.end;
		pc.readActions       = sc.readActions;
		pc.pitch             = sc.pitch;
		pc.inputMonitor      = sc.inputMonitor;
		pc.midiInVeloAsVol   = sc.midiInVeloAsVol;
		pc.midiInReadActions = sc.midiInReadActions;
		pc.midiInPitch       = sc.midiInPitch;
	}
	else
	if (c.type == ChannelType::MIDI) {
		const MidiChannel& mc = static_cast<const MidiChannel&>(c);
		pc.midiOut     = mc.midiOut;
		pc.midiOutChan = mc.midiOutChan;
	}

	return pc;
}
}}}; // giada::m::channelManager
