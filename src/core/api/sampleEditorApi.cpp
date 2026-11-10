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

#include "src/core/api/sampleEditorApi.h"
#include "src/core/channels/channelManager.h"
#include "src/core/kernelAudio.h"
#include "src/core/mixer.h"
#include "src/core/rendering/reactor.h"
#include "src/core/waveFactory.h"
#include "src/core/waveFx.h"
#include "src/utils/log.h"

namespace giada::m
{
namespace
{
template <typename Edit>
void editWave(model::RealtimeModel& model, ID channelId, Scene scene, Edit edit)
{
	model.writeDocumentAndAssets(model::SwapType::HARD,
	    [channelId, scene, edit = std::move(edit)](model::Document& doc, model::Assets& assets)
	{
		model::Channel& channel      = doc.tracks.getChannel(channelId);
		const ID        waveId       = channel.sampleChannel->getWaveId(scene);
		const auto      originalWave = assets.waves.get(waveId);
		auto            editedWave   = std::make_shared<model::Wave>(*originalWave);

		edit(*editedWave);

		assets.waves.set(waveId, std::move(editedWave));
	});
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

SampleEditorApi::SampleEditorApi(model::RealtimeModel& m, KernelAudio& k, ChannelManager& cm,
    rendering::Reactor& re, Sequencer& s)
: m_model(m)
, m_kernelAudio(k)
, m_channelManager(cm)
, m_reactor(re)
, m_sequencer(s)
{
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::loadPreviewChannel(ID sourceChannelId)
{
	m_channelManager.loadWaveInPreviewChannel(sourceChannelId, m_sequencer.getCurrentScene());
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::freePreviewChannel()
{
	m_channelManager.freeWaveInPreviewChannel();
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::setPreviewTracker(Frame f)
{
	m_channelManager.setPreviewTracker(f);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::setPreviewLoop(bool shouldLoop)
{
	m_channelManager.setSamplePlayerMode(PREVIEW_CHANNEL_ID, shouldLoop ? SamplePlayerMode::SINGLE_ENDLESS : SamplePlayerMode::SINGLE_BASIC_PAUSE);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::togglePreview()
{
	const bool  canRecordActions = false;
	const bool  canQuantize      = false;
	const Tick  currentTickQ     = Tick{0};
	const float velocity         = G_MAX_VELOCITY_FLOAT;
	const Scene scene            = m_sequencer.getCurrentScene();
	m_reactor.keyPress(PREVIEW_CHANNEL_ID, scene, velocity, canRecordActions, canQuantize, currentTickQ);
}

/* -------------------------------------------------------------------------- */

Frame SampleEditorApi::getPreviewTracker()
{
	return m_model.getCurrentAssets()->channelsParameters.get(PREVIEW_CHANNEL_ID)->getTracker();
}

/* -------------------------------------------------------------------------- */

ChannelStatus SampleEditorApi::getPreviewStatus()
{
	return m_model.getCurrentAssets()->channelsParameters.get(PREVIEW_CHANNEL_ID)->getPlayStatus();
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::cut(ID channelId, Frame a, Frame b)
{
	/*
	copy(channelId, a, b);
	model_DEPR_::SharedLock lock = m_model_DEPR_.lockShared();
	wfx::cut(getWave(channelId), a, b);
	resetRange(channelId);
	loadPreviewChannel(channelId); // Refresh preview channel properties
	*/
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::copy(ID channelId, Frame a, Frame b)
{
	const Scene           scene   = m_sequencer.getCurrentScene();
	const model::Channel& channel = m_model.getCurrentDocument()->tracks.getChannel(channelId);
	const ID              waveId  = channel.sampleChannel->getWaveId(scene);
	const auto            wave    = m_model.getCurrentAssets()->waves.get(waveId);

	m_waveBuffer = std::make_unique<model::Wave>(*waveFactory::createFromWave(*wave, a, b));
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::paste(ID channelId, Frame a)
{
#if 0
	if (m_waveBuffer == nullptr)
	{
		u::log::print("[sampleEditor::paste] Buffer is empty, nothing to paste\n");
		return;
	}

	/* Get the existing wave in channel. */

	Wave& wave = getWave(channelId);

	/* Temporary disable wave reading in channel. From now on, the audio
	    thread won't be reading any wave, so editing it is safe.  */

	model_DEPR_::SharedLock lock = m_model_DEPR_.lockShared();

	/* Paste copied data to destination wave. */

	wfx::paste(*m_waveBuffer, wave, a);

	/* Pass the old wave that contains the pasted data to channel. */

	m_channelManager.getChannel(channelId).setWave(&wave, Scene{0}, 1.0f); // TODO - scenes

	/* Just brutally restore begin/end points. */

	resetRange(channelId);
	loadPreviewChannel(channelId); // Refresh preview channel properties
#endif
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::silence(ID channelId, Frame a, Frame b)
{
	const Scene scene = m_sequencer.getCurrentScene();

	editWave(m_model, channelId, scene, [a, b](model::Wave& wave)
	{
		wfx::silence(wave, a, b);
	});
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::fade(ID channelId, Frame a, Frame b, wfx::Fade type)
{
	const Scene scene = m_sequencer.getCurrentScene();

	editWave(m_model, channelId, scene, [a, b, type](model::Wave& wave)
	{
		wfx::fade(wave, a, b, type);
	});
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::smoothEdges(ID channelId, Frame a, Frame b)
{
	const Scene scene = m_sequencer.getCurrentScene();

	editWave(m_model, channelId, scene, [a, b](model::Wave& wave)
	{
		wfx::smooth(wave, a, b);
	});
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::reverse(ID channelId, Frame a, Frame b)
{
	const Scene scene = m_sequencer.getCurrentScene();

	editWave(m_model, channelId, scene, [a, b](model::Wave& wave)
	{
		wfx::reverse(wave, a, b);
	});
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::normalize(ID channelId, Frame a, Frame b)
{
	const Scene scene = m_sequencer.getCurrentScene();

	editWave(m_model, channelId, scene, [a, b](model::Wave& wave)
	{
		wfx::normalize(wave, a, b);
	});
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::trim(ID channelId, Frame a, Frame b)
{
	const Scene scene = m_sequencer.getCurrentScene();

	editWave(m_model, channelId, scene, [a, b](model::Wave& wave)
	{
		wfx::smooth(wave, a, b);
	});

	m_model.flush();

	resetRange(channelId);
	loadPreviewChannel(channelId); // Refresh preview channel properties
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::shift(ID channelId, Frame offset)
{
#if 0
	const Channel& ch       = m_channelManager.getChannel(channelId);
	const Scene    scene    = m_sequencer.getCurrentScene();
	const Frame    oldShift = ch.sampleChannel->getShift(scene);

	m::model_DEPR_::SharedLock lock = m_model_DEPR_.lockShared();
	// m::wfx::shift(getWave(channelId), offset - oldShift);
	//  Model has been swapped by DataLock constructor, needs to get Channel again
	m_channelManager.getChannel(channelId).sampleChannel->setShift(offset, scene);
#endif
}

/* -------------------------------------------------------------------------- */

ID SampleEditorApi::toNewChannel(ID channelId, Frame a, Frame b)
{
	/*
	const int sampleRate = m_kernelAudio.getSampleRate();
	const int bufferSize = m_kernelAudio.getBufferSize();
	Wave&     wave       = m_model_DEPR_.addWave(waveFactory::createFromWave_DEPR_(getWave(channelId), a, b));

	const ID newChannelId = m_channelManager.addChannel(ChannelType::SAMPLE, 0, sampleRate, bufferSize); // TODO trackIndex
	m_channelManager.loadSampleChannel(newChannelId, wave, m_sequencer.getCurrentScene());

	return newChannelId;
	*/
	return {};
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::setRange(ID channelId, FrameRange range)
{
	m_channelManager.setRange(channelId, range, m_sequencer.getCurrentScene());
}

void SampleEditorApi::resetRange(ID channelId)
{
	m_channelManager.resetRange(channelId, m_sequencer.getCurrentScene());
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::reload(ID channelId)
{
	/*
	const int                sampleRate  = m_kernelAudio.getSampleRate();
	const Resampler::Quality rsmpQuality = m_model_DEPR_.get().kernelAudio.rsmpQuality;
	// TODO - error checking
	m_channelManager.loadSampleChannel(channelId, getWave(channelId).getPath(), sampleRate, rsmpQuality, Scene{0});
	loadPreviewChannel(channelId); // Refresh preview channel properties
	*/
}
} // namespace giada::m
