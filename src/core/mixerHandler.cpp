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
#include "core/clock.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/init.h"
#include "core/kernelAudio.h"
#include "core/kernelMidi.h"
#include "core/midiMapConf.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/recManager.h"
#include "core/recorder.h"
#include "core/recorderHandler.h"
#include "core/wave.h"
#include "core/waveFx.h"
#include "core/waveManager.h"
#include "glue/channel.h"
#include "glue/main.h"
#include "utils/fs.h"
#include "utils/log.h"
#include "utils/string.h"
#include "utils/vector.h"
#include <algorithm>
#include <cassert>
#include <vector>

namespace giada::m::mh
{
namespace
{
channel::Data& addChannel_(ChannelType type, ID columnId)
{
	model::get().channels.push_back(channelManager::create(/*id=*/0, type, columnId));
	model::swap(model::SwapType::HARD);

	return model::get().channels.back();
}

/* -------------------------------------------------------------------------- */

waveManager::Result createWave_(const std::string& fname)
{
	return waveManager::createFromFile(fname, /*id=*/0, conf::conf.samplerate,
	    conf::conf.rsmpQuality);
}

/* -------------------------------------------------------------------------- */

bool anyChannel_(std::function<bool(const channel::Data&)> f)
{
	return std::any_of(model::get().channels.begin(), model::get().channels.end(), f);
}

/* -------------------------------------------------------------------------- */

template <typename F>
std::vector<ID> getChannelsIf_(F f)
{
	std::vector<ID> ids;
	for (const channel::Data& ch : model::get().channels)
		if (f(ch))
			ids.push_back(ch.id);
	return ids;
}

std::vector<ID> getRecordableChannels_()
{
	return getChannelsIf_([](const channel::Data& c) { return c.canInputRec() && !c.hasWave(); });
}

std::vector<ID> getOverdubbableChannels_()
{
	return getChannelsIf_([](const channel::Data& c) { return c.canInputRec() && c.hasWave(); });
}

/* -------------------------------------------------------------------------- */

void setupChannelPostRecording_(channel::Data& ch)
{
	/* Start sample channels in loop mode right away. */
	if (ch.samplePlayer->isAnyLoopMode())
		samplePlayer::kickIn(ch, clock::getCurrentFrame());
	/* Disable 'arm' button if overdub protection is on. */
	if (ch.audioReceiver->overdubProtection == true)
		ch.armed = false;
}

/* -------------------------------------------------------------------------- */

/* recordChannel_
Records the current Mixer audio input data into an empty channel. */

void recordChannel_(ID channelId)
{
	/* Create a new Wave with audio coming from Mixer's virtual input. */

	std::string filename = "TAKE-" + std::to_string(patch::patch.lastTakeId++) + ".wav";

	std::unique_ptr<Wave> wave = waveManager::createEmpty(clock::getFramesInLoop(),
	    G_MAX_IO_CHANS, conf::conf.samplerate, filename);

	wave->copyData(mixer::getRecBuffer());

	/* Update channel with the new Wave. */

	model::add(std::move(wave));

	channel::Data& ch = model::get().getChannel(channelId);
	samplePlayer::loadWave(ch, &model::back<Wave>());
	setupChannelPostRecording_(ch);

	model::swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

/* overdubChannel_
Records the current Mixer audio input data into a channel with an existing
Wave, overdub mode. */

void overdubChannel_(ID channelId)
{
	Wave* wave = model::get().getChannel(channelId).samplePlayer->getWave();

	model::DataLock lock;
	wave->addData(mixer::getRecBuffer());
	wave->setLogical(true);

	setupChannelPostRecording_(model::get().getChannel(channelId));
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void init()
{
	mixer::init(clock::getFramesInLoop(), kernelAudio::getRealBufSize());

	model::get().channels.clear();

	model::get().channels.push_back(channelManager::create(
	    mixer::MASTER_OUT_CHANNEL_ID, ChannelType::MASTER, /*columnId=*/0));
	model::get().channels.push_back(channelManager::create(
	    mixer::MASTER_IN_CHANNEL_ID, ChannelType::MASTER, /*columnId=*/0));
	model::get().channels.push_back(channelManager::create(
	    mixer::PREVIEW_CHANNEL_ID, ChannelType::PREVIEW, /*columnId=*/0));

	model::swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void close()
{
	mixer::disable();
}

/* -------------------------------------------------------------------------- */

void addChannel(ChannelType type, ID columnId)
{
	addChannel_(type, columnId);
}

/* -------------------------------------------------------------------------- */

int loadChannel(ID channelId, const std::string& fname)
{
	waveManager::Result res = createWave_(fname);

	if (res.status != G_RES_OK)
		return res.status;

	model::add(std::move(res.wave));

	Wave& wave = model::back<Wave>();
	Wave* old  = model::get().getChannel(channelId).samplePlayer->getWave();

	samplePlayer::loadWave(model::get().getChannel(channelId), &wave);
	model::swap(model::SwapType::HARD);

	/* Remove old wave, if any. It is safe to do it now: the audio thread is
	already processing the new layout. */

	if (old != nullptr)
		model::remove<Wave>(*old);

	return res.status;
}

/* -------------------------------------------------------------------------- */

int addAndLoadChannel(ID columnId, const std::string& fname)
{
	waveManager::Result res = createWave_(fname);
	if (res.status == G_RES_OK)
		addAndLoadChannel(columnId, std::move(res.wave));
	return res.status;
}

void addAndLoadChannel(ID columnId, std::unique_ptr<Wave>&& w)
{
	model::add(std::move(w));

	Wave&          wave    = model::back<Wave>();
	channel::Data& channel = addChannel_(ChannelType::SAMPLE, columnId);

	samplePlayer::loadWave(channel, &wave);
	model::swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void cloneChannel(ID channelId)
{
	channel::Data& oldChannel = model::get().getChannel(channelId);
	channel::Data  newChannel = channelManager::create(oldChannel);

	/* Clone plugins, actions and wave first in their own lists. */

#ifdef WITH_VST
	newChannel.plugins = pluginHost::clonePlugins(oldChannel.plugins);
#endif
	recorderHandler::cloneActions(channelId, newChannel.id);

	if (newChannel.samplePlayer && newChannel.samplePlayer->hasWave())
	{
		Wave* wave = newChannel.samplePlayer->getWave();
		model::add(waveManager::createFromWave(*wave, 0, wave->getSize()));
	}

	/* Then push the new channel in the channels vector. */

	model::get().channels.push_back(newChannel);
	model::swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void freeChannel(ID channelId)
{
	channel::Data& ch = model::get().getChannel(channelId);

	assert(ch.samplePlayer);

	const Wave* wave = ch.samplePlayer->getWave();

	samplePlayer::loadWave(ch, nullptr);
	model::swap(model::SwapType::HARD);

	if (wave != nullptr)
		model::remove<Wave>(*wave);
}

/* -------------------------------------------------------------------------- */

void freeAllChannels()
{
	for (channel::Data& ch : model::get().channels)
		if (ch.samplePlayer)
			samplePlayer::loadWave(ch, nullptr);
	model::swap(model::SwapType::HARD);
	model::clear<model::WavePtrs>();
}

/* -------------------------------------------------------------------------- */

void deleteChannel(ID channelId)
{
	const channel::Data& ch   = model::get().getChannel(channelId);
	const Wave*          wave = ch.samplePlayer ? ch.samplePlayer->getWave() : nullptr;
#ifdef WITH_VST
	const std::vector<Plugin*> plugins = ch.plugins;
#endif

	u::vector::removeIf(model::get().channels, [channelId](const channel::Data& c) {
		return c.id == channelId;
	});
	model::swap(model::SwapType::HARD);

	if (wave != nullptr)
		model::remove<Wave>(*wave);

#ifdef WITH_VST
	pluginHost::freePlugins(plugins);
#endif
}

/* -------------------------------------------------------------------------- */

void renameChannel(ID channelId, const std::string& name)
{
	model::get().getChannel(channelId).name = name;
	model::swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void updateSoloCount()
{
	bool hasSolos = anyChannel_([](const channel::Data& ch) {
		return !ch.isInternal() && ch.solo;
	});

	model::get().mixer.hasSolos = hasSolos;
	model::swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void setInToOut(bool v)
{
	model::get().mixer.inToOut = v;
	model::swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

float getInVol()
{
	return model::get().getChannel(mixer::MASTER_IN_CHANNEL_ID).volume;
}

float getOutVol()
{
	return model::get().getChannel(mixer::MASTER_OUT_CHANNEL_ID).volume;
}

bool getInToOut()
{
	return model::get().mixer.inToOut;
}

/* -------------------------------------------------------------------------- */

/* Push a new Wave into each recordable channel. Warning: this algorithm will 
require some changes when we will allow overdubbing (the previous existing Wave
has to be overwritten somehow). */

void finalizeInputRec()
{
	for (ID id : getRecordableChannels_())
		recordChannel_(id);
	for (ID id : getOverdubbableChannels_())
		overdubChannel_(id);

	mixer::clearRecBuffer();
}

/* -------------------------------------------------------------------------- */

bool hasInputRecordableChannels()
{
	return anyChannel_([](const channel::Data& ch) { return ch.canInputRec(); });
}

bool hasActionRecordableChannels()
{
	return anyChannel_([](const channel::Data& ch) { return ch.canActionRec(); });
}

bool hasLogicalSamples()
{
	return anyChannel_([](const channel::Data& ch) { return ch.samplePlayer && ch.samplePlayer->hasLogicalWave(); });
}

bool hasEditedSamples()
{
	return anyChannel_([](const channel::Data& ch) {
		return ch.samplePlayer && ch.samplePlayer->hasEditedWave();
	});
}

bool hasActions()
{
	return anyChannel_([](const channel::Data& ch) { return ch.hasActions; });
}

bool hasAudioData()
{
	return anyChannel_([](const channel::Data& ch) {
		return ch.samplePlayer && ch.samplePlayer->hasWave();
	});
}
} // namespace giada::m::mh
