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

#include "core/sampleEditorEngine.h"
#include "core/engine.h"
#include "core/waveFactory.h"
#include "core/waveFx.h"
#include "utils/log.h"

namespace giada::m
{
SampleEditorEngine::SampleEditorEngine(Engine& e, model::Model& m, ChannelManager& cm)
: m_engine(e)
, m_model(m)
, m_channelManager(cm)
{
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::cut(ID channelId, Frame a, Frame b)
{
	copy(channelId, a, b);
	model::DataLock lock = m_model.lockData();
	wfx::cut(getWave(channelId), a, b);
	resetBeginEnd(channelId);
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::copy(ID channelId, Frame a, Frame b)
{
	m_waveBuffer = waveFactory::createFromWave(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::paste(ID channelId, Frame a)
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

	m_channelManager.getChannel(channelId).samplePlayer->setWave(&wave, 1.0f);

	/* Just brutally restore begin/end points. */

	resetBeginEnd(channelId);
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::silence(ID channelId, Frame a, Frame b)
{
	model::DataLock lock = m_model.lockData();
	wfx::silence(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::fade(ID channelId, Frame a, Frame b, wfx::Fade type)
{
	model::DataLock lock = m_model.lockData();
	wfx::fade(getWave(channelId), a, b, type);
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::smoothEdges(ID channelId, Frame a, Frame b)
{
	model::DataLock lock = m_model.lockData();
	wfx::smooth(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::reverse(ID channelId, Frame a, Frame b)
{
	model::DataLock lock = m_model.lockData();
	wfx::reverse(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::normalize(ID channelId, Frame a, Frame b)
{
	model::DataLock lock = m_model.lockData();
	wfx::normalize(getWave(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::trim(ID channelId, Frame a, Frame b)
{
	model::DataLock lock = m_model.lockData();
	wfx::trim(getWave(channelId), a, b);
	resetBeginEnd(channelId);
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::shift(ID channelId, Frame offset)
{
	const Channel&      ch           = m_channelManager.getChannel(channelId);
	const SamplePlayer& samplePlayer = ch.samplePlayer.value();
	const Frame         oldShift     = samplePlayer.shift;

	m::model::DataLock lock = m_model.lockData();
	m::wfx::shift(getWave(channelId), offset - oldShift);
	// Model has been swapped by DataLock constructor, needs to get Channel again
	m_channelManager.getChannel(channelId).samplePlayer->shift = offset;
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::toNewChannel(ID channelId, ID columnId, Frame a, Frame b)
{
	const int position   = m_channelManager.getLastChannelPosition(columnId);
	const int bufferSize = m_engine.getBufferSize();

	m_model.addShared(waveFactory::createFromWave(getWave(channelId), a, b));
	Wave& wave = m_model.backShared<Wave>();

	const Channel& ch = m_channelManager.addChannel(ChannelType::SAMPLE, columnId, position, bufferSize);
	m_channelManager.loadSampleChannel(ch.id, wave);
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::setBeginEnd(ID channelId, Frame b, Frame e)
{
	m_channelManager.setBeginEnd(channelId, b, e);
}

void SampleEditorEngine::resetBeginEnd(ID channelId)
{
	m_channelManager.resetBeginEnd(channelId);
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::reload(ID channelId)
{
	const int                sampleRate  = m_engine.getSampleRate();
	const Resampler::Quality rsmpQuality = m_engine.conf.data.rsmpQuality;
	// TODO - error checking
	m_channelManager.loadSampleChannel(channelId, getWave(channelId).getPath(), sampleRate, rsmpQuality);
}

/* -------------------------------------------------------------------------- */

Wave& SampleEditorEngine::getWave(ID channelId) const
{
	Channel&      ch           = m_channelManager.getChannel(channelId);
	SamplePlayer& samplePlayer = ch.samplePlayer.value();

	return *samplePlayer.getWave();
}
} // namespace giada::m
