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
#include "core/channels/channelFactory.h"
#include "core/model/model.h"
#include "core/waveFactory.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "utils/log.h"

namespace giada::m
{
ChannelManager::ChannelManager(model::Model& model, ChannelFactory& cm, WaveFactory& wm)
: m_model(model)
, m_channelFactory(cm)
, m_waveManager(wm)
, m_hasInputRecordableChannels(false)
{
}

/* -------------------------------------------------------------------------- */

void ChannelManager::reset(Frame framesInBuffer)
{
	m_model.get().channels.clear();

	m_model.get().channels.push_back(m_channelFactory.create(
	    Mixer::MASTER_OUT_CHANNEL_ID, ChannelType::MASTER, /*columnId=*/0, /*position=*/0, framesInBuffer));
	m_model.get().channels.push_back(m_channelFactory.create(
	    Mixer::MASTER_IN_CHANNEL_ID, ChannelType::MASTER, /*columnId=*/0, /*position=*/0, framesInBuffer));
	m_model.get().channels.push_back(m_channelFactory.create(
	    Mixer::PREVIEW_CHANNEL_ID, ChannelType::PREVIEW, /*columnId=*/0, /*position=*/0, framesInBuffer));

	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

Channel& ChannelManager::addChannel(ChannelType type, ID columnId, int position, int bufferSize)
{
	m_model.get().channels.push_back(m_channelFactory.create(/*id=*/0, type, columnId, position, bufferSize));
	m_model.swap(model::SwapType::HARD);

	return m_model.get().channels.back();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::loadSampleChannel(ID channelId, std::unique_ptr<Wave> w)
{
	m_model.addShared(std::move(w));

	Channel& channel = m_model.get().getChannel(channelId);
	Wave&    newWave = m_model.backShared<Wave>();
	Wave*    oldWave = channel.samplePlayer->getWave();

	loadSampleChannel(channel, &newWave);
	m_model.swap(model::SwapType::HARD);

	/* Remove old wave, if any. It is safe to do it now: the audio thread is
	already processing the new layout. */

	if (oldWave != nullptr)
		m_model.removeShared<Wave>(*oldWave);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::addAndLoadSampleChannel(int bufferSize, std::unique_ptr<Wave> w, ID columnId, int position)
{
	m_model.addShared(std::move(w));

	Wave&    wave    = m_model.backShared<Wave>();
	Channel& channel = addChannel(ChannelType::SAMPLE, columnId, position, bufferSize);

	loadSampleChannel(channel, &wave);
	m_model.swap(model::SwapType::HARD);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::cloneChannel(ID channelId, int bufferSize, const std::vector<Plugin*>& plugins)
{
	const Channel& oldChannel = m_model.get().getChannel(channelId);
	Channel        newChannel = m_channelFactory.create(oldChannel, bufferSize);

	/* Clone Wave first, if any. */

	if (oldChannel.samplePlayer && oldChannel.samplePlayer->hasWave())
	{
		const Wave& oldWave  = *oldChannel.samplePlayer->getWave();
		const Frame oldShift = oldChannel.samplePlayer->shift;
		const Frame oldBegin = oldChannel.samplePlayer->begin;
		const Frame oldEnd   = oldChannel.samplePlayer->end;
		m_model.addShared(m_waveManager.createFromWave(oldWave));
		loadSampleChannel(newChannel, &m_model.backShared<Wave>(), oldBegin, oldEnd, oldShift);
	}

	newChannel.plugins = plugins;

	/* Then push the new channel in the channels vector. */

	m_model.get().channels.push_back(newChannel);
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::freeSampleChannel(ID channelId)
{
	Channel& ch = m_model.get().getChannel(channelId);

	assert(ch.samplePlayer);

	const Wave* wave = ch.samplePlayer->getWave();

	loadSampleChannel(ch, nullptr);
	m_model.swap(model::SwapType::HARD);

	if (wave != nullptr)
		m_model.removeShared<Wave>(*wave);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::freeAllSampleChannels()
{
	for (Channel& ch : m_model.get().channels)
		if (ch.samplePlayer)
			loadSampleChannel(ch, nullptr);

	m_model.swap(model::SwapType::HARD);
	m_model.clearShared<model::WavePtrs>();

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::deleteChannel(ID channelId)
{
	const Channel& ch   = m_model.get().getChannel(channelId);
	const Wave*    wave = ch.samplePlayer ? ch.samplePlayer->getWave() : nullptr;

	u::vector::removeIf(m_model.get().channels, [channelId](const Channel& c) {
		return c.id == channelId;
	});
	m_model.swap(model::SwapType::HARD);

	if (wave != nullptr)
		m_model.removeShared<Wave>(*wave);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::renameChannel(ID channelId, const std::string& name)
{
	m_model.get().getChannel(channelId).name = name;
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::moveChannel(ID channelId, ID newColumnId, int newPosition)
{
	/* Make room in the destination column for the new channel. */

	for (Channel& ch : m_model.get().channels)
		if (ch.columnId == newColumnId && ch.position >= newPosition)
			ch.position++;

	Channel& channel = m_model.get().getChannel(channelId);
	channel.columnId = newColumnId;
	channel.position = newPosition;

	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

float ChannelManager::getMasterInVol() const
{
	return m_model.get().getChannel(Mixer::MASTER_IN_CHANNEL_ID).volume;
}

float ChannelManager::getMasterOutVol() const
{
	return m_model.get().getChannel(Mixer::MASTER_OUT_CHANNEL_ID).volume;
}

/* -------------------------------------------------------------------------- */

int ChannelManager::getLastChannelPosition(ID columnId) const
{
	std::vector<const Channel*> column = getColumn(columnId);
	return column.empty() ? 0 : column.back()->position + 1;
}
/* -------------------------------------------------------------------------- */

std::vector<const Channel*> ChannelManager::getColumn(ID columnId) const
{
	std::vector<const Channel*> column;

	for (const Channel& ch : m_model.get().channels)
		if (ch.columnId == columnId)
			column.push_back(&ch);

	return column;
}

/* -------------------------------------------------------------------------- */

void ChannelManager::finalizeInputRec(const mcl::AudioBuffer& buffer, Frame recordedFrames, Frame currentFrame)
{
	for (Channel* ch : getRecordableChannels())
		recordChannel(*ch, buffer, recordedFrames, currentFrame);
	for (Channel* ch : getOverdubbableChannels())
		overdubChannel(*ch, buffer, currentFrame);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setInputMonitor(ID channelId, bool value)
{
	m_model.get().getChannel(channelId).audioReceiver->inputMonitor = value;
	m_model.swap(model::SwapType::HARD);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setOverdubProtection(ID channelId, bool value)
{
	m::Channel& ch                      = m_model.get().getChannel(channelId);
	ch.audioReceiver->overdubProtection = value;
	if (value == true && ch.armed)
		ch.armed = false;
	m_model.swap(model::SwapType::HARD);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setSamplePlayerMode(ID channelId, SamplePlayerMode mode)
{
	m_model.get().getChannel(channelId).samplePlayer->mode = mode;
	m_model.swap(model::SwapType::HARD);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setHeight(ID channelId, Pixel height)
{
	m_model.get().getChannel(channelId).height = height;
	m_model.swap(model::SwapType::SOFT);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

bool ChannelManager::hasInputRecordableChannels() const
{
	return m_hasInputRecordableChannels.load();
}

bool ChannelManager::hasActions() const
{
	return forAnyChannel([](const Channel& ch) { return ch.hasActions; });
}

bool ChannelManager::hasAudioData() const
{
	return forAnyChannel([](const Channel& ch) {
		return ch.samplePlayer && ch.samplePlayer->hasWave();
	});
}

bool ChannelManager::hasSolos() const
{
	return forAnyChannel([](const Channel& ch) {
		return !ch.isInternal() && ch.isSoloed();
	});
}

/* -------------------------------------------------------------------------- */

bool ChannelManager::forAnyChannel(std::function<bool(const Channel&)> f) const
{
	return std::any_of(m_model.get().channels.begin(), m_model.get().channels.end(), f);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::loadSampleChannel(Channel& ch, Wave* w, Frame begin, Frame end, Frame shift) const
{
	ch.samplePlayer->loadWave(*ch.shared, w, begin, end, shift);
	ch.name = w != nullptr ? w->getBasename(/*ext=*/false) : "";
}

/* -------------------------------------------------------------------------- */

std::vector<Channel*> ChannelManager::getChannelsIf(std::function<bool(const Channel&)> f)
{
	std::vector<Channel*> out;
	for (Channel& ch : m_model.get().channels)
		if (f(ch))
			out.push_back(&ch);
	return out;
}

std::vector<Channel*> ChannelManager::getRecordableChannels()
{
	return getChannelsIf([](const Channel& c) { return c.canInputRec() && !c.hasWave(); });
}

std::vector<Channel*> ChannelManager::getOverdubbableChannels()
{
	return getChannelsIf([](const Channel& c) { return c.canInputRec() && c.hasWave(); });
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setupChannelPostRecording(Channel& ch, Frame currentFrame)
{
	/* Start sample channels in loop mode right away. */
	if (ch.samplePlayer->isAnyLoopMode())
		ch.samplePlayer->kickIn(*ch.shared, currentFrame);
	/* Disable 'arm' button if overdub protection is on. */
	if (ch.audioReceiver->overdubProtection == true)
		ch.armed = false;
}

/* -------------------------------------------------------------------------- */

void ChannelManager::recordChannel(Channel& ch, const mcl::AudioBuffer& buffer, Frame recordedFrames, Frame currentFrame)
{
	assert(onChannelRecorded != nullptr);

	std::unique_ptr<Wave> wave = onChannelRecorded(recordedFrames);

	G_DEBUG("Created new Wave, size={}", wave->getBuffer().countFrames());

	/* Copy up to wave.getSize() from the mixer's input buffer into wave's. */

	wave->getBuffer().set(buffer, wave->getBuffer().countFrames());

	/* Update channel with the new Wave. */

	m_model.addShared(std::move(wave));
	loadSampleChannel(ch, &m_model.backShared<Wave>());
	setupChannelPostRecording(ch, currentFrame);

	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::overdubChannel(Channel& ch, const mcl::AudioBuffer& buffer, Frame currentFrame)
{
	Wave* wave = ch.samplePlayer->getWave();

	/* Need model::DataLock here, as data might be being read by the audio
	thread at the same time. */

	model::DataLock lock = m_model.lockData();

	wave->getBuffer().sum(buffer, /*gain=*/1.0f);
	wave->setLogical(true);

	setupChannelPostRecording(ch, currentFrame);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::triggerOnChannelsAltered()
{
	assert(onChannelsAltered != nullptr);

	m_hasInputRecordableChannels.store(forAnyChannel([](const Channel& ch) { return ch.canInputRec(); }));
	onChannelsAltered();
}
} // namespace giada::m
