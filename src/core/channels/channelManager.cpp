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
#include "src/core/mixer.h"
#include "src/core/model/model.h"
#include "src/core/model/modelView.h"
#include "src/core/model/wave.h"
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

/* -------------------------------------------------------------------------- */

void loadSampleChannel_(model::Channel& ch, model::Wave* w, Scene scene)
{
	const ID         waveId    = w != nullptr ? w->id : ID{};
	const auto       waveName  = w != nullptr ? w->getBasename(/*ext=*/false) : "";
	const FrameRange waveRange = w != nullptr ? FrameRange{0, w->getBuffer().countFrames()} : FrameRange{};
	ch.loadSample({waveId, waveRange}, scene);
	ch.setName(waveName, scene);
}

/* -------------------------------------------------------------------------- */

void freeSampleChannel_(model::Channel& ch, model::Assets& assets, Scene sceneToFree)
{
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
		loadSampleChannel_(ch, nullptr, Scene{sceneIndex});
	}

	for (const ID waveId : wavesToRemove)
		assets.waves.remove(waveId);
}

/* -------------------------------------------------------------------------- */

void copyChannelToScene_(model::Channel& ch, model::Assets& assets,
    Scene srcScene, Scene dstScene)
{
	if (ch.type != ChannelType::SAMPLE) // Currently relevant for Sample Channels only.
		return;

	if (!ch.sampleChannel->hasWave(srcScene) || ch.sampleChannel->hasWave(dstScene))
		return;

	/* Ideally we could just copy the Sample structure to the new scene, so that
	the Sample::waveId points to the original Wave object. This way we would end
	up with the same Wave object shared across two scenes.
	This would be very confusing for the user, though, especially when destructively
	editing the Wave (e.g. cut/copy/paste). It's better to just create a new copy
	of Wave and set it to the new scene. */

	Sample sample = ch.sampleChannel->getSample(srcScene);
	auto   wave   = waveFactory::createFromWave(*assets.waves.get(sample.waveId));

	sample.waveId = wave->id;
	ch.sampleChannel->loadSample(sample, dstScene);
	ch.setName(ch.getName(srcScene), dstScene);

	assets.waves.set(wave->id, std::move(wave));
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ChannelManager::ChannelManager(model::Model& model)
: m_model(model)
{
}

/* -------------------------------------------------------------------------- */

void ChannelManager::reset(int sampleRate, Frame framesInBuffer)
{
	const bool               overdubProtection = false;
	const Resampler::Quality rsmpQuality       = m_model.read().getDocument().kernelAudio.rsmpQuality;

	channelFactory::Data masterOutData = channelFactory::create(MASTER_OUT_CHANNEL_ID,
	    ChannelType::MASTER, sampleRate, framesInBuffer, rsmpQuality, overdubProtection);
	channelFactory::Data masterInData  = channelFactory::create(MASTER_IN_CHANNEL_ID,
	     ChannelType::MASTER, sampleRate, framesInBuffer, rsmpQuality, overdubProtection);
	channelFactory::Data previewData   = channelFactory::create(PREVIEW_CHANNEL_ID,
	      ChannelType::PREVIEW, sampleRate, framesInBuffer, rsmpQuality, overdubProtection);

	m_model.write(model::SwapType::HARD,
	    [masterOutData, masterInData, previewData](model::ModelEdit& edit) mutable
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
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
	m_model.write(model::SwapType::NONE, [bufferSize](model::AssetsEdit& edit)
	{
		model::Assets& assets = edit.get();
		for (const auto& [id, rendering] : assets.channelsRendering)
			rendering->setAudioBufferSize(bufferSize);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::addTrack(int sampleRate, Frame bufferSize)
{
	const bool               overdubProtection = false;
	const Resampler::Quality rsmpQuality       = m_model.read().getDocument().kernelAudio.rsmpQuality;
	channelFactory::Data     groupData         = channelFactory::create(/*id=*/{}, ChannelType::GROUP, sampleRate,
        bufferSize, rsmpQuality, overdubProtection);

	m_model.write(mcl::SwapType::HARD, [groupData](model::ModelEdit& edit) mutable
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		doc.tracks.add(std::move(groupData.channel), G_DEFAULT_TRACK_WIDTH, /*isInternal=*/false);
		assets.channelsParameters.set(groupData.channel.id, groupData.parameters);
		assets.channelsRendering.set(groupData.channel.id, groupData.rendering);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::removeTrack(std::size_t trackIndex)
{
	assert(canRemoveTrack(trackIndex));

	m_model.write(model::SwapType::HARD, [trackIndex](model::ModelEdit& edit)
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		const ID groupId = doc.tracks.get(trackIndex).getGroupChannel().id;

		doc.tracks.remove(trackIndex);
		assets.channelsParameters.remove(groupId);
		assets.channelsRendering.remove(groupId);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setTrackWidth(std::size_t trackIndex, int width)
{
	m_model.write(model::SwapType::HARD, [trackIndex, width](model::DocumentEdit& edit)
	{
		edit.get().tracks.get(trackIndex).width = width;
	});
}

/* -------------------------------------------------------------------------- */

ID ChannelManager::addChannel(ChannelType type, std::size_t trackIndex,
    int sampleRate, int bufferSize)
{
	const bool               overdubProtectionDefaultOn = m_model.read().getDocument().behaviors.overdubProtectionDefaultOn;
	const Resampler::Quality rsmpQuality                = m_model.read().getDocument().kernelAudio.rsmpQuality;

	channelFactory::Data data      = channelFactory::create(/*id=*/{}, type, sampleRate,
        bufferSize, rsmpQuality, overdubProtectionDefaultOn);
	const ID             channelId = data.channel.id;

	// TODO - setupChannelCallbacks(data.channel, *data.shared);

	m_model.write(model::SwapType::HARD,
	    [data, trackIndex, channelId](model::ModelEdit& edit) mutable
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
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

void ChannelManager::loadSampleChannel(ID channelId, std::shared_ptr<model::Wave> newWave, Scene scene)
{
	m_model.write(model::SwapType::HARD,
	    [channelId, newWave, scene](model::ModelEdit& edit) mutable
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		model::Channel& channel   = doc.tracks.getChannel(channelId);
		const ID        oldWaveId = channel.sampleChannel->getWaveId(scene);
		loadSampleChannel_(channel, newWave.get(), scene);
		assets.waves.set(newWave->id, std::move(newWave));
		assets.waves.remove(oldWaveId);
	});

	m_model.flush();

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::cloneChannel(ID channelId, Scene scene, int sampleRate, int bufferSize, const std::vector<Plugin*>& plugins)
{
	m_model.write(model::SwapType::HARD,
	    [channelId, scene, sampleRate, bufferSize](model::ModelEdit& edit)
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		const model::Channel&    oldChannel     = doc.tracks.getChannel(channelId);
		const std::size_t        trackIndex     = doc.tracks.getByChannel(channelId).getIndex();
		const Resampler::Quality rsmpQuality    = doc.kernelAudio.rsmpQuality;
		channelFactory::Data     newChannelData = channelFactory::create(oldChannel, sampleRate, bufferSize, rsmpQuality);
		const ID                 newChannelId   = newChannelData.channel.id;

		// TODO channel callbacks

		/* Clone Wave first, if any. */

		if (oldChannel.sampleChannel && oldChannel.sampleChannel->hasWave(scene))
		{
			const Sample& sample    = oldChannel.sampleChannel->getSample(scene);
			const ID      oldWaveId = sample.waveId;
			const Frame   oldShift  = sample.shift;
			const auto    oldRange  = sample.range;
			auto          oldWave   = assets.waves.get(oldWaveId);
			auto          newWave   = waveFactory::createFromWave(*oldWave);
			assets.waves.set(newWave->id, newWave);
			newChannelData.channel.loadSample({newWave->id, oldRange, oldShift}, scene);
		}

		// TODO plug-ins

		doc.tracks.addChannel(std::move(newChannelData.channel), trackIndex);
		assets.channelsParameters.set(newChannelId, newChannelData.parameters);
		assets.channelsRendering.set(newChannelId, newChannelData.rendering);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::copyChannelToScene(ID channelId, Scene srcScene, Scene dstScene)
{
	m_model.write(model::SwapType::HARD,
	    [channelId, srcScene, dstScene](model::ModelEdit& edit)
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		copyChannelToScene_(doc.tracks.getChannel(channelId), assets, srcScene, dstScene);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::copyAllChannelsToScene(Scene srcScene, Scene dstScene)
{
	m_model.write(model::SwapType::HARD,
	    [srcScene, dstScene](model::ModelEdit& edit)
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		doc.tracks.forEachChannel([&](model::Channel& channel)
		{
			copyChannelToScene_(channel, assets, srcScene, dstScene);
			return true;
		});
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::freeSampleChannel(ID channelId, Scene sceneToFree)
{
	m_model.write(model::SwapType::HARD,
	    [channelId, sceneToFree](model::ModelEdit& edit)
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		freeSampleChannel_(doc.tracks.getChannel(channelId), assets, sceneToFree);
	});

	m_model.flush();

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::freeAllSampleChannels(Scene scene)
{
	m_model.write(model::SwapType::HARD,
	    [scene](model::ModelEdit& edit)
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		doc.tracks.forEachChannel([&assets, scene](model::Channel& ch)
		{
			if (ch.sampleChannel)
				freeSampleChannel_(ch, assets, scene);
			return true;
		});
	});

	m_model.flush();

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::moveChannel(ID channelId, std::size_t newTrackIndex, std::size_t newPosition)
{
	m_model.write(model::SwapType::HARD, [channelId, newTrackIndex, newPosition](model::DocumentEdit& edit)
	{
		model::Document& doc = edit.get();
		model::Channel ch = doc.tracks.getChannel(channelId); // Make copy
		doc.tracks.removeChannel(channelId);
		doc.tracks.addChannel(std::move(ch), newTrackIndex, newPosition);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::deleteChannel(ID channelId)
{
	m_model.write(model::SwapType::HARD,
	    [channelId](model::ModelEdit& edit) mutable
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
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
	m_model.write(model::SwapType::SOFT, [channelId, name, scene](model::DocumentEdit& edit)
	{
		model::Document& doc = edit.get();
		for (const std::size_t sceneIndex : utils::container::range(G_MAX_NUM_SCENES))
			if (!scene.isValid() || scene.getIndex() == sceneIndex)
				doc.tracks.getChannel(channelId).setName(name, Scene{sceneIndex});
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::finalizeInputRec(const mcl::AudioBuffer& buffer,
    Frame recordedFrames, Frame currentFrame, Scene scene)
{
	for (const ID id : getRecordableChannels(scene))
		recordChannel(id, buffer, recordedFrames, currentFrame, scene);
	for (const ID id : getOverdubbableChannels(scene))
		overdubChannel(id, buffer, currentFrame, scene);

	triggerOnChannelsAltered();
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setInputMonitor(ID channelId, bool value)
{
	m_model.write(model::SwapType::HARD, [channelId, value](model::DocumentEdit& edit)
	{
		edit.get().tracks.getChannel(channelId).sampleChannel->inputMonitor = value;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setVolume(ID channelId, float value)
{
	m_model.write(model::SwapType::SOFT, [channelId, value](model::DocumentEdit& edit)
	{
		edit.get().tracks.getChannel(channelId).volume = std::clamp(value, 0.0f, G_MAX_VOLUME);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPitch(ID channelId, float value, Scene scene)
{
	m_model.write(model::SwapType::SOFT, [channelId, value, scene](model::DocumentEdit& edit)
	{
		model::Document& doc = edit.get();
		model::Channel& ch      = doc.tracks.getChannel(channelId);
		model::Channel& preview = doc.tracks.getChannel(PREVIEW_CHANNEL_ID);

		assert(ch.sampleChannel);

		const float pitch = std::clamp(value, G_MIN_PITCH, G_MAX_PITCH);

		ch.sampleChannel->setPitch(pitch, scene);
		preview.sampleChannel->setPitch(pitch, Scene{0});
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::increasePitchBySemitone(ID channelId, Scene scene)
{
	const auto            view = m_model.read();
	const model::Channel& ch   = view.getDocument().tracks.getChannel(channelId);
	assert(ch.sampleChannel);
	setPitch(channelId, ch.sampleChannel->getPitch(scene) * G_SEMITONE_RATIO, scene);
}

void ChannelManager::decreasePitchBySemitone(ID channelId, Scene scene)
{
	const auto            view = m_model.read();
	const model::Channel& ch   = view.getDocument().tracks.getChannel(channelId);
	assert(ch.sampleChannel);
	setPitch(channelId, ch.sampleChannel->getPitch(scene) / G_SEMITONE_RATIO, scene);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setTime(ID channelId, float value, Scene scene)
{
	m_model.write(model::SwapType::SOFT, [channelId, value, scene](model::DocumentEdit& edit)
	{
		model::Document& doc = edit.get();
		model::Channel& ch      = doc.tracks.getChannel(channelId);
		model::Channel& preview = doc.tracks.getChannel(PREVIEW_CHANNEL_ID);

		assert(ch.sampleChannel);

		const float time = std::clamp(value, G_MIN_TIME, G_MAX_TIME);

		ch.sampleChannel->setTime(time, scene);
		preview.sampleChannel->setTime(time, Scene{0});
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPlaybackMode(ID channelId, PlaybackMode playbackMode, Scene scene)
{
	m_model.write(model::SwapType::SOFT, [channelId, playbackMode, scene](model::DocumentEdit& edit)
	{
		model::Document& doc = edit.get();
		model::Channel& ch      = doc.tracks.getChannel(channelId);
		model::Channel& preview = doc.tracks.getChannel(PREVIEW_CHANNEL_ID);

		assert(ch.sampleChannel);

		ch.sampleChannel->setPlaybackMode(playbackMode, scene);
		preview.sampleChannel->setPlaybackMode(playbackMode, Scene{0});
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPan(ID channelId, float value)
{
	m_model.write(model::SwapType::SOFT, [channelId, value](model::DocumentEdit& edit)
	{
		edit.get().tracks.getChannel(channelId).pan = std::clamp(value, 0.0f, G_MAX_PAN);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setRange(ID channelId, FrameRange range, Scene scene)
{
	m_model.write(model::SwapType::HARD,
	    [channelId, range, scene](model::ModelEdit& edit) mutable
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		model::Channel& ch      = doc.tracks.getChannel(channelId);
		model::Channel& preview = doc.tracks.getChannel(PREVIEW_CHANNEL_ID);

		assert(ch.sampleChannel);
		assert(ch.sampleChannel->hasWave(scene));

		const auto wave = assets.waves.get(ch.sampleChannel->getWaveId(scene));

		range.setA(std::clamp(range.getA(), 0, wave->getBuffer().countFrames() - 1));
		range.setB(std::clamp(range.getB(), 1, wave->getBuffer().countFrames()));
		if (range.getA() >= range.getB())
			range.setA(range.getB() - 1);
		else if (range.getB() < range.getA())
			range.setB(range.getA() + 1);

		assets.channelsParameters.get(channelId)->setTracker(range.getA());

		ch.sampleChannel->setRange(range, scene);
		preview.sampleChannel->setRange(range, Scene{0});
	});
}

void ChannelManager::resetRange(ID channelId, Scene scene)
{
	m_model.write(model::SwapType::HARD,
	    [channelId, scene](model::ModelEdit& edit)
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		model::Channel& ch = doc.tracks.getChannel(channelId);

		assert(ch.sampleChannel);

		const auto wave = assets.waves.get(ch.sampleChannel->getWaveId(scene));

		ch.sampleChannel->setRange({0, wave->getBuffer().countFrames()}, scene);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::toggleArm(ID channelId)
{
	m_model.write(model::SwapType::SOFT, [channelId](model::DocumentEdit& edit)
	{
		model::Channel& ch = edit.get().tracks.getChannel(channelId);
		ch.armed           = !ch.armed;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setOverdubProtection(ID channelId, bool value)
{
	m_model.write(model::SwapType::SOFT, [channelId, value](model::DocumentEdit& edit)
	{
		model::Channel& ch                  = edit.get().tracks.getChannel(channelId);
		ch.sampleChannel->overdubProtection = value;
		if (value == true && ch.armed)
			ch.armed = false;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setSamplePlayerMode(ID channelId, SamplePlayerMode mode)
{
	m_model.write(model::SwapType::SOFT, [channelId, mode](model::DocumentEdit& edit)
	{
		edit.get().tracks.getChannel(channelId).sampleChannel->mode = mode;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setHeight(ID channelId, int height)
{
	m_model.write(model::SwapType::SOFT, [channelId, height](model::DocumentEdit& edit)
	{
		edit.get().tracks.getChannel(channelId).height = height;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setSendToMaster(ID channelId, bool value)
{
	m_model.write(model::SwapType::NONE, [channelId, value](model::DocumentEdit& edit)
	{
		model::Document& doc = edit.get();
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

	m_model.write(model::SwapType::HARD, [channelId, offset](model::DocumentEdit& edit)
	{
		edit.get().tracks.getChannel(channelId).extraOutputs.push_back(offset);
	});
}

void ChannelManager::removeExtraOutput(ID channelId, std::size_t i)
{
	m_model.write(model::SwapType::HARD, [channelId, i](model::DocumentEdit& edit)
	{
		model::Channel& ch = edit.get().tracks.getChannel(channelId);
		utils::container::removeAt(ch.extraOutputs, i);
		if (ch.extraOutputs.empty())
			ch.sendToMaster = true;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::loadWaveInPreviewChannel(ID channelId, Scene scene)
{
	m_model.write(model::SwapType::SOFT, [channelId, scene](model::DocumentEdit& edit)
	{
		model::Document& doc = edit.get();
		model::Channel&       previewCh = doc.tracks.getChannel(PREVIEW_CHANNEL_ID);
		const model::Channel& sourceCh  = doc.tracks.getChannel(channelId);

		assert(previewCh.sampleChannel);
		assert(sourceCh.sampleChannel);

		previewCh.loadSample(sourceCh.sampleChannel->getSample(scene), Scene{0});
		previewCh.sampleChannel->mode = SamplePlayerMode::SINGLE_BASIC_PAUSE;
		previewCh.sampleChannel->setRange(sourceCh.sampleChannel->getRange(scene), Scene{0});
		previewCh.sampleChannel->setPitch(sourceCh.sampleChannel->getPitch(scene), Scene{0});

		doc.mixer.renderPreview = true;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::freeWaveInPreviewChannel()
{
	m_model.write(model::SwapType::SOFT, [](model::DocumentEdit& edit)
	{
		model::Document& doc = edit.get();
		doc.tracks.getChannel(PREVIEW_CHANNEL_ID).loadSample({}, Scene{0});
		doc.mixer.renderPreview = false;
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setPreviewTracker(Frame f)
{
	m_model.read().getAssets().channelsParameters.get(PREVIEW_CHANNEL_ID)->setTracker(f);
}

/* -------------------------------------------------------------------------- */

void ChannelManager::finalizeActionRec(const std::unordered_set<ID>& ids)
{
	m_model.write(model::SwapType::HARD, [ids](model::ModelEdit& edit)
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		for (ID id : ids)
		{
			model::Channel& ch     = doc.tracks.getChannel(id);
			auto            params = assets.channelsParameters.get(id);

			params->setReadActions(true);
			params->setRecStatus(ChannelStatus::PLAY);
			if (ch.type == ChannelType::MIDI)
				params->setPlayStatus(ChannelStatus::PLAY);
		}
	});
}

/* -------------------------------------------------------------------------- */

bool ChannelManager::hasInputRecordableChannels(Scene scene) const
{
	return m_model.read().getDocument().tracks.hasInputRecordableChannels(scene);
}

bool ChannelManager::hasAudioData(Scene scene) const
{
	return m_model.read().getDocument().tracks.hasSampleChannelsWithWaves(scene);
}

bool ChannelManager::hasSolos() const
{
	return m_model.read().getDocument().tracks.hasSoloedChannels();
}

/* -------------------------------------------------------------------------- */

bool ChannelManager::canRemoveTrack(std::size_t trackIndex) const
{
	return m_model.read().getDocument().tracks.get(trackIndex).getNumChannels() == 1;
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

std::vector<ID> ChannelManager::getRecordableChannels(Scene scene) const
{
	return m_model.read().getDocument().tracks.getChannelsIf([scene](const model::Channel& c)
	{ return c.isRecordable(scene); });
}

std::vector<ID> ChannelManager::getOverdubbableChannels(Scene scene) const
{
	return m_model.read().getDocument().tracks.getChannelsIf([scene](const model::Channel& c)
	{ return c.isOverdubbable(scene); });
}

/* -------------------------------------------------------------------------- */

void ChannelManager::setupChannelPostRecording(
    model::Channel&                   channel,
    const model::Channel::Parameters& params,
    Frame                             currentFrame)
{
	if (channel.sampleChannel->isAnyLoopMode())
		params.kickIn(currentFrame);

	if (channel.sampleChannel->overdubProtection)
		channel.armed = false;
}

/* -------------------------------------------------------------------------- */

void ChannelManager::recordChannel(
    ID                      channelId,
    const mcl::AudioBuffer& mixerRecBuffer,
    Frame                   recordedFrames,
    Frame                   currentFrame,
    Scene                   scene)
{
	assert(onChannelRecorded != nullptr);

	std::shared_ptr<model::Wave> wave = onChannelRecorded(recordedFrames);

	/* Copy up to wave.getSize() from the mixer recording buffer into wave. */

	wave->getBuffer().setAll(mixerRecBuffer, wave->getBuffer().countFrames(), 0, 0);

	/* Update channel with the new Wave. */

	m_model.write(model::SwapType::HARD,
	    [this, channelId, wave, scene, currentFrame](model::ModelEdit& edit) mutable
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		model::Channel& channel = doc.tracks.getChannel(channelId);
		const auto      params  = assets.channelsParameters.get(channelId);
		const ID        waveId  = wave->id;

		loadSampleChannel_(channel, wave.get(), scene);
		assets.waves.set(waveId, std::move(wave));

		setupChannelPostRecording(channel, *params, currentFrame);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::overdubChannel(
    ID                      channelId,
    const mcl::AudioBuffer& mixerRecBuffer,
    Frame                   currentFrame,
    Scene                   scene)
{
	m_model.write(model::SwapType::HARD,
	    [this, channelId, &mixerRecBuffer, currentFrame, scene](model::ModelEdit& edit)
	{
		model::Document& doc    = edit.getDocument();
		model::Assets&   assets = edit.getAssets();
		model::Channel& channel = doc.tracks.getChannel(channelId);
		const ID        waveId  = channel.sampleChannel->getWaveId(scene);
		const auto      params  = assets.channelsParameters.get(channelId);

		/* Copy the wave before changing it: the audio thread may still be reading
		the original. Publish the copy once the overdub is complete. Same
		copy-on-write trick used by SampleEditorApi. */

		const auto originalWave   = assets.waves.get(waveId);
		auto       overdubbedWave = std::make_shared<model::Wave>(*originalWave);

		overdubbedWave->getBuffer().sumAll(mixerRecBuffer);
		overdubbedWave->setLogical(true);

		assets.waves.set(waveId, std::move(overdubbedWave));
		setupChannelPostRecording(channel, *params, currentFrame);
	});
}

/* -------------------------------------------------------------------------- */

void ChannelManager::triggerOnChannelsAltered()
{
	assert(onChannelsAltered != nullptr);
	onChannelsAltered();
}
} // namespace giada::m
