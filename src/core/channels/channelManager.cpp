/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/channels/channelManager.h"
#include "src/core/channels/channel.h"
#include "src/core/channels/channelFactory.h"
#include "src/core/midiEvent.h"
#include "src/core/mixer.h"
#include "src/core/model/wave.h"
#include "src/core/model_DEPR_/model.h"
#include "src/core/rendering/midiOutput.h"
#include "src/core/rendering/midiReactions.h"
#include "src/core/rendering/sampleReactions.h"
#include "src/core/waveFactory.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "src/deps/mcl-utils/src/container.hpp"
#include "src/utils/log.h"

namespace utils = mcl::utils;

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

ChannelManager::ChannelManager(model::Model& model, model_DEPR_::Model& model_depr_, MidiMapper<KernelMidi>& m, KernelMidi& km)
: m_model(model)
, m_model_DEPR_(model_depr_)
, m_kernelMidi(km)
, m_midiMapper(m)
{
}

/* -------------------------------------------------------------------------- */

Channel& ChannelManager::getChannel(ID channelId)
{
	return m_model_DEPR_.get().tracks.getChannel(channelId);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::reset(int sampleRate, Frame framesInBuffer)
{
	/* DEPR */
	{
		/* Create internal track with internal channels (Master In/Out, Preview). */

		const bool               overdubProtection = false;
		const Resampler::Quality rsmpQuality       = m_model_DEPR_.get().kernelAudio.rsmpQuality;

		channelFactory::Data_DEPR_ masterOutData = channelFactory::create_DEPR_(
		    MASTER_OUT_CHANNEL_ID, ChannelType::MASTER, sampleRate, framesInBuffer, rsmpQuality, overdubProtection);
		channelFactory::Data_DEPR_ masterInData = channelFactory::create_DEPR_(
		    MASTER_IN_CHANNEL_ID, ChannelType::MASTER, sampleRate, framesInBuffer, rsmpQuality, overdubProtection);
		channelFactory::Data_DEPR_ previewData = channelFactory::create_DEPR_(
		    PREVIEW_CHANNEL_ID, ChannelType::PREVIEW, sampleRate, framesInBuffer, rsmpQuality, overdubProtection);

		m_model_DEPR_.get().tracks = {};

		model_DEPR_::Track& track = m_model_DEPR_.get().tracks.add(std::move(masterOutData.channel), 0, /*isInternal=*/true);
		track.addChannel(std::move(masterInData.channel));
		track.addChannel(std::move(previewData.channel));

		m_model_DEPR_.addChannelShared(std::move(masterOutData.shared));
		m_model_DEPR_.addChannelShared(std::move(masterInData.shared));
		m_model_DEPR_.addChannelShared(std::move(previewData.shared));
	}
	/* DEPR */

	const bool               overdubProtection = false;
	const Resampler::Quality rsmpQuality       = m_model.getCurrentDocument()->kernelAudio.rsmpQuality;

	channelFactory::Data masterOutData = channelFactory::create(MASTER_OUT_CHANNEL_ID,
	    ChannelType::MASTER, sampleRate, framesInBuffer, rsmpQuality, overdubProtection);
	channelFactory::Data masterInData  = channelFactory::create(MASTER_IN_CHANNEL_ID,
	     ChannelType::MASTER, sampleRate, framesInBuffer, rsmpQuality, overdubProtection);
	channelFactory::Data previewData   = channelFactory::create(PREVIEW_CHANNEL_ID,
	      ChannelType::PREVIEW, sampleRate, framesInBuffer, rsmpQuality, overdubProtection);

	m_model.writeDocumentAndAssets(model::SwapType::HARD,
	    [masterOutData, masterInData, previewData](model::Document& doc, model::Assets& assets) mutable
	{
		doc.tracks = {};

		model::Track& internalTrack = doc.tracks.add(std::move(masterOutData.channel), 0, /*isInternal=*/true);
		internalTrack.addChannel(std::move(masterInData.channel));
		internalTrack.addChannel(std::move(previewData.channel));

		assets.channelsParameters.set(MASTER_OUT_CHANNEL_ID, masterOutData.parameters);
		assets.channelsParameters.set(MASTER_IN_CHANNEL_ID, masterInData.parameters);
		assets.channelsParameters.set(PREVIEW_CHANNEL_ID, previewData.parameters);
		assets.channelsRendering.set(MASTER_OUT_CHANNEL_ID, masterOutData.rendering);
		assets.channelsRendering.set(MASTER_IN_CHANNEL_ID, masterInData.rendering);
		assets.channelsRendering.set(PREVIEW_CHANNEL_ID, previewData.rendering);
	});

	addTrack(sampleRate, framesInBuffer);
	addTrack(sampleRate, framesInBuffer);
	addTrack(sampleRate, framesInBuffer);
	addTrack(sampleRate, framesInBuffer);
	addTrack(sampleRate, framesInBuffer);
	addTrack(sampleRate, framesInBuffer);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setBufferSize(int bufferSize)
{
	for (auto& channelShared : m_model_DEPR_.getAllChannelsShared())
		channelShared->setBufferSize(bufferSize);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::addTrack(int sampleRate, Frame bufferSize)
{
	const bool               overdubProtection = false;
	const Resampler::Quality rsmpQuality       = m_model.getCurrentDocument()->kernelAudio.rsmpQuality;
	channelFactory::Data     groupData         = channelFactory::create(/*id=*/{}, ChannelType::GROUP, sampleRate,
        bufferSize, rsmpQuality, overdubProtection);

	m_model.writeDocumentAndAssets(mcl::SwapType::HARD, [groupData](model::Document& doc, model::Assets& assets) mutable
	{
		doc.tracks.add(std::move(groupData.channel), G_DEFAULT_TRACK_WIDTH, /*isInternal=*/false);
		assets.channelsParameters.set(groupData.channel.id, groupData.parameters);
		assets.channelsRendering.set(groupData.channel.id, groupData.rendering);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::removeTrack(std::size_t trackIndex)
{
	assert(canRemoveTrack(trackIndex));

	m_model.writeDocumentAndAssets(model::SwapType::HARD, [trackIndex](model::Document& doc, model::Assets& assets)
	{
		const ID groupId = doc.tracks.get(trackIndex).getGroupChannel().id;

		doc.tracks.remove(trackIndex);
		assets.channelsParameters.remove(groupId);
		assets.channelsRendering.remove(groupId);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setTrackWidth(std::size_t trackIndex, int width)
{
	m_model.writeDocument(model::SwapType::HARD, [trackIndex, width](model::Document& doc)
	{
		doc.tracks.get(trackIndex).width = width;
	});
}

/* -------------------------------------------------------------------------- */

ID ChannelManager::addChannel(ChannelType type, std::size_t trackIndex,
    int sampleRate, int bufferSize)
{
	const bool               overdubProtectionDefaultOn = m_model.getCurrentDocument()->behaviors.overdubProtectionDefaultOn;
	const Resampler::Quality rsmpQuality                = m_model.getCurrentDocument()->kernelAudio.rsmpQuality;

	channelFactory::Data data      = channelFactory::create(/*id=*/{}, type, sampleRate,
        bufferSize, rsmpQuality, overdubProtectionDefaultOn);
	const ID             channelId = data.channel.id;

	// TODO - setupChannelCallbacks(data.channel, *data.shared);

	m_model.writeDocumentAndAssets(model::SwapType::HARD,
	    [data, trackIndex, channelId](model::Document& doc, model::Assets& assets) mutable
	{
		doc.tracks.addChannel(std::move(data.channel), trackIndex);
		assets.channelsParameters.set(channelId, data.parameters);
		assets.channelsRendering.set(channelId, data.rendering);
	});

	m_model.flush();

	triggerOnChannelsAltered();

	return channelId;
}

/* -------------------------------------------------------------------------- */

int ChannelManager::loadSampleChannel(ID channelId, const std::string& fname, int sampleRate,
    Resampler::Quality quality, Scene scene)
{
	waveFactory::Result res = waveFactory::createFromFile(fname, /*id=*/{}, sampleRate, quality);
	if (res.status != G_RES_OK)
		return res.status;

	loadSampleChannel(channelId, std::move(res.wave), scene);

	return G_RES_OK;
}

/* -------------------------------------------------------------------------- */

void ChannelManager::loadSampleChannel(ID channelId, Wave& wave, Scene scene)
{
	Channel&    channel = m_model_DEPR_.get().tracks.getChannel(channelId);
	Wave&       newWave = wave;
	const Wave* oldWave = channel.sampleChannel->getWave(scene);

	loadSampleChannel(channel, &newWave, scene);
	m_model_DEPR_.swap(model_DEPR_::SwapType::HARD);

	/* Remove the old Wave, if any. It is safe to do it now: the audio thread is
	already processing the new Document. */

	if (oldWave != nullptr)
		m_model_DEPR_.removeWave(*oldWave);

	triggerOnChannelsAltered();
}

void ChannelManager::loadSampleChannel(ID channelId, std::shared_ptr<model::Wave> newWave, Scene scene)
{
	m_model.writeDocumentAndAssets(model::SwapType::HARD,
	    [this, channelId, newWave, scene](model::Document& doc, model::Assets& assets) mutable
	{
		model::Channel& channel   = doc.tracks.getChannel(channelId);
		const ID        oldWaveId = channel.sampleChannel->getWaveId(scene);
		loadSampleChannel(channel, newWave.get(), scene);
		assets.waves.set(newWave->id, std::move(newWave));
		assets.waves.remove(oldWaveId);
	});

	m_model.flush();

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::cloneChannel(ID channelId, Scene scene, int sampleRate, int bufferSize, const std::vector<Plugin*>& plugins)
{
	const Channel&             oldChannel     = m_model_DEPR_.get().tracks.getChannel(channelId);
	const std::size_t          trackIndex     = m_model_DEPR_.get().tracks.getByChannel(channelId).getIndex();
	const Resampler::Quality   rsmpQuality    = m_model_DEPR_.get().kernelAudio.rsmpQuality;
	channelFactory::Data_DEPR_ newChannelData = channelFactory::create(oldChannel, sampleRate, bufferSize, rsmpQuality);

	setupChannelCallbacks(newChannelData.channel, *newChannelData.shared);

	/* Clone Wave first, if any. */

	if (oldChannel.sampleChannel && oldChannel.sampleChannel->hasWave(scene))
	{
		const Sample& sample   = oldChannel.sampleChannel->getSample(scene);
		const Wave&   oldWave  = *sample.wave;
		const Frame   oldShift = sample.shift;
		const auto    oldRange = sample.range;
		Wave&         wave     = m_model_DEPR_.addWave(waveFactory::createFromWave(oldWave));

		// TODO - new model
		newChannelData.channel.loadSample({nullptr, wave.id, oldRange, oldShift}, scene);
	}

	newChannelData.channel.plugins = plugins;

	/* Then push the new channel in the channels vector. */

	m_model_DEPR_.get().tracks.get(trackIndex).addChannel(std::move(newChannelData.channel));
	m_model_DEPR_.addChannelShared(std::move(newChannelData.shared));
	m_model_DEPR_.swap(model_DEPR_::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::copyChannelToScene(ID channelId, Scene srcScene, Scene dstScene)
{
	Channel& ch = m_model_DEPR_.get().tracks.getChannel(channelId);
	copyChannelToScene(ch, srcScene, dstScene);
	m_model_DEPR_.swap(model_DEPR_::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::copyAllChannelsToScene(Scene srcScene, Scene dstScene)
{
	m_model_DEPR_.get().tracks.forEachChannel([this, srcScene, dstScene](Channel& ch)
	{
		copyChannelToScene(ch, srcScene, dstScene);
		return true;
	});
	m_model_DEPR_.swap(model_DEPR_::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::freeSampleChannel(ID channelId, Scene sceneToFree)
{
	m_model.writeDocumentAndAssets(model::SwapType::HARD,
	    [this, channelId, sceneToFree](model::Document& doc, model::Assets& assets) mutable
	{
		model::Channel& ch = doc.tracks.getChannel(channelId);

		assert(ch.sampleChannel);

		std::vector<ID> wavesToRemove;
		for (const std::size_t sceneIndex : utils::container::range(G_MAX_NUM_SCENES))
		{
			if (sceneToFree.isValid() && sceneToFree.getIndex() != sceneIndex)
				continue;
			const ID waveId = ch.sampleChannel->getWaveId(Scene{sceneIndex});
			if (!waveId.isValid())
				continue;
			wavesToRemove.push_back(waveId);
			loadSampleChannel(ch, nullptr, Scene{sceneIndex});
		}

		for (const ID waveId : wavesToRemove)
			assets.waves.remove(waveId);
	});

	m_model.flush();

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::freeAllSampleChannels(Scene scene)
{
	// TODO - not super optmized, will swap model for each sample channel
	m_model_DEPR_.get().tracks.forEachChannel([this, scene](Channel& ch)
	{
		if (ch.sampleChannel)
			freeSampleChannel(ch.id, scene);
		return true;
	});

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::moveChannel(ID channelId, std::size_t newTrackIndex, std::size_t newPosition)
{
	m_model.writeDocument(model::SwapType::HARD, [channelId, newTrackIndex, newPosition](model::Document& doc)
	{
		model::Channel ch = doc.tracks.getChannel(channelId); // Make copy
		doc.tracks.removeChannel(channelId);
		doc.tracks.addChannel(std::move(ch), newTrackIndex, newPosition);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::deleteChannel(ID channelId)
{
	m_model.writeDocumentAndAssets(model::SwapType::HARD,
	    [channelId](model::Document& doc, model::Assets& assets) mutable
	{
		const model::Channel& ch = doc.tracks.getChannel(channelId);

		if (ch.type == ChannelType::SAMPLE)
			for (const Sample& sample : ch.sampleChannel->getSamples())
				if (sample.waveId.isValid())
					assets.waves.remove(sample.waveId);

		doc.tracks.getByChannel(channelId).removeChannel(channelId);
	});

	m_model.flush();

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::renameChannel(ID channelId, const std::string& name, Scene scene)
{
	m_model.writeDocument(model::SwapType::SOFT, [channelId, name, scene](model::Document& doc)
	{
		for (const std::size_t sceneIndex : utils::container::range(G_MAX_NUM_SCENES))
			if (!scene.isValid() || scene.getIndex() == sceneIndex)
				doc.tracks.getChannel(channelId).setName(name, Scene{sceneIndex});
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::finalizeInputRec(const mcl::AudioBuffer& buffer, Frame recordedFrames, Frame currentFrame, Scene scene)
{
	for (Channel* ch : getRecordableChannels(scene))
		recordChannel(*ch, buffer, recordedFrames, currentFrame, scene);
	for (Channel* ch : getOverdubbableChannels(scene))
		overdubChannel(*ch, buffer, currentFrame, scene);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setInputMonitor(ID channelId, bool value)
{
	m_model_DEPR_.get().tracks.getChannel(channelId).sampleChannel->inputMonitor = value;
	m_model_DEPR_.swap(model_DEPR_::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setVolume(ID channelId, float value)
{
	m_model.writeDocument(model::SwapType::SOFT, [channelId, value](model::Document& doc)
	{
		doc.tracks.getChannel(channelId).volume = std::clamp(value, 0.0f, G_MAX_VOLUME);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPitch(ID channelId, float value, Scene scene)
{
	Channel& c       = m_model_DEPR_.get().tracks.getChannel(channelId);
	Channel& preview = m_model_DEPR_.get().tracks.getChannel(PREVIEW_CHANNEL_ID);

	assert(c.sampleChannel);

	const float pitch = std::clamp(value, G_MIN_PITCH, G_MAX_PITCH);

	c.sampleChannel->setPitch(pitch, scene);
	preview.sampleChannel->setPitch(pitch, Scene{0});
	m_model_DEPR_.swap(model_DEPR_::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::increasePitchBySemitone(ID channelId, Scene scene)
{
	const Channel& ch = getChannel(channelId);
	assert(ch.sampleChannel);
	setPitch(channelId, ch.sampleChannel->getPitch(scene) * G_SEMITONE_RATIO, scene);
}

void ChannelManager::decreasePitchBySemitone(ID channelId, Scene scene)
{
	const Channel& ch = getChannel(channelId);
	assert(ch.sampleChannel);
	setPitch(channelId, ch.sampleChannel->getPitch(scene) / G_SEMITONE_RATIO, scene);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setTime(ID channelId, float value, Scene scene)
{
	Channel& c       = m_model_DEPR_.get().tracks.getChannel(channelId);
	Channel& preview = m_model_DEPR_.get().tracks.getChannel(PREVIEW_CHANNEL_ID);

	assert(c.sampleChannel);

	const float time = std::clamp(value, G_MIN_TIME, G_MAX_TIME);

	c.sampleChannel->setTime(time, scene);
	preview.sampleChannel->setTime(time, Scene{0});
	m_model_DEPR_.swap(model_DEPR_::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPlaybackMode(ID channelId, PlaybackMode playbackMode, Scene scene)
{
	Channel& c       = m_model_DEPR_.get().tracks.getChannel(channelId);
	Channel& preview = m_model_DEPR_.get().tracks.getChannel(PREVIEW_CHANNEL_ID);

	assert(c.sampleChannel);

	c.sampleChannel->setPlaybackMode(playbackMode, scene);
	preview.sampleChannel->setPlaybackMode(playbackMode, Scene{0});
	m_model_DEPR_.swap(model_DEPR_::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPan(ID channelId, float value)
{
	m_model_DEPR_.get().tracks.getChannel(channelId).pan = std::clamp(value, 0.0f, G_MAX_PAN);
	m_model_DEPR_.swap(model_DEPR_::SwapType::SOFT);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setRange(ID channelId, FrameRange range, Scene scene)
{
	Channel& c       = m_model_DEPR_.get().tracks.getChannel(channelId);
	Channel& preview = m_model_DEPR_.get().tracks.getChannel(PREVIEW_CHANNEL_ID);

	assert(c.sampleChannel);

	range.setA(std::clamp(range.getA(), 0, c.sampleChannel->getWaveSize(scene) - 1));
	range.setB(std::clamp(range.getB(), 1, c.sampleChannel->getWaveSize(scene)));
	if (range.getA() >= range.getB())
		range.setA(range.getB() - 1);
	else if (range.getB() < range.getA())
		range.setB(range.getA() + 1);

	if (c.shared->tracker.load() < range.getA())
		c.shared->tracker.store(range.getA());

	c.sampleChannel->setRange(range, scene);
	preview.sampleChannel->setRange(range, Scene{0});
	m_model_DEPR_.swap(model_DEPR_::SwapType::HARD);
}

void ChannelManager::resetRange(ID channelId, Scene scene)
{
	Channel& c = m_model_DEPR_.get().tracks.getChannel(channelId);

	assert(c.sampleChannel);

	c.sampleChannel->setRange({0, c.sampleChannel->getWaveSize(scene)}, scene);
	m_model_DEPR_.swap(model_DEPR_::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::toggleArm(ID channelId)
{
	m_model.writeDocument(model::SwapType::SOFT, [channelId](model::Document& doc)
	{
		model::Channel& ch = doc.tracks.getChannel(channelId);
		ch.armed           = !ch.armed;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setOverdubProtection(ID channelId, bool value)
{
	m_model.writeDocument(model::SwapType::SOFT, [channelId, value](model::Document& doc)
	{
		model::Channel& ch                  = doc.tracks.getChannel(channelId);
		ch.sampleChannel->overdubProtection = value;
		if (value == true && ch.armed)
			ch.armed = false;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setSamplePlayerMode(ID channelId, SamplePlayerMode mode)
{
	m_model.writeDocument(model::SwapType::SOFT, [channelId, mode](model::Document& doc)
	{
		doc.tracks.getChannel(channelId).sampleChannel->mode = mode;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setHeight(ID channelId, int height)
{
	m_model.writeDocument(model::SwapType::SOFT, [channelId, height](model::Document& doc)
	{
		doc.tracks.getChannel(channelId).height = height;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setSendToMaster(ID channelId, bool value)
{
	m_model.writeDocument(model::SwapType::NONE, [channelId, value](model::Document& doc)
	{
		/* Can't toggle 'send to master' flag if there are no extra outputs. */
		assert(doc.tracks.getChannel(channelId).extraOutputs.size() > 0);

		doc.tracks.getChannel(channelId).sendToMaster = value;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::addExtraOutput(ID channelId, int offset)
{
	/* 'offset' can be only multiple of 2, that is stereo pair. */
	assert(offset >= 0 && offset % 2 == 0);

	m_model.writeDocument(model::SwapType::HARD, [channelId, offset](model::Document& doc)
	{
		doc.tracks.getChannel(channelId).extraOutputs.push_back(offset);
	});
}

void ChannelManager::removeExtraOutput(ID channelId, std::size_t i)
{
	m_model.writeDocument(model::SwapType::HARD, [channelId, i](model::Document& doc)
	{
		model::Channel& ch = doc.tracks.getChannel(channelId);
		utils::container::removeAt(ch.extraOutputs, i);
		if (ch.extraOutputs.empty())
			ch.sendToMaster = true;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::loadWaveInPreviewChannel(ID channelId, Scene scene)
{
	m_model.writeDocument(model::SwapType::SOFT, [channelId, scene](model::Document& doc)
	{
		model::Channel&       previewCh = doc.tracks.getChannel(PREVIEW_CHANNEL_ID);
		const model::Channel& sourceCh  = doc.tracks.getChannel(channelId);

		assert(previewCh.sampleChannel);
		assert(sourceCh.sampleChannel);

		previewCh.loadSample(sourceCh.sampleChannel->getSample(scene), Scene{0});
		previewCh.sampleChannel->mode = SamplePlayerMode::SINGLE_BASIC_PAUSE;
		previewCh.sampleChannel->setRange(sourceCh.sampleChannel->getRange(scene), Scene{0});
		previewCh.sampleChannel->setPitch(sourceCh.sampleChannel->getPitch(scene), Scene{0});
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::freeWaveInPreviewChannel()
{
	m_model.writeDocument(model::SwapType::SOFT, [](model::Document& doc)
	{
		doc.tracks.getChannel(PREVIEW_CHANNEL_ID).loadSample({}, Scene{0});
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPreviewTracker(Frame f)
{
	m_model_DEPR_.get().tracks.getChannel(PREVIEW_CHANNEL_ID).shared->tracker.store(f);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::finalizeActionRec(const std::unordered_set<ID>& ids)
{
	for (ID id : ids)
	{
		Channel& ch = m_model_DEPR_.get().tracks.getChannel(id);
		ch.shared->readActions.store(true);
		ch.shared->recStatus.store(ChannelStatus::PLAY);
		if (ch.type == ChannelType::MIDI)
			ch.shared->playStatus.store(ChannelStatus::PLAY);
	}
	m_model_DEPR_.swap(model_DEPR_::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

bool ChannelManager::hasInputRecordableChannels(Scene scene) const
{
	return m_model.getCurrentDocument()->tracks.hasInputRecordableChannels(scene);
}

bool ChannelManager::hasAudioData(Scene scene) const
{
	return m_model.getCurrentDocument()->tracks.hasSampleChannelsWithWaves(scene);
}

bool ChannelManager::hasSolos() const
{
	return m_model.getCurrentDocument()->tracks.hasSoloedChannels();
}

/* -------------------------------------------------------------------------- */

bool ChannelManager::canRemoveTrack(std::size_t trackIndex) const
{
	return m_model.getCurrentDocument()->tracks.get(trackIndex).getNumChannels() == 1;
}

/* -------------------------------------------------------------------------- */

void ChannelManager::loadSampleChannel(Channel& ch, Wave* w, Scene scene) const
{
	ch.loadSample({w, {}}, scene);
	ch.setName(w != nullptr ? w->getBasename(/*ext=*/false) : "", scene);
}

void ChannelManager::loadSampleChannel(model::Channel& ch, model::Wave* w, Scene scene) const
{
	const ID         waveId    = w != nullptr ? w->id : ID{};
	const auto       waveName  = w != nullptr ? w->getBasename(/*ext=*/false) : "";
	const FrameRange waveRange = w != nullptr ? FrameRange{0, w->getBuffer().countFrames()} : FrameRange{};
	ch.loadSample({nullptr, waveId, waveRange}, scene);
	ch.setName(waveName, scene);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setupChannelCallbacks(const Channel& ch, ChannelShared& shared) const
{
	/*
	TODO
	assert(onChannelPlayStatusChanged != nullptr);

	shared.playStatus.onChange = [this, id = ch.id](ChannelStatus status)
	{
	    onChannelPlayStatusChanged(id, status);
	};

	if (ch.type == ChannelType ::SAMPLE)
	{
	    shared.quantizer->schedule(Q_ACTION_PLAY + ch.id.getValue(), [&shared](Frame delta)
	    {
	        rendering::playSampleChannel(shared, delta);
	    });
	    shared.quantizer->schedule(Q_ACTION_REWIND + ch.id.getValue(), [&shared](Frame delta)
	    {
	        const ChannelStatus status = shared.playStatus.load();
	        if (status == ChannelStatus::OFF)
	            rendering::playSampleChannel(shared, delta);
	        else if (status == ChannelStatus::PLAY || status == ChannelStatus::ENDING)
	            rendering::rewindSampleChannel(shared, delta);
	    });
	}
	*/
}

/* -------------------------------------------------------------------------- */

std::vector<Channel*> ChannelManager::getRecordableChannels(Scene scene)
{
	return m_model_DEPR_.get().tracks.getChannelsIf([scene](const Channel& c)
	{ return c.canInputRec(scene) && !c.hasWave(scene); });
}

std::vector<Channel*> ChannelManager::getOverdubbableChannels(Scene scene)
{
	return m_model_DEPR_.get().tracks.getChannelsIf([scene](const Channel& c)
	{ return c.canInputRec(scene) && c.hasWave(scene); });
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

void ChannelManager::recordChannel(Channel& ch, const mcl::AudioBuffer& buffer, Frame recordedFrames, Frame currentFrame, Scene scene)
{
	assert(onChannelRecorded != nullptr);

	std::unique_ptr<Wave> wave = onChannelRecorded(recordedFrames);

	G_DEBUG("Created new Wave, size={}", wave->getBuffer().countFrames());

	/* Copy up to wave.getSize() from the mixer's input buffer into wave's. */

	wave->getBuffer().setAll(buffer, wave->getBuffer().countFrames(), 0, 0);

	/* Update channel with the new Wave. */

	loadSampleChannel(ch, &m_model_DEPR_.addWave(std::move(wave)), scene);
	setupChannelPostRecording(ch, currentFrame);

	m_model_DEPR_.swap(model_DEPR_::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::overdubChannel(Channel& ch, const mcl::AudioBuffer& buffer, Frame currentFrame, Scene scene)
{
	Wave* wave = ch.sampleChannel->getWave(scene);

	/* Need model::DataLock here, as data might be being read by the audio
	thread at the same time. */

	model_DEPR_::SharedLock lock = m_model_DEPR_.lockShared();

	wave->getBuffer().sumAll(buffer);
	wave->setLogical(true);

	setupChannelPostRecording(ch, currentFrame);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::copyChannelToScene(Channel& ch, Scene srcScene, Scene dstScene)
{
	if (ch.type != ChannelType::SAMPLE) // Currently relevant for Sample Channels only.
		return;

	if (!ch.sampleChannel->hasWave(srcScene) || ch.sampleChannel->hasWave(dstScene))
		return;

	/* Ideally we could just copy the Sample structure to the new scene, so that
	the Sample::wave pointer points to the original, and now shared, Wave object.
	This would be very confusing for the user, though, especially when destructively
	editing the Wave (e.g. cut/copy/paste). It's better to just create a new copy
	of Wave and set it to the new scene. */

	Sample sample = ch.sampleChannel->getSample(srcScene);
	Wave&  wave   = m_model_DEPR_.addWave(waveFactory::createFromWave(*sample.wave));

	sample.wave = &wave;
	ch.loadSample(sample, dstScene);
	ch.setName(ch.getName(srcScene), dstScene);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::triggerOnChannelsAltered()
{
	assert(onChannelsAltered != nullptr);
	onChannelsAltered();
}
} // namespace giada::m
