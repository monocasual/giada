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

#include "sampleEditorApi.h"
#include "core/channels/channelManager.h"
#include "core/kernelAudio.h"
#include "core/waveFactory.h"
#include "core/waveFx.h"
#include "utils/log.h"

namespace giada::m
{
SampleEditorApi::SampleEditorApi(KernelAudio& k, model::Model& m, ChannelManager& cm)
: m_kernelAudio(k)
, m_model(m)
, m_channelManager(cm)
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

void SampleEditorApi::cut(ID channelId, Frame a, Frame b)
{
	copy(channelId, a, b);
	model::DataLock lock = m_model.lockData();
	wfx::cut(getWave(channelId), a, b);
	resetBeginEnd(channelId);
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

	model::DataLock lock = m_model.lockData();

	/* Paste copied data to destination wave. */

	wfx::paste(*m_waveBuffer, wave, a);

	/* Pass the old wave that contains the pasted data to channel. */

	m_channelManager.getChannel(channelId).setWave(&wave, 1.0f);

	/* Just brutally restore begin/end points. */

	resetBeginEnd(channelId);
	loadPreviewChannel(channelId); // Refresh preview channel properties
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::silence(ID channelId, Frame a, Frame b)
{
	model::DataLock lock = m_model.lockData();
	wfx::silence(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::fade(ID channelId, Frame a, Frame b, wfx::Fade type)
{
	model::DataLock lock = m_model.lockData();
	wfx::fade(getWave(channelId), a, b, type);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::smoothEdges(ID channelId, Frame a, Frame b)
{
	model::DataLock lock = m_model.lockData();
	wfx::smooth(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::reverse(ID channelId, Frame a, Frame b)
{
	model::DataLock lock = m_model.lockData();
	wfx::reverse(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::normalize(ID channelId, Frame a, Frame b)
{
	model::DataLock lock = m_model.lockData();
	wfx::normalize(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::trim(ID channelId, Frame a, Frame b)
{
	model::DataLock lock = m_model.lockData();
	wfx::trim(getWave(channelId), a, b);
	resetBeginEnd(channelId);
	loadPreviewChannel(channelId); // Refresh preview channel properties
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::shift(ID channelId, Frame offset)
{
	const Channel& ch       = m_channelManager.getChannel(channelId);
	const Frame    oldShift = ch.sampleChannel->shift;

	m::model::DataLock lock = m_model.lockData();
	m::wfx::shift(getWave(channelId), offset - oldShift);
	// Model has been swapped by DataLock constructor, needs to get Channel again
	m_channelManager.getChannel(channelId).sampleChannel->shift = offset;
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::toNewChannel(ID channelId, Frame a, Frame b)
{
	const int bufferSize = m_kernelAudio.getBufferSize();
	Wave&     wave       = m_model.addWave(waveFactory::createFromWave(getWave(channelId), a, b));

	const Channel& ch = m_channelManager.addChannel(ChannelType::SAMPLE, bufferSize);
	m_channelManager.loadSampleChannel(ch.id, wave);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::setBeginEnd(ID channelId, Frame b, Frame e)
{
	m_channelManager.setBeginEnd(channelId, b, e);
}

void SampleEditorApi::resetBeginEnd(ID channelId)
{
	m_channelManager.resetBeginEnd(channelId);
}

/* -------------------------------------------------------------------------- */

void SampleEditorApi::reload(ID channelId)
{
	const int                sampleRate  = m_kernelAudio.getSampleRate();
	const Resampler::Quality rsmpQuality = m_model.get().kernelAudio.rsmpQuality;
	// TODO - error checking
	m_channelManager.loadSampleChannel(channelId, getWave(channelId).getPath(), sampleRate, rsmpQuality);
}

/* -------------------------------------------------------------------------- */

Wave& SampleEditorApi::getWave(ID channelId) const
{
	return *m_channelManager.getChannel(channelId).sampleChannel->getWave();
}
} // namespace giada::m
