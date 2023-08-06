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

#include "core/channels/channelManager.h"
#include "core/channels/channel.h"
#include "core/channels/channelFactory.h"
#include "core/midiEvent.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/rendering/midiChannel.h"
#include "core/waveFactory.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "utils/log.h"

namespace giada::m
{
namespace
{
constexpr int Q_ACTION_PLAY   = 0;
constexpr int Q_ACTION_REWIND = 10000; // Avoid clash with Q_ACTION_PLAY + channelId
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ChannelManager::ChannelManager(model::Model& model, MidiMapper<KernelMidi>& m, ActionRecorder& a, KernelMidi& km)
: m_model(model)
, m_kernelMidi(km)
, m_midiLighter(m)
, m_midiActionRecorder(a)
, m_sampleActionRecorder(a)
{
}

/* -------------------------------------------------------------------------- */

Channel& ChannelManager::getChannel(ID channelId)
{
	return m_model.get().channels.get(channelId);
}

std::vector<Channel>& ChannelManager::getAllChannels()
{
	return m_model.get().channels.getAll();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::reset(Frame framesInBuffer)
{
	m_model.get().channels = {};

	const ID                 columnId          = 0;
	const int                position          = 0;
	const bool               overdubProtection = false;
	const Resampler::Quality rsmpQuality       = m_model.get().kernelAudio.rsmpQuality;

	channelFactory::Data masterOutData = channelFactory::create(
	    Mixer::MASTER_OUT_CHANNEL_ID, ChannelType::MASTER, columnId, position, framesInBuffer, rsmpQuality, overdubProtection);
	channelFactory::Data masterInData = channelFactory::create(
	    Mixer::MASTER_IN_CHANNEL_ID, ChannelType::MASTER, columnId, position, framesInBuffer, rsmpQuality, overdubProtection);
	channelFactory::Data previewData = channelFactory::create(
	    Mixer::PREVIEW_CHANNEL_ID, ChannelType::PREVIEW, columnId, position, framesInBuffer, rsmpQuality, overdubProtection);

	m_model.get().channels.add(masterOutData.channel);
	m_model.get().channels.add(masterInData.channel);
	m_model.get().channels.add(previewData.channel);

	m_model.addChannelShared(std::move(masterOutData.shared));
	m_model.addChannelShared(std::move(masterInData.shared));
	m_model.addChannelShared(std::move(previewData.shared));
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setBufferSize(int bufferSize)
{
	for (auto& channelShared : m_model.getAllChannelsShared())
		channelShared->setBufferSize(bufferSize);
}

/* -------------------------------------------------------------------------- */

Channel& ChannelManager::addChannel(ChannelType type, ID columnId, int position, int bufferSize)
{
	const bool               overdubProtectionDefaultOn = m_model.get().behaviors.overdubProtectionDefaultOn;
	const Resampler::Quality rsmpQuality                = m_model.get().kernelAudio.rsmpQuality;

	channelFactory::Data data = channelFactory::create(/*id=*/0, type, columnId, position, bufferSize, rsmpQuality, overdubProtectionDefaultOn);

	setupChannelCallbacks(data.channel, *data.shared);

	m_model.get().channels.add(data.channel);
	m_model.addChannelShared(std::move(data.shared));
	m_model.swap(model::SwapType::HARD);

	triggerOnChannelsAltered();

	return m_model.get().channels.getLast();
}

/* -------------------------------------------------------------------------- */

int ChannelManager::loadSampleChannel(ID channelId, const std::string& fname, int sampleRate, Resampler::Quality quality)
{
	waveFactory::Result res = waveFactory::createFromFile(fname, /*id=*/0, sampleRate, quality);
	if (res.status != G_RES_OK)
		return res.status;

	loadSampleChannel(channelId, m_model.addWave(std::move(res.wave)));

	return G_RES_OK;
}

/* -------------------------------------------------------------------------- */

void ChannelManager::loadSampleChannel(ID channelId, Wave& wave)
{
	Channel&    channel = m_model.get().channels.get(channelId);
	Wave&       newWave = wave;
	const Wave* oldWave = channel.sampleChannel->getWave();

	loadSampleChannel(channel, &newWave);
	m_model.swap(model::SwapType::HARD);

	/* Remove the old Wave, if any. It is safe to do it now: the audio thread is 
	already processing the new layout. */

	if (oldWave != nullptr)
		m_model.removeWave(*oldWave);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::cloneChannel(ID channelId, int bufferSize, const std::vector<Plugin*>& plugins)
{
	const Channel&           oldChannel     = m_model.get().channels.get(channelId);
	const Resampler::Quality rsmpQuality    = m_model.get().kernelAudio.rsmpQuality;
	channelFactory::Data     newChannelData = channelFactory::create(oldChannel, bufferSize, rsmpQuality);

	setupChannelCallbacks(newChannelData.channel, *newChannelData.shared);

	/* Clone Wave first, if any. */

	if (oldChannel.sampleChannel && oldChannel.sampleChannel->hasWave())
	{
		const Wave& oldWave  = *oldChannel.sampleChannel->getWave();
		const Frame oldShift = oldChannel.sampleChannel->shift;
		const Frame oldBegin = oldChannel.sampleChannel->begin;
		const Frame oldEnd   = oldChannel.sampleChannel->end;
		Wave&       wave     = m_model.addWave(waveFactory::createFromWave(oldWave));
		loadSampleChannel(newChannelData.channel, &wave, oldBegin, oldEnd, oldShift);
	}

	newChannelData.channel.plugins = plugins;

	/* Then push the new channel in the channels vector. */

	m_model.get().channels.add(newChannelData.channel);
	m_model.addChannelShared(std::move(newChannelData.shared));
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::freeSampleChannel(ID channelId)
{
	Channel& ch = m_model.get().channels.get(channelId);

	assert(ch.sampleChannel);

	const Wave* wave = ch.sampleChannel->getWave();

	loadSampleChannel(ch, nullptr);
	m_model.swap(model::SwapType::HARD);

	if (wave != nullptr)
		m_model.removeWave(*wave);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::freeAllSampleChannels()
{
	for (Channel& ch : m_model.get().channels.getAll())
		if (ch.sampleChannel)
			loadSampleChannel(ch, nullptr);

	m_model.swap(model::SwapType::HARD);
	m_model.clearWaves();

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::deleteChannel(ID channelId)
{
	const Channel& ch   = m_model.get().channels.get(channelId);
	const Wave*    wave = ch.sampleChannel ? ch.sampleChannel->getWave() : nullptr;

	m_model.get().channels.remove(channelId);
	m_model.swap(model::SwapType::HARD);

	if (wave != nullptr)
		m_model.removeWave(*wave);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::renameChannel(ID channelId, const std::string& name)
{
	m_model.get().channels.get(channelId).name = name;
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::moveChannel(ID channelId, ID newColumnId, int newPosition)
{
	model::Channels& channels = m_model.get().channels;

	/* Make room in the destination column for the new channel. */

	for (Channel& ch : channels.getAll())
		if (ch.columnId == newColumnId && ch.position >= newPosition)
			ch.position++;

	Channel& channel = channels.get(channelId);
	channel.columnId = newColumnId;
	channel.position = newPosition;

	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

float ChannelManager::getMasterInVol() const
{
	return m_model.get().channels.get(Mixer::MASTER_IN_CHANNEL_ID).volume;
}

float ChannelManager::getMasterOutVol() const
{
	return m_model.get().channels.get(Mixer::MASTER_OUT_CHANNEL_ID).volume;
}

/* -------------------------------------------------------------------------- */

int ChannelManager::getLastChannelPosition(ID columnId) const
{
	std::vector<const Channel*> column = m_model.get().channels.getColumn(columnId);
	return column.empty() ? 0 : column.back()->position + 1;
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
	Channel& ch = m_model.get().channels.get(channelId);

	if (ch.type == ChannelType::MIDI)
	{
		m_midiController.keyPress(ch.shared->playStatus);
	}
	else if (ch.type == ChannelType::SAMPLE && ch.hasWave())
	{
		const bool             isAnyLoopMode = ch.sampleChannel->isAnyLoopMode();
		const bool             velocityAsVol = ch.sampleChannel->velocityAsVol;
		const SamplePlayerMode mode          = ch.sampleChannel->mode;

		if (canRecordActions && !isAnyLoopMode)
			m_sampleActionRecorder.keyPress(channelId, *ch.shared, currentFrameQuantized, mode, ch.hasActions);

		m_sampleReactor.keyPress(channelId, *ch.shared, mode, velocity, canQuantize, isAnyLoopMode, velocityAsVol, ch.volume_i);
	}

	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::keyRelease(ID channelId, bool canRecordActions, Frame currentFrameQuantized)
{
	Channel& ch = m_model.get().channels.get(channelId);

	if (ch.type != ChannelType::SAMPLE || !ch.hasWave())
		return;

	const bool             isAnyLoopMode = ch.sampleChannel->isAnyLoopMode();
	const SamplePlayerMode mode          = ch.sampleChannel->mode;

	if (canRecordActions && !isAnyLoopMode)
		m_sampleActionRecorder.keyRelease(channelId, canRecordActions, currentFrameQuantized, mode, ch.hasActions);

	m_sampleReactor.keyRelease(*ch.shared, mode);

	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::keyKill(ID channelId, bool canRecordActions, Frame currentFrameQuantized)
{
	Channel& ch = m_model.get().channels.get(channelId);

	if (ch.type == ChannelType::MIDI)
	{
		m_midiController.keyKill(ch.shared->playStatus);
		rendering::sendMidiAllNotesOffToPlugins(ch.shared->midiQueue);

		if (ch.isPlaying() && ch.canSendMidi())
			rendering::sendMidiAllNotesOff(ch.midiChannel->outputFilter, m_kernelMidi);
	}
	else if (ch.type == ChannelType::SAMPLE)
	{
		const SamplePlayerMode mode = ch.sampleChannel->mode;

		if (ch.hasWave() && canRecordActions)
			m_sampleActionRecorder.keyKill(channelId, canRecordActions, currentFrameQuantized, mode, ch.hasActions);

		m_sampleReactor.keyKill(*ch.shared, mode);
	}

	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::processMidiEvent(ID channelId, const MidiEvent& e, bool canRecordActions, Frame currentFrameQuantized)
{
	Channel& ch = m_model.get().channels.get(channelId);

	assert(ch.type == ChannelType::MIDI);

	if (canRecordActions)
		m_midiActionRecorder.record(channelId, e, currentFrameQuantized, ch.hasActions);

	rendering::sendMidiEventToPlugins(ch.shared->midiQueue, e);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setInputMonitor(ID channelId, bool value)
{
	m_model.get().channels.get(channelId).sampleChannel->inputMonitor = value;
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setVolume(ID channelId, float value)
{
	m_model.get().channels.get(channelId).volume = std::clamp(value, 0.0f, G_MAX_VOLUME);
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPitch(ID channelId, float value)
{
	assert(m_model.get().channels.get(channelId).sampleChannel);

	m_model.get().channels.get(channelId).sampleChannel->pitch = std::clamp(value, G_MIN_PITCH, G_MAX_PITCH);
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPan(ID channelId, float value)
{
	m_model.get().channels.get(channelId).pan = std::clamp(value, 0.0f, G_MAX_PAN);
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setBeginEnd(ID channelId, Frame b, Frame e)
{
	Channel& c = m_model.get().channels.get(channelId);

	assert(c.sampleChannel);

	b = std::clamp(b, 0, c.sampleChannel->getWaveSize() - 1);
	e = std::clamp(e, 1, c.sampleChannel->getWaveSize() - 1);
	if (b >= e)
		b = e - 1;
	else if (e < b)
		e = b + 1;

	if (c.shared->tracker.load() < b)
		c.shared->tracker.store(b);

	c.sampleChannel->begin = b;
	c.sampleChannel->end   = e;
	m_model.swap(model::SwapType::HARD);
}

void ChannelManager::resetBeginEnd(ID channelId)
{
	Channel& c = m_model.get().channels.get(channelId);

	assert(c.sampleChannel);

	c.sampleChannel->begin = 0;
	c.sampleChannel->end   = c.sampleChannel->getWaveSize();
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::toggleMute(ID channelId)
{
	Channel&   ch      = m_model.get().channels.get(channelId);
	const bool newMute = !ch.isMuted();

	ch.setMute(newMute);

	m_model.swap(model::SwapType::SOFT);
	m_midiLighter.sendMute(ch.midiLightning, newMute);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::toggleSolo(ID channelId)
{
	Channel&   ch      = m_model.get().channels.get(channelId);
	const bool newSolo = !ch.isSoloed();

	ch.setSolo(newSolo);

	m_model.swap(model::SwapType::SOFT);
	m_midiLighter.sendSolo(ch.midiLightning, newSolo);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::toggleArm(ID channelId)
{
	Channel& ch = m_model.get().channels.get(channelId);
	ch.armed    = !ch.armed;

	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::toggleReadActions(ID channelId, bool seqIsRunning)
{
	Channel& ch = m_model.get().channels.get(channelId);
	if (!ch.hasActions)
		return;
	m_sampleActionRecorder.toggleReadActions(*ch.shared, m_model.get().behaviors.treatRecsAsLoops, seqIsRunning);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::killReadActions(ID channelId)
{
	/* Killing Read Actions, i.e. shift + click on 'R' button is meaningful 
	only when the treatRecsAsLoops flag is true. */

	if (!m_model.get().behaviors.treatRecsAsLoops)
		return;
	Channel& ch = m_model.get().channels.get(channelId);
	m_sampleActionRecorder.killReadActions(*ch.shared);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setOverdubProtection(ID channelId, bool value)
{
	Channel& ch                         = m_model.get().channels.get(channelId);
	ch.sampleChannel->overdubProtection = value;
	if (value == true && ch.armed)
		ch.armed = false;
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setSamplePlayerMode(ID channelId, SamplePlayerMode mode)
{
	m_model.get().channels.get(channelId).sampleChannel->mode = mode;
	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setHeight(ID channelId, Pixel height)
{
	m_model.get().channels.get(channelId).height = height;
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::loadWaveInPreviewChannel(ID channelId)
{
	Channel& previewCh = m_model.get().channels.get(Mixer::PREVIEW_CHANNEL_ID);
	Channel& sourceCh  = m_model.get().channels.get(channelId);

	assert(previewCh.sampleChannel);
	assert(sourceCh.sampleChannel);

	previewCh.loadWave(sourceCh.sampleChannel->getWave());
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::freeWaveInPreviewChannel()
{
	Channel& previewCh = m_model.get().channels.get(Mixer::PREVIEW_CHANNEL_ID);

	previewCh.loadWave(nullptr);
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPreviewTracker(Frame f)
{
	m_model.get().channels.get(m::Mixer::PREVIEW_CHANNEL_ID).shared->tracker.store(f);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::stopAll()
{
	for (Channel& ch : m_model.get().channels.getAll())
	{
		if (ch.type == ChannelType::MIDI)
		{
			m_midiController.stop(ch.shared->playStatus);
			rendering::sendMidiAllNotesOffToPlugins(ch.shared->midiQueue);

			if (ch.isPlaying() && ch.canSendMidi())
				rendering::sendMidiAllNotesOff(ch.midiChannel->outputFilter, m_kernelMidi);
		}
		else if (ch.type == ChannelType::SAMPLE)
		{
			m_sampleReactor.stopBySeq(*ch.shared, m_model.get().behaviors.chansStopOnSeqHalt, ch.sampleChannel->isAnyLoopMode());
		}
	}
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::rewindAll()
{
	for (Channel& ch : m_model.get().channels.getAll())
		if (ch.type == ChannelType::MIDI)
			m_midiController.rewind(ch.shared->playStatus);
	m_model.swap(model::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

bool ChannelManager::saveSample(ID channelId, const std::string& filePath)
{
	Channel& ch = m_model.get().channels.get(channelId);

	assert(ch.sampleChannel);

	Wave* wave = ch.sampleChannel->getWave();

	assert(wave != nullptr);

	if (!waveFactory::save(*wave, filePath))
		return false;

	u::log::print("[saveSample] sample saved to {}\n", filePath);

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
		Channel& ch = m_model.get().channels.get(id);
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
	return m_model.get().channels.anyOf([](const Channel& ch) { return ch.canInputRec(); });
}

bool ChannelManager::hasActions() const
{
	return m_model.get().channels.anyOf([](const Channel& ch) { return ch.hasActions; });
}

bool ChannelManager::hasAudioData() const
{
	return m_model.get().channels.anyOf([](const Channel& ch) {
		return ch.sampleChannel && ch.sampleChannel->hasWave();
	});
}

bool ChannelManager::hasSolos() const
{
	return m_model.get().channels.anyOf([](const Channel& ch) {
		return !ch.isInternal() && ch.isSoloed();
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::loadSampleChannel(Channel& ch, Wave* w, Frame begin, Frame end, Frame shift) const
{
	ch.loadWave(w, begin, end, shift);
	ch.name = w != nullptr ? w->getBasename(/*ext=*/false) : "";
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setupChannelCallbacks(const Channel& ch, ChannelShared& shared) const
{
	shared.playStatus.onChange = [this, midiLightning = ch.midiLightning](ChannelStatus status) {
		m_midiLighter.sendStatus(midiLightning, status, /*isAudible=*/true /* TODO!!! */);
	};

	if (ch.type == ChannelType ::SAMPLE)
	{
		shared.quantizer->schedule(Q_ACTION_PLAY + ch.id, [this, channelId = ch.id](Frame delta) {
			Channel& ch = m_model.get().channels.get(channelId);
			m_sampleReactor.play(*ch.shared, delta);
		});
		shared.quantizer->schedule(Q_ACTION_REWIND + ch.id, [this, channelId = ch.id](Frame delta) {
			Channel&            ch     = m_model.get().channels.get(channelId);
			const ChannelStatus status = ch.shared->playStatus.load();
			if (status == ChannelStatus::OFF)
				m_sampleReactor.play(*ch.shared, delta);
			else if (status == ChannelStatus::PLAY || status == ChannelStatus::ENDING)
				m_sampleReactor.rewind(*ch.shared, delta);
		});
	}
}

/* -------------------------------------------------------------------------- */

std::vector<Channel*> ChannelManager::getRecordableChannels()
{
	return m_model.get().channels.getIf([](const Channel& c) { return c.canInputRec() && !c.hasWave(); });
}

std::vector<Channel*> ChannelManager::getOverdubbableChannels()
{
	return m_model.get().channels.getIf([](const Channel& c) { return c.canInputRec() && c.hasWave(); });
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setupChannelPostRecording(Channel& ch, Frame currentFrame)
{
	/* Start sample channels in loop mode right away. */
	if (ch.sampleChannel->isAnyLoopMode())
		ch.kickIn(currentFrame);
	/* Disable 'arm' button if overdub protection is on. */
	if (ch.sampleChannel->overdubProtection == true)
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

	loadSampleChannel(ch, &m_model.addWave(std::move(wave)));
	setupChannelPostRecording(ch, currentFrame);

	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::overdubChannel(Channel& ch, const mcl::AudioBuffer& buffer, Frame currentFrame)
{
	Wave* wave = ch.sampleChannel->getWave();

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
