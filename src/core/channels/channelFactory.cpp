/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/channels/channelFactory.h"
#include "src/core/channels/channel.h"
#include "src/core/conf.h"
#include "src/core/model/model.h"
#include "src/core/patch.h"
#include "src/core/plugins/plugin.h"
#include "src/core/plugins/pluginHost.h"
#include "src/core/wave.h"
#include <cassert>
#include <memory>

namespace giada::m::channelFactory
{
namespace
{
IdManager channelId_;

/* -------------------------------------------------------------------------- */

std::unique_ptr<ChannelShared> makeShared_(ChannelType type, ID channelId, int bufferSize, Resampler::Quality quality)
{
	std::unique_ptr<ChannelShared> shared = std::make_unique<ChannelShared>(channelId, bufferSize);

	if (type == ChannelType::SAMPLE || type == ChannelType::PREVIEW)
	{
		shared->quantizer.emplace();
		shared->renderQueue.emplace(/*size=*/2, 0, /*num_threads=*/2);
		shared->resampler.emplace(quality, G_MAX_IO_CHANS);
	}

	return shared;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ID getNextId()
{
	return channelId_.getNext();
}

/* -------------------------------------------------------------------------- */

void reset()
{
	channelId_ = IdManager();
}

/* -------------------------------------------------------------------------- */

Data create(ID channelId, ChannelType type, int bufferSize, Resampler::Quality quality, bool overdubProtection)
{
	channelId = channelId_.generate(channelId);

	std::unique_ptr<ChannelShared> shared = makeShared_(type, channelId, bufferSize, quality);
	Channel                        ch     = Channel(type, channelId, *shared.get());

	if (ch.sampleChannel)
		ch.sampleChannel->overdubProtection = overdubProtection;

	return {ch, std::move(shared)};
}

/* -------------------------------------------------------------------------- */

Data create(const Channel& o, int bufferSize, Resampler::Quality quality)
{
	std::unique_ptr<ChannelShared> shared = makeShared_(o.type, o.id, bufferSize, quality);
	Channel                        ch     = Channel(o);

	ch.id     = channelId_.generate();
	ch.shared = shared.get();

	return {ch, std::move(shared)};
}

/* -------------------------------------------------------------------------- */

Channel deserializeChannel(const Patch::Channel& pch, ChannelShared& shared, float samplerateRatio, Wave* wave, std::vector<Plugin*> plugins)
{
	channelId_.set(pch.id);
	return Channel(pch, shared, samplerateRatio, wave, plugins);
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<ChannelShared> deserializeShared(const Patch::Channel& pch, int bufferSize, Resampler::Quality quality)
{
	return makeShared_(pch.type, pch.id, bufferSize, quality);
}

/* -------------------------------------------------------------------------- */

const Patch::Channel serializeChannel(const Channel& c)
{
	Patch::Channel pc;

	for (const Plugin* p : c.plugins)
		pc.pluginIds.push_back(p->id);

	pc.id                = c.id;
	pc.type              = c.type;
	pc.height            = c.height;
	pc.name              = c.getName();
	pc.key               = c.key;
	pc.mute              = c.isMuted();
	pc.solo              = c.isSoloed();
	pc.volume            = c.volume;
	pc.pan               = c.pan.asFloat();
	pc.hasActions        = c.hasActions;
	pc.readActions       = c.shared->readActions.load();
	pc.armed             = c.armed;
	pc.sendToMaster      = c.sendToMaster;
	pc.extraOutputs      = c.extraOutputs;
	pc.midiIn            = c.midiInput.enabled;
	pc.midiInFilter      = c.midiInput.filter;
	pc.midiInKeyPress    = c.midiInput.keyPress.getValue();
	pc.midiInKeyRel      = c.midiInput.keyRelease.getValue();
	pc.midiInKill        = c.midiInput.kill.getValue();
	pc.midiInArm         = c.midiInput.arm.getValue();
	pc.midiInVolume      = c.midiInput.volume.getValue();
	pc.midiInMute        = c.midiInput.mute.getValue();
	pc.midiInSolo        = c.midiInput.solo.getValue();
	pc.midiInReadActions = c.midiInput.readActions.getValue();
	pc.midiInPitch       = c.midiInput.pitch.getValue();
	pc.midiOutL          = c.midiLightning.enabled;
	pc.midiOutLplaying   = c.midiLightning.playing.getValue();
	pc.midiOutLmute      = c.midiLightning.mute.getValue();
	pc.midiOutLsolo      = c.midiLightning.solo.getValue();

	if (c.type == ChannelType::SAMPLE)
	{
		pc.waveId            = c.sampleChannel->getWaveId(0);
		pc.mode              = c.sampleChannel->mode;
		pc.range             = c.sampleChannel->getRange(0);
		pc.pitch             = c.sampleChannel->pitch;
		pc.shift             = c.sampleChannel->shift;
		pc.midiInVeloAsVol   = c.sampleChannel->velocityAsVol;
		pc.inputMonitor      = c.sampleChannel->inputMonitor;
		pc.overdubProtection = c.sampleChannel->overdubProtection;
	}
	else if (c.type == ChannelType::MIDI)
	{
		pc.midiOut     = c.midiChannel->outputEnabled;
		pc.midiOutChan = c.midiChannel->outputFilter;
	}

	return pc;
}
} // namespace giada::m::channelFactory
