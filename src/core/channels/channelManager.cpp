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
#include "core/midiEvent.h"
#include "core/model/model.h"
#include "core/waveFactory.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "utils/log.h"

namespace giada::m
{
ChannelManager::ChannelManager(const Conf::Data& c, model::Model& model)
: m_conf(c)
, m_model(model)
{
}

/* -------------------------------------------------------------------------- */

Channel& ChannelManager::getChannel(ID channelId)
{
	return m_model.get().getChannel(channelId);
}

std::vector<Channel>& ChannelManager::getAllChannels()
{
	return m_model.get().channels;
}

/* -------------------------------------------------------------------------- */

void ChannelManager::reset(Frame framesInBuffer)
{
	m_model.get().channels.clear();

	const ID   columnId          = 0;
	const int  position          = 0;
	const bool overdubProtection = false;

	channelFactory::Data masterOutData = channelFactory::create(
	    Mixer::MASTER_OUT_CHANNEL_ID, ChannelType::MASTER, columnId, position, framesInBuffer, m_conf.rsmpQuality, overdubProtection);
	channelFactory::Data masterInData = channelFactory::create(
	    Mixer::MASTER_IN_CHANNEL_ID, ChannelType::MASTER, columnId, position, framesInBuffer, m_conf.rsmpQuality, overdubProtection);
	channelFactory::Data previewData = channelFactory::create(
	    Mixer::PREVIEW_CHANNEL_ID, ChannelType::PREVIEW, columnId, position, framesInBuffer, m_conf.rsmpQuality, overdubProtection);

	m_model.get().channels.push_back(masterOutData.channel);
	m_model.get().channels.push_back(masterInData.channel);
	m_model.get().channels.push_back(previewData.channel);

	m_model.addShared(std::move(masterOutData.shared));
	m_model.addShared(std::move(masterInData.shared));
	m_model.addShared(std::move(previewData.shared));

	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

Channel& ChannelManager::addChannel(ChannelType type, ID columnId, int position, int bufferSize)
{
	channelFactory::Data data = channelFactory::create(/*id=*/0, type, columnId, position, bufferSize, m_conf.rsmpQuality, m_conf.overdubProtectionDefaultOn);

	m_model.get().channels.push_back(data.channel);
	m_model.addShared(std::move(data.shared));
	m_model.swap(model::SwapType::HARD);

	triggerOnChannelsAltered();

	return m_model.get().channels.back();
}

/* -------------------------------------------------------------------------- */

int ChannelManager::loadSampleChannel(ID channelId, const std::string& fname, int sampleRate, Resampler::Quality quality)
{
	WaveFactory::Result res = WaveFactory::createFromFile(fname, /*id=*/0, sampleRate, quality);
	if (res.status != G_RES_OK)
		return res.status;

	m_model.addShared(std::move(res.wave));
	loadSampleChannel(channelId, m_model.backShared<Wave>());

	return G_RES_OK;
}

/* -------------------------------------------------------------------------- */

void ChannelManager::loadSampleChannel(ID channelId, Wave& wave)
{
	Wave&       newWave = wave;
	const Wave* oldWave = getWaveInSampleChannel(channelId);

	loadSampleChannel(m_model.get().getChannel(channelId), &newWave);
	m_model.swap(model::SwapType::HARD);

	/* Remove the old Wave, if any. It is safe to do it now: the audio thread is 
	already processing the new layout. */

	if (oldWave != nullptr)
		m_model.removeShared<Wave>(*oldWave);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::cloneChannel(ID channelId, int bufferSize, const std::vector<Plugin*>& plugins)
{
	const Channel&       oldChannel     = m_model.get().getChannel(channelId);
	channelFactory::Data newChannelData = channelFactory::create(oldChannel, bufferSize, m_conf.rsmpQuality);

	/* Clone Wave first, if any. */

	if (oldChannel.samplePlayer && oldChannel.samplePlayer->hasWave())
	{
		const Wave& oldWave  = *oldChannel.samplePlayer->getWave();
		const Frame oldShift = oldChannel.samplePlayer->shift;
		const Frame oldBegin = oldChannel.samplePlayer->begin;
		const Frame oldEnd   = oldChannel.samplePlayer->end;
		m_model.addShared(WaveFactory::createFromWave(oldWave));
		loadSampleChannel(newChannelData.channel, &m_model.backShared<Wave>(), oldBegin, oldEnd, oldShift);
	}

	newChannelData.channel.plugins = plugins;

	/* Then push the new channel in the channels vector. */

	m_model.get().channels.push_back(newChannelData.channel);
	m_model.addShared(std::move(newChannelData.shared));
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

void ChannelManager::keyPress(ID channelId, int velocity, bool canRecordActions, bool canQuantize, Frame currentFrameQuantized)
{
	Channel& ch = m_model.get().getChannel(channelId);

	if (ch.midiController)
		ch.midiController->keyPress(ch.shared->playStatus);
	if (ch.sampleActionRecorder && ch.hasWave() && canRecordActions && !ch.samplePlayer->isAnyLoopMode())
		ch.sampleActionRecorder->keyPress(channelId, *ch.shared, currentFrameQuantized, ch.samplePlayer->mode, ch.hasActions);
	if (ch.sampleReactor)
		ch.sampleReactor->keyPress(channelId, *ch.shared, ch.samplePlayer->mode, velocity, canQuantize, ch.samplePlayer->isAnyLoopMode(), ch.samplePlayer->velocityAsVol, ch.volume_i);

	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::keyRelease(ID channelId, bool canRecordActions, Frame currentFrameQuantized)
{
	Channel& ch = m_model.get().getChannel(channelId);

	if (ch.sampleActionRecorder && ch.hasWave() && canRecordActions && !ch.samplePlayer->isAnyLoopMode())
		ch.sampleActionRecorder->keyRelease(channelId, canRecordActions, currentFrameQuantized, ch.samplePlayer->mode, ch.hasActions);
	if (ch.sampleReactor)
		ch.sampleReactor->keyRelease(*ch.shared, ch.samplePlayer->mode);

	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::keyKill(ID channelId, bool canRecordActions, Frame currentFrameQuantized)
{
	Channel& ch = m_model.get().getChannel(channelId);

	if (ch.midiController)
		ch.midiController->keyKill(ch.shared->playStatus);
	if (ch.midiReceiver)
		ch.midiReceiver->stop(ch.shared->midiQueue);
	if (ch.midiSender && ch.isPlaying() && !ch.isMuted())
		ch.midiSender->stop();
	if (ch.sampleActionRecorder && ch.hasWave() && canRecordActions)
		ch.sampleActionRecorder->keyKill(channelId, canRecordActions, currentFrameQuantized, ch.samplePlayer->mode, ch.hasActions);
	if (ch.sampleReactor)
		ch.sampleReactor->keyKill(*ch.shared, ch.samplePlayer->mode);

	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::processMidiEvent(ID channelId, const MidiEvent& e, bool canRecordActions, Frame currentFrameQuantized)
{
	Channel& ch = m_model.get().getChannel(channelId);

	if (ch.midiActionRecorder && canRecordActions)
		ch.midiActionRecorder->record(channelId, e, currentFrameQuantized, ch.hasActions);
	if (ch.midiReceiver)
		ch.midiReceiver->parseMidi(ch.shared->midiQueue, e);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setInputMonitor(ID channelId, bool value)
{
	m_model.get().getChannel(channelId).audioReceiver->inputMonitor = value;
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setVolume(ID channelId, float value)
{
	m_model.get().getChannel(channelId).volume = std::clamp(value, 0.0f, G_MAX_VOLUME);
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPitch(ID channelId, float value)
{
	assert(m_model.get().getChannel(channelId).samplePlayer);

	m_model.get().getChannel(channelId).samplePlayer->pitch = std::clamp(value, G_MIN_PITCH, G_MAX_PITCH);
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPan(ID channelId, float value)
{
	m_model.get().getChannel(channelId).pan = std::clamp(value, 0.0f, G_MAX_PAN);
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setBeginEnd(ID channelId, Frame b, Frame e)
{
	Channel& c = m_model.get().getChannel(channelId);

	assert(c.samplePlayer);

	b = std::clamp(b, 0, c.samplePlayer->getWaveSize() - 1);
	e = std::clamp(e, 1, c.samplePlayer->getWaveSize() - 1);
	if (b >= e)
		b = e - 1;
	else if (e < b)
		e = b + 1;

	if (c.shared->tracker.load() < b)
		c.shared->tracker.store(b);

	c.samplePlayer->begin = b;
	c.samplePlayer->end   = e;
	m_model.swap(model::SwapType::HARD);
}

void ChannelManager::resetBeginEnd(ID channelId)
{
	Channel& c = m_model.get().getChannel(channelId);

	assert(c.samplePlayer);

	c.samplePlayer->begin = 0;
	c.samplePlayer->end   = c.samplePlayer->getWaveSize();
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::toggleMute(ID channelId)
{
	Channel& ch = m_model.get().getChannel(channelId);
	ch.setMute(!ch.isMuted());

	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::toggleSolo(ID channelId)
{
	Channel& ch = m_model.get().getChannel(channelId);
	ch.setSolo(!ch.isSoloed());

	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::toggleArm(ID channelId)
{
	Channel& ch = m_model.get().getChannel(channelId);
	ch.armed    = !ch.armed;

	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::toggleReadActions(ID channelId, bool seqIsRunning)
{
	assert(m_model.get().getChannel(channelId).sampleActionRecorder);

	Channel& ch = m_model.get().getChannel(channelId);
	if (!ch.hasActions)
		return;
	ch.sampleActionRecorder->toggleReadActions(*ch.shared, m_conf.treatRecsAsLoops, seqIsRunning);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::killReadActions(ID channelId)
{
	assert(m_model.get().getChannel(channelId).sampleActionRecorder);

	/* Killing Read Actions, i.e. shift + click on 'R' button is meaningful 
	only when the conf::treatRecsAsLoops is true. */

	if (!m_conf.treatRecsAsLoops)
		return;
	Channel& ch = m_model.get().getChannel(channelId);
	ch.sampleActionRecorder->killReadActions(*ch.shared);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setOverdubProtection(ID channelId, bool value)
{
	Channel& ch                         = m_model.get().getChannel(channelId);
	ch.audioReceiver->overdubProtection = value;
	if (value == true && ch.armed)
		ch.armed = false;
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setSamplePlayerMode(ID channelId, SamplePlayerMode mode)
{
	m_model.get().getChannel(channelId).samplePlayer->mode = mode;
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setHeight(ID channelId, Pixel height)
{
	m_model.get().getChannel(channelId).height = height;
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::loadWaveInPreviewChannel(ID channelId)
{
	Channel& previewCh = m_model.get().getChannel(Mixer::PREVIEW_CHANNEL_ID);
	Channel& sourceCh  = m_model.get().getChannel(channelId);

	assert(previewCh.samplePlayer);
	assert(sourceCh.samplePlayer);

	previewCh.samplePlayer->loadWave(*previewCh.shared, sourceCh.samplePlayer->getWave());
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::freeWaveInPreviewChannel()
{
	Channel& previewCh = m_model.get().getChannel(Mixer::PREVIEW_CHANNEL_ID);

	previewCh.samplePlayer->loadWave(*previewCh.shared, nullptr);
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPreviewTracker(Frame f)
{
	m_model.get().getChannel(m::Mixer::PREVIEW_CHANNEL_ID).shared->tracker.store(f);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::stopAll()
{
	for (Channel& ch : m_model.get().channels)
	{
		if (ch.midiController)
			ch.midiController->stop(ch.shared->playStatus);
		if (ch.sampleReactor && ch.samplePlayer)
			ch.sampleReactor->stopBySeq(*ch.shared, m_conf.chansStopOnSeqHalt, ch.samplePlayer->isAnyLoopMode());
		if (ch.midiSender && ch.isPlaying() && !ch.isMuted())
			ch.midiSender->stop();
		if (ch.midiReceiver)
			ch.midiReceiver->stop(ch.shared->midiQueue);
	}
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::rewindAll()
{
	for (Channel& ch : m_model.get().channels)
		if (ch.midiController)
			ch.midiController->rewind(ch.shared->playStatus);
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

bool ChannelManager::saveSample(ID channelId, const std::string& filePath)
{
	Channel& ch = m_model.get().getChannel(channelId);

	assert(ch.samplePlayer);

	Wave* wave = m_model.findShared<Wave>(ch.samplePlayer->getWaveId());

	assert(wave != nullptr);

	if (!WaveFactory::save(*wave, filePath))
		return false;

	u::log::print("[saveSample] sample saved to %s\n", filePath);

	/* Reset logical and edited states in Wave. */

	model::DataLock lock = m_model.lockData();
	wave->setLogical(false);
	wave->setEdited(false);

	return true;
}

/* -------------------------------------------------------------------------- */

void ChannelManager::consolidateChannels(const std::unordered_set<ID>& ids)
{
	for (ID id : ids)
	{
		Channel& ch = m_model.get().getChannel(id);
		ch.shared->readActions.store(true);
		ch.shared->recStatus.store(ChannelStatus::PLAY);
		if (ch.type == ChannelType::MIDI)
			ch.shared->playStatus.store(ChannelStatus::PLAY);
	}
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

bool ChannelManager::hasInputRecordableChannels() const
{
	return forAnyChannel([](const Channel& ch) { return ch.canInputRec(); });
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

const Wave* ChannelManager::getWaveInSampleChannel(ID channelId) const
{
	return m_model.get().getChannel(channelId).samplePlayer->getWave();
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
	onChannelsAltered();
}
} // namespace giada::m
