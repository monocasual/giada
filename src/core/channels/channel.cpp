/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/channels/channel.h"
#include "core/actions/actionRecorder.h"
#include "core/conf.h"
#include "core/engine.h"
#include "core/midiMapper.h"
#include "core/model/model.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/recorder.h"
#include <cassert>

extern giada::m::Engine g_engine;

namespace giada::m
{
Channel::Channel(ChannelType type, ID id, ID columnId, int position, ChannelShared& s)
: shared(&s)
, id(id)
, type(type)
, columnId(columnId)
, position(position)
, volume(G_DEFAULT_VOL)
, volume_i(G_DEFAULT_VOL)
, pan(G_DEFAULT_PAN)
, armed(false)
, key(0)
, hasActions(false)
, height(G_GUI_UNIT)
, m_mute(false)
, m_solo(false)
{
	switch (type)
	{
	case ChannelType::SAMPLE:
		sampleReactor.emplace(*shared, id);
		sampleChannel.emplace();
		break;

	case ChannelType::PREVIEW:
		sampleReactor.emplace(*shared, id);
		sampleChannel.emplace();
		break;

	case ChannelType::MIDI:
		midiController.emplace();
		midiSender.emplace(g_engine.getKernelMidi());
		midiActionRecorder.emplace(g_engine.getActionRecorder());
		midiReceiver.emplace();
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

Channel::Channel(const Patch::Channel& p, ChannelShared& s, float samplerateRatio, Wave* wave, std::vector<Plugin*> plugins)
: shared(&s)
, id(p.id)
, type(p.type)
, columnId(p.columnId)
, position(p.position)
, volume(p.volume)
, volume_i(G_DEFAULT_VOL)
, pan(p.pan)
, armed(p.armed)
, key(p.key)
, hasActions(p.hasActions)
, name(p.name)
, height(p.height)
, plugins(plugins)
, midiLearn(p)
, midiLightning(p)
, m_mute(p.mute)
, m_solo(p.solo)
{
	shared->readActions.store(p.readActions);
	shared->recStatus.store(p.readActions ? ChannelStatus::PLAY : ChannelStatus::OFF);

	switch (type)
	{
	case ChannelType::SAMPLE:
		sampleReactor.emplace(*shared, id);
		sampleChannel.emplace(p, wave, samplerateRatio);
		break;

	case ChannelType::PREVIEW:
		sampleReactor.emplace(*shared, id);
		sampleChannel.emplace(p, wave, samplerateRatio);
		break;

	case ChannelType::MIDI:
		midiController.emplace();
		midiSender.emplace(p, g_engine.getKernelMidi());
		midiActionRecorder.emplace(g_engine.getActionRecorder());
		midiReceiver.emplace();
		break;

	default:
		break;
	}

	setWave(wave, samplerateRatio);
}

/* -------------------------------------------------------------------------- */

Channel::Channel(const Channel& other)
{
	*this = other;
}

/* -------------------------------------------------------------------------- */

Channel& Channel::operator=(const Channel& other)
{
	if (this == &other)
		return *this;

	shared     = other.shared;
	id         = other.id;
	type       = other.type;
	columnId   = other.columnId;
	position   = other.position;
	volume     = other.volume;
	volume_i   = other.volume_i;
	pan        = other.pan;
	m_mute     = other.m_mute;
	m_solo     = other.m_solo;
	armed      = other.armed;
	key        = other.key;
	hasActions = other.hasActions;
	name       = other.name;
	height     = other.height;
	plugins    = other.plugins;

	midiLearn          = other.midiLearn;
	sampleReactor      = other.sampleReactor;
	midiController     = other.midiController;
	midiReceiver       = other.midiReceiver;
	midiSender         = other.midiSender;
	midiActionRecorder = other.midiActionRecorder;
	sampleChannel      = other.sampleChannel;

	return *this;
}

/* -------------------------------------------------------------------------- */

bool Channel::operator==(const Channel& other)
{
	return id == other.id;
}

/* -------------------------------------------------------------------------- */

bool Channel::isInternal() const
{
	return type == ChannelType::MASTER || type == ChannelType::PREVIEW;
}

bool Channel::isMuted() const
{
	/* Internals can't be muted. */
	return !isInternal() && m_mute;
}

bool Channel::isSoloed() const
{
	return m_solo;
}

bool Channel::isAudible(bool mixerHasSolos) const
{
	if (isInternal())
		return true;
	if (isMuted())
		return false;
	return !mixerHasSolos || (mixerHasSolos && isSoloed());
}

bool Channel::canInputRec() const
{
	if (type != ChannelType::SAMPLE)
		return false;

	bool hasWave     = sampleChannel->hasWave();
	bool isProtected = sampleChannel->overdubProtection;
	bool canOverdub  = !hasWave || (hasWave && !isProtected);

	return armed && canOverdub;
}

bool Channel::canActionRec() const
{
	return hasWave() && !sampleChannel->isAnyLoopMode();
}

bool Channel::hasWave() const
{
	return sampleChannel && sampleChannel->hasWave();
}

bool Channel::isPlaying() const
{
	ChannelStatus s = shared->playStatus.load();
	return s == ChannelStatus::PLAY || s == ChannelStatus::ENDING;
}

/* -------------------------------------------------------------------------- */

void Channel::setMute(bool v)
{
	m_mute = v;
}

void Channel::setSolo(bool v)
{
	m_solo = v;
}

/* -------------------------------------------------------------------------- */

void Channel::loadWave(Wave* w, Frame newBegin, Frame newEnd, Frame newShift)
{
	shared->tracker.store(0);
	shared->playStatus.store(w != nullptr ? ChannelStatus::OFF : ChannelStatus::EMPTY);

	sampleChannel->loadWave(w, newBegin, newEnd, newShift);
}

/* -------------------------------------------------------------------------- */

void Channel::setWave(Wave* w, float samplerateRatio)
{
	sampleChannel->setWave(w, samplerateRatio);
}

/* -------------------------------------------------------------------------- */

void Channel::kickIn(Frame f)
{
	shared->tracker.store(f);
	shared->playStatus.store(ChannelStatus::PLAY);
}
} // namespace giada::m