/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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
SampleEditorApi::SampleEditorApi(KernelAudio& k, model::Model& m, ChannelManager& cm,
    rendering::Reactor& re)
: m_kernelAudio(k)
, m_model(m)
, m_channelManager(cm)
, m_reactor(re)
{
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::loadPreviewChannel(ID sourceChannelId)
{
	m_channelManager.loadWaveInPreviewChannel(sourceChannelId);
	m_model.get().mixer.renderPreview = true;
	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::freePreviewChannel()
{
	m_channelManager.freeWaveInPreviewChannel();
	m_model.get().mixer.renderPreview = false;
	m_model.swap(model::SwapType::NONE);
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
	const Frame currentFrameQ    = 0;
	const float velocity         = G_MAX_VELOCITY_FLOAT;
	m_reactor.keyPress(PREVIEW_CHANNEL_ID, velocity, canRecordActions, canQuantize, currentFrameQ);
}

/* -------------------------------------------------------------------------- */

Frame SampleEditorApi::getPreviewTracker()
{
	return m_channelManager.getChannel(PREVIEW_CHANNEL_ID).shared->tracker.load();
}

/* -------------------------------------------------------------------------- */

ChannelStatus SampleEditorApi::getPreviewStatus()
{
	return m_channelManager.getChannel(PREVIEW_CHANNEL_ID).shared->playStatus.load();
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::cut(ID channelId, Frame a, Frame b)
{
	copy(channelId, a, b);
	model::SharedLock lock = m_model.lockShared();
	wfx::cut(getWave(channelId), a, b);
	resetRange(channelId);
	loadPreviewChannel(channelId); // Refresh preview channel properties
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::copy(ID channelId, Frame a, Frame b)
{
	m_waveBuffer = waveFactory::createFromWave(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::paste(ID channelId, Frame a)
{
	if (m_waveBuffer == nullptr)
	{
		u::log::print("[sampleEditor::paste] Buffer is empty, nothing to paste\n");
		return;
	}

	/* Get the existing wave in channel. */

	Wave& wave = getWave(channelId);

	/* Temporary disable wave reading in channel. From now on, the audio
	    thread won't be reading any wave, so editing it is safe.  */

	model::SharedLock lock = m_model.lockShared();

	/* Paste copied data to destination wave. */

	wfx::paste(*m_waveBuffer, wave, a);

	/* Pass the old wave that contains the pasted data to channel. */

	m_channelManager.getChannel(channelId).setWave(&wave, 1.0f, /*scene=*/0);

	/* Just brutally restore begin/end points. */

	resetRange(channelId);
	loadPreviewChannel(channelId); // Refresh preview channel properties
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::silence(ID channelId, Frame a, Frame b)
{
	model::SharedLock lock = m_model.lockShared();
	wfx::silence(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::fade(ID channelId, Frame a, Frame b, wfx::Fade type)
{
	model::SharedLock lock = m_model.lockShared();
	wfx::fade(getWave(channelId), a, b, type);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::smoothEdges(ID channelId, Frame a, Frame b)
{
	model::SharedLock lock = m_model.lockShared();
	wfx::smooth(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::reverse(ID channelId, Frame a, Frame b)
{
	model::SharedLock lock = m_model.lockShared();
	wfx::reverse(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::normalize(ID channelId, Frame a, Frame b)
{
	model::SharedLock lock = m_model.lockShared();
	wfx::normalize(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::trim(ID channelId, Frame a, Frame b)
{
	model::SharedLock lock = m_model.lockShared();
	wfx::trim(getWave(channelId), a, b);
	resetRange(channelId);
	loadPreviewChannel(channelId); // Refresh preview channel properties
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::shift(ID channelId, Frame offset)
{
	const Channel& ch       = m_channelManager.getChannel(channelId);
	const Frame    oldShift = ch.sampleChannel->shift;

	m::model::SharedLock lock = m_model.lockShared();
	m::wfx::shift(getWave(channelId), offset - oldShift);
	// Model has been swapped by DataLock constructor, needs to get Channel again
	m_channelManager.getChannel(channelId).sampleChannel->shift = offset;
}

/* -------------------------------------------------------------------------- */

const Channel& SampleEditorApi::toNewChannel(ID channelId, Frame a, Frame b)
{
	const int bufferSize = m_kernelAudio.getBufferSize();
	Wave&     wave       = m_model.addWave(waveFactory::createFromWave(getWave(channelId), a, b));

	const Channel& ch = m_channelManager.addChannel(ChannelType::SAMPLE, 0, bufferSize); // TODO trackIndex
	m_channelManager.loadSampleChannel(ch.id, wave, 0);                                  // TODO - scene

	return ch;
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::setRange(ID channelId, SampleRange range)
{
	m_channelManager.setRange(channelId, range);
}

void SampleEditorApi::resetRange(ID channelId)
{
	m_channelManager.resetRange(channelId);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::reload(ID channelId)
{
	const int                sampleRate  = m_kernelAudio.getSampleRate();
	const Resampler::Quality rsmpQuality = m_model.get().kernelAudio.rsmpQuality;
	// TODO - error checking
	m_channelManager.loadSampleChannel(channelId, getWave(channelId).getPath(), sampleRate, rsmpQuality, /*scene=*/0);
	loadPreviewChannel(channelId); // Refresh preview channel properties
}

/* -------------------------------------------------------------------------- */

Wave& SampleEditorApi::getWave(ID channelId) const
{
	return *m_channelManager.getChannel(channelId).sampleChannel->getWave(0);
}
} // namespace giada::m
