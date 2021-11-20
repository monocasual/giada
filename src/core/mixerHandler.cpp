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

#include "core/mixerHandler.h"
#include "core/channels/channelManager.h"
#include "core/const.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/plugins/pluginManager.h"
#include "core/recorder.h"
#include "glue/channel.h"
#include "glue/main.h"
#include "utils/fs.h"
#include "utils/log.h"
#include "utils/string.h"
#include "utils/vector.h"
#include <algorithm>
#include <cassert>
#include <vector>

namespace giada::m
{
MixerHandler::MixerHandler(model::Model& model, Mixer& mixer)
: onChannelsAltered(nullptr)
, onChannelRecorded(nullptr)
, m_model(model)
, m_mixer(mixer)
{
}

/* -------------------------------------------------------------------------- */

void MixerHandler::reset(Frame framesInLoop, Frame framesInBuffer, ChannelManager& channelManager)
{
	m_mixer.reset(framesInLoop, framesInBuffer);

	m_model.get().channels.clear();

	m_model.get().channels.push_back(channelManager.create(
	    Mixer::MASTER_OUT_CHANNEL_ID, ChannelType::MASTER, /*columnId=*/0, framesInBuffer));
	m_model.get().channels.push_back(channelManager.create(
	    Mixer::MASTER_IN_CHANNEL_ID, ChannelType::MASTER, /*columnId=*/0, framesInBuffer));
	m_model.get().channels.push_back(channelManager.create(
	    Mixer::PREVIEW_CHANNEL_ID, ChannelType::PREVIEW, /*columnId=*/0, framesInBuffer));

	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

Channel& MixerHandler::addChannel(ChannelType type, ID columnId, int bufferSize,
    ChannelManager& channelManager)
{
	m_model.get().channels.push_back(channelManager.create(/*id=*/0, type, columnId, bufferSize));
	m_model.swap(model::SwapType::HARD);

	return m_model.get().channels.back();
}

/* -------------------------------------------------------------------------- */

void MixerHandler::loadChannel(ID channelId, std::unique_ptr<Wave> w)
{
	assert(onChannelsAltered != nullptr);

	m_model.addShared(std::move(w));

	Channel& channel = m_model.get().getChannel(channelId);
	Wave&    wave    = m_model.backShared<Wave>();
	Wave*    old     = channel.samplePlayer->getWave();

	channel.samplePlayer->loadWave(channel, &wave);
	m_model.swap(model::SwapType::HARD);

	/* Remove old wave, if any. It is safe to do it now: the audio thread is
	already processing the new layout. */

	if (old != nullptr)
		m_model.removeShared<Wave>(*old);

	onChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void MixerHandler::addAndLoadChannel(ID columnId, std::unique_ptr<Wave> w, int bufferSize,
    ChannelManager& channelManager)
{
	assert(onChannelsAltered != nullptr);

	m_model.addShared(std::move(w));

	Wave&    wave    = m_model.backShared<Wave>();
	Channel& channel = addChannel(ChannelType::SAMPLE, columnId, bufferSize, channelManager);

	channel.samplePlayer->loadWave(channel, &wave);
	m_model.swap(model::SwapType::HARD);

	onChannelsAltered();
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_VST
void MixerHandler::cloneChannel(ID channelId, int sampleRate, int bufferSize,
    ChannelManager& channelManager, WaveManager& waveManager, const Sequencer& sequencer,
    PluginManager& pluginManager)
#else
void MixerHandler::cloneChannel(ID channelId, int bufferSize, ChannelManager& channelManager,
    WaveManager& waveManager)
#endif
{
	const Channel& oldChannel = m_model.get().getChannel(channelId);
	Channel        newChannel = channelManager.create(oldChannel, bufferSize);

	/* Clone waves and plugins first in their own lists. */

	if (oldChannel.samplePlayer && oldChannel.samplePlayer->hasWave())
	{
		const Wave& oldWave = *oldChannel.samplePlayer->getWave();
		m_model.addShared(waveManager.createFromWave(oldWave, 0, oldWave.getBuffer().countFrames()));
		newChannel.samplePlayer->loadWave(newChannel, &m_model.backShared<Wave>());
	}

#ifdef WITH_VST
	for (const Plugin* plugin : oldChannel.plugins)
	{
		m_model.addShared(pluginManager.makePlugin(*plugin, sampleRate, bufferSize, sequencer));
		newChannel.plugins.push_back(&m_model.backShared<Plugin>());
	}
#endif

	/* Then push the new channel in the channels vector. */

	m_model.get().channels.push_back(newChannel);
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void MixerHandler::freeChannel(ID channelId)
{
	assert(onChannelsAltered != nullptr);

	Channel& ch = m_model.get().getChannel(channelId);

	assert(ch.samplePlayer);

	const Wave* wave = ch.samplePlayer->getWave();

	ch.samplePlayer->loadWave(ch, nullptr);
	m_model.swap(model::SwapType::HARD);

	if (wave != nullptr)
		m_model.removeShared<Wave>(*wave);

	onChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void MixerHandler::freeAllChannels()
{
	assert(onChannelsAltered != nullptr);

	for (Channel& ch : m_model.get().channels)
		if (ch.samplePlayer)
			ch.samplePlayer->loadWave(ch, nullptr);

	m_model.swap(model::SwapType::HARD);
	m_model.clearShared<model::WavePtrs>();

	onChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void MixerHandler::deleteChannel(ID channelId)
{
	assert(onChannelsAltered != nullptr);

	const Channel& ch   = m_model.get().getChannel(channelId);
	const Wave*    wave = ch.samplePlayer ? ch.samplePlayer->getWave() : nullptr;
#ifdef WITH_VST
	const std::vector<Plugin*> plugins = ch.plugins;
#endif

	u::vector::removeIf(m_model.get().channels, [channelId](const Channel& c) {
		return c.id == channelId;
	});
	m_model.swap(model::SwapType::HARD);

	if (wave != nullptr)
		m_model.removeShared<Wave>(*wave);

	onChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void MixerHandler::renameChannel(ID channelId, const std::string& name)
{
	m_model.get().getChannel(channelId).name = name;
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void MixerHandler::updateSoloCount()
{
	bool hasSolos = forAnyChannel([](const Channel& ch) {
		return ch.isSoloed();
	});

	m_model.get().mixer.hasSolos = hasSolos;
	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void MixerHandler::setInToOut(bool v)
{
	m_model.get().mixer.inToOut = v;
	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

float MixerHandler::getInVol() const
{
	return m_model.get().getChannel(Mixer::MASTER_IN_CHANNEL_ID).volume;
}

float MixerHandler::getOutVol() const
{
	return m_model.get().getChannel(Mixer::MASTER_OUT_CHANNEL_ID).volume;
}

bool MixerHandler::getInToOut() const
{
	return m_model.get().mixer.inToOut;
}

/* -------------------------------------------------------------------------- */

void MixerHandler::startInputRec(Frame currentFrame)
{
	m_mixer.startInputRec(currentFrame);
}

/* -------------------------------------------------------------------------- */

Frame MixerHandler::stopInputRec()
{
	return m_mixer.stopInputRec();
}

/* -------------------------------------------------------------------------- */

void MixerHandler::finalizeInputRec(Frame recordedFrames, Frame currentFrame)
{
	for (Channel* ch : getRecordableChannels())
		recordChannel(*ch, recordedFrames, currentFrame);
	for (Channel* ch : getOverdubbableChannels())
		overdubChannel(*ch, currentFrame);

	m_mixer.clearRecBuffer();

	onChannelsAltered();
}

/* -------------------------------------------------------------------------- */

bool MixerHandler::hasInputRecordableChannels() const
{
	return forAnyChannel([](const Channel& ch) { return ch.canInputRec(); });
}

bool MixerHandler::hasActionRecordableChannels() const
{
	return forAnyChannel([](const Channel& ch) { return ch.canActionRec(); });
}

bool MixerHandler::hasLogicalSamples() const
{
	return forAnyChannel([](const Channel& ch) { return ch.samplePlayer && ch.samplePlayer->hasLogicalWave(); });
}

bool MixerHandler::hasEditedSamples() const
{
	return forAnyChannel([](const Channel& ch) {
		return ch.samplePlayer && ch.samplePlayer->hasEditedWave();
	});
}

bool MixerHandler::hasActions() const
{
	return forAnyChannel([](const Channel& ch) { return ch.hasActions; });
}

bool MixerHandler::hasAudioData() const
{
	return forAnyChannel([](const Channel& ch) {
		return ch.samplePlayer && ch.samplePlayer->hasWave();
	});
}

/* -------------------------------------------------------------------------- */

bool MixerHandler::forAnyChannel(std::function<bool(const Channel&)> f) const
{
	return std::any_of(m_model.get().channels.begin(), m_model.get().channels.end(), f);
}

/* -------------------------------------------------------------------------- */

std::vector<Channel*> MixerHandler::getChannelsIf(std::function<bool(const Channel&)> f)
{
	std::vector<Channel*> out;
	for (Channel& ch : m_model.get().channels)
		if (f(ch))
			out.push_back(&ch);
	return out;
}

std::vector<Channel*> MixerHandler::getRecordableChannels()
{
	return getChannelsIf([](const Channel& c) { return c.canInputRec() && !c.hasWave(); });
}

std::vector<Channel*> MixerHandler::getOverdubbableChannels()
{
	return getChannelsIf([](const Channel& c) { return c.canInputRec() && c.hasWave(); });
}

/* -------------------------------------------------------------------------- */

void MixerHandler::setupChannelPostRecording(Channel& ch, Frame currentFrame)
{
	/* Start sample channels in loop mode right away. */
	if (ch.samplePlayer->isAnyLoopMode())
		ch.samplePlayer->kickIn(ch, currentFrame);
	/* Disable 'arm' button if overdub protection is on. */
	if (ch.audioReceiver->overdubProtection == true)
		ch.armed = false;
}

/* -------------------------------------------------------------------------- */

void MixerHandler::recordChannel(Channel& ch, Frame recordedFrames, Frame currentFrame)
{
	assert(onChannelRecorded != nullptr);

	std::unique_ptr<Wave> wave = onChannelRecorded(recordedFrames);

	G_DEBUG("Created new Wave, size=" << wave->getBuffer().countFrames());

	/* Copy up to wave.getSize() from the mixer's input buffer into wave's. */

	wave->getBuffer().set(m_mixer.getRecBuffer(), wave->getBuffer().countFrames());

	/* Update channel with the new Wave. */

	m_model.addShared(std::move(wave));
	ch.samplePlayer->loadWave(ch, &m_model.backShared<Wave>());
	setupChannelPostRecording(ch, currentFrame);

	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void MixerHandler::overdubChannel(Channel& ch, Frame currentFrame)
{
	Wave* wave = ch.samplePlayer->getWave();

	/* Need model::DataLock here, as data might be being read by the audio
	thread at the same time. */

	model::DataLock lock = m_model.lockData();

	wave->getBuffer().sum(m_mixer.getRecBuffer(), /*gain=*/1.0f);
	wave->setLogical(true);

	setupChannelPostRecording(ch, currentFrame);
}
} // namespace giada::m
