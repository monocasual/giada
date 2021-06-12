/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "channelManager.h"
#include "core/action.h"
#include "core/channels/channel.h"
#include "core/channels/samplePlayer.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/idManager.h"
#include "core/kernelAudio.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/recorderHandler.h"
#include "core/wave.h"
#include "core/waveManager.h"
#include "utils/fs.h"
#include <cassert>

namespace giada::m::channelManager
{
namespace
{
IdManager channelId_;

/* -------------------------------------------------------------------------- */

channel::State& makeState_(ChannelType type)
{
	std::unique_ptr<channel::State> state = std::make_unique<channel::State>();

	if (type == ChannelType::SAMPLE || type == ChannelType::PREVIEW)
		state->resampler = Resampler(static_cast<Resampler::Quality>(conf::conf.rsmpQuality), G_MAX_IO_CHANS);

	model::add(std::move(state));
	return model::back<channel::State>();
}

/* -------------------------------------------------------------------------- */

channel::Buffer& makeBuffer_()
{
	model::add(std::make_unique<channel::Buffer>(kernelAudio::getRealBufSize()));
	return model::back<channel::Buffer>();
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void init()
{
	channelId_ = IdManager();
}

/* -------------------------------------------------------------------------- */

channel::Data create(ID channelId, ChannelType type, ID columnId)
{
	channel::Data out = channel::Data(type, channelId_.generate(channelId),
	    columnId, makeState_(type), makeBuffer_());

	if (out.audioReceiver)
		out.audioReceiver->overdubProtection = conf::conf.overdubProtectionDefaultOn;

	return out;
}

/* -------------------------------------------------------------------------- */

channel::Data create(const channel::Data& o)
{
	channel::Data out = channel::Data(o);

	out.id     = channelId_.generate();
	out.state  = &makeState_(o.type);
	out.buffer = &makeBuffer_();

	return out;
}

/* -------------------------------------------------------------------------- */

channel::Data deserializeChannel(const patch::Channel& pch, float samplerateRatio)
{
	channelId_.set(pch.id);
	return channel::Data(pch, makeState_(pch.type), makeBuffer_(), samplerateRatio);
}

/* -------------------------------------------------------------------------- */

const patch::Channel serializeChannel(const channel::Data& c)
{
	patch::Channel pc;

#ifdef WITH_VST
	for (const Plugin* p : c.plugins)
		pc.pluginIds.push_back(p->id);
#endif

	pc.id                = c.id;
	pc.type              = c.type;
	pc.columnId          = c.columnId;
	pc.height            = c.height;
	pc.name              = c.name;
	pc.key               = c.key;
	pc.mute              = c.mute;
	pc.solo              = c.solo;
	pc.volume            = c.volume;
	pc.pan               = c.pan;
	pc.hasActions        = c.hasActions;
	pc.readActions       = c.state->readActions.load();
	pc.armed             = c.armed;
	pc.midiIn            = c.midiLearner.enabled;
	pc.midiInFilter      = c.midiLearner.filter;
	pc.midiInKeyPress    = c.midiLearner.keyPress.getValue();
	pc.midiInKeyRel      = c.midiLearner.keyRelease.getValue();
	pc.midiInKill        = c.midiLearner.kill.getValue();
	pc.midiInArm         = c.midiLearner.arm.getValue();
	pc.midiInVolume      = c.midiLearner.volume.getValue();
	pc.midiInMute        = c.midiLearner.mute.getValue();
	pc.midiInSolo        = c.midiLearner.solo.getValue();
	pc.midiInReadActions = c.midiLearner.readActions.getValue();
	pc.midiInPitch       = c.midiLearner.pitch.getValue();
	pc.midiOutL          = c.midiLighter.enabled;
	pc.midiOutLplaying   = c.midiLighter.playing.getValue();
	pc.midiOutLmute      = c.midiLighter.mute.getValue();
	pc.midiOutLsolo      = c.midiLighter.solo.getValue();

	if (c.type == ChannelType::SAMPLE)
	{
		pc.waveId            = c.samplePlayer->getWaveId();
		pc.mode              = c.samplePlayer->mode;
		pc.begin             = c.samplePlayer->begin;
		pc.end               = c.samplePlayer->end;
		pc.pitch             = c.samplePlayer->pitch;
		pc.shift             = c.samplePlayer->shift;
		pc.midiInVeloAsVol   = c.samplePlayer->velocityAsVol;
		pc.inputMonitor      = c.audioReceiver->inputMonitor;
		pc.overdubProtection = c.audioReceiver->overdubProtection;
	}
	else if (c.type == ChannelType::MIDI)
	{
		pc.midiOut     = c.midiSender->enabled;
		pc.midiOutChan = c.midiSender->filter;
	}

	return pc;
}
} // namespace giada::m::channelManager
