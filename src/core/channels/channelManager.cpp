/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/channels/channelManager.h"
#include "core/channels/channel.h"
#include "core/channels/samplePlayer.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"
#include "core/wave.h"
#include <cassert>

namespace giada::m
{
ChannelManager::ChannelManager(const Conf::Data& c, model::Model& m)
: m_conf(c)
, m_model(m)
{
}

/* -------------------------------------------------------------------------- */

ID ChannelManager::getNextId() const
{
	return m_channelId.getNext();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::reset()
{
	m_channelId = IdManager();
}

/* -------------------------------------------------------------------------- */

Channel ChannelManager::create(ID channelId, ChannelType type, ID columnId, int bufferSize)
{
	Channel out = Channel(type, m_channelId.generate(channelId), columnId,
	    makeShared(type, bufferSize));

	if (out.audioReceiver)
		out.audioReceiver->overdubProtection = m_conf.overdubProtectionDefaultOn;

	return out;
}

/* -------------------------------------------------------------------------- */

Channel ChannelManager::create(const Channel& o, int bufferSize)
{
	Channel out = Channel(o);

	out.id     = m_channelId.generate();
	out.shared = &makeShared(o.type, bufferSize);

	return out;
}

/* -------------------------------------------------------------------------- */

Channel ChannelManager::deserializeChannel(const Patch::Channel& pch, float samplerateRatio, int bufferSize)
{
	m_channelId.set(pch.id);
	return Channel(pch, makeShared(pch.type, bufferSize), samplerateRatio, m_model.findShared<Wave>(pch.waveId));
}

/* -------------------------------------------------------------------------- */

const Patch::Channel ChannelManager::serializeChannel(const Channel& c)
{
	Patch::Channel pc;

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
	pc.mute              = c.isMuted();
	pc.solo              = c.isSoloed();
	pc.volume            = c.volume;
	pc.pan               = c.pan;
	pc.hasActions        = c.hasActions;
	pc.readActions       = c.shared->readActions.load();
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

/* -------------------------------------------------------------------------- */

ChannelShared& ChannelManager::makeShared(ChannelType type, int bufferSize)
{
	std::unique_ptr<ChannelShared> shared = std::make_unique<ChannelShared>(bufferSize);

	if (type == ChannelType::SAMPLE || type == ChannelType::PREVIEW)
	{
		shared->quantizer.emplace();
		shared->renderQueue.emplace();
		shared->resampler.emplace(static_cast<Resampler::Quality>(m_conf.rsmpQuality), G_MAX_IO_CHANS);
	}

	m_model.addShared(std::move(shared));
	return m_model.backShared<ChannelShared>();
}
} // namespace giada::m
