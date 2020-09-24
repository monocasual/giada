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
#include "core/channels/channel.h"
#include "core/channels/samplePlayer.h"
#include "core/const.h"
#include "core/kernelAudio.h"
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


std::unique_ptr<Channel> create(ChannelType type, int bufferSize, ID columnId,
	const conf::Conf& conf)
{
	std::unique_ptr<Channel> ch = std::make_unique<Channel>(type, 
		channelId_.get(), columnId, kernelAudio::getRealBufSize(), conf);
	
	return ch;
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Channel> create(const Channel& o)
{
	std::unique_ptr<Channel> ch = std::make_unique<Channel>(o);
	ch->id = channelId_.get();
	return ch;
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Channel> deserializeChannel(const patch::Channel& pch, int bufferSize)
{
	channelId_.set(pch.id);
	return std::make_unique<Channel>(pch, bufferSize);
}


/* -------------------------------------------------------------------------- */


const patch::Channel serializeChannel(const Channel& c)
{
	patch::Channel pc;

#ifdef WITH_VST
	for (ID pid : c.pluginIds)
		pc.pluginIds.push_back(pid);
#endif

	pc.id                = c.id;
	pc.type              = c.getType();
    pc.columnId          = c.getColumnId();
    pc.height            = c.state->height;
    pc.name              = c.state->name;
    pc.key               = c.state->key.load();
    pc.mute              = c.state->mute.load();
    pc.solo              = c.state->solo.load();
    pc.volume            = c.state->volume.load();
    pc.pan               = c.state->pan.load();
    pc.hasActions        = c.state->hasActions;
    pc.readActions       = c.state->readActions.load();
    pc.armed             = c.state->armed.load();
    pc.midiIn            = c.midiLearner.state->enabled.load();
    pc.midiInFilter      = c.midiLearner.state->filter.load();
    pc.midiInKeyPress    = c.midiLearner.state->keyPress.load();
    pc.midiInKeyRel      = c.midiLearner.state->keyRelease.load();
    pc.midiInKill        = c.midiLearner.state->kill.load();
    pc.midiInArm         = c.midiLearner.state->arm.load();
    pc.midiInVolume      = c.midiLearner.state->volume.load();
    pc.midiInMute        = c.midiLearner.state->mute.load();
    pc.midiInSolo        = c.midiLearner.state->solo.load();
	pc.midiInReadActions = c.midiLearner.state->readActions.load();
	pc.midiInPitch       = c.midiLearner.state->pitch.load();
    pc.midiOutL          = c.midiLighter.state->enabled.load(); 
    pc.midiOutLplaying   = c.midiLighter.state->playing.load();
    pc.midiOutLmute      = c.midiLighter.state->mute.load();
    pc.midiOutLsolo      = c.midiLighter.state->solo.load();

	if (c.getType() == ChannelType::SAMPLE) {
		pc.waveId            = c.samplePlayer->getWaveId();
		pc.mode              = c.samplePlayer->state->mode.load();
		pc.begin             = c.samplePlayer->state->begin.load();
		pc.end               = c.samplePlayer->state->end.load();
		pc.pitch             = c.samplePlayer->state->pitch.load();
		pc.shift             = c.samplePlayer->state->shift.load();
		pc.midiInVeloAsVol   = c.samplePlayer->state->velocityAsVol.load();
		pc.inputMonitor      = c.audioReceiver->state->inputMonitor.load();
		pc.overdubProtection = c.audioReceiver->state->overdubProtection.load();

	}
	else
	if (c.getType() == ChannelType::MIDI) { 
		pc.midiOut     = c.midiSender->state->enabled.load();
		pc.midiOutChan = c.midiSender->state->filter.load();
	}

	return pc;
}
}}} // giada::m::channelManager
