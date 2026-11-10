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

#include "src/glue/sampleEditor.h"
#include "src/core/engine.h"
#include "src/gui/dialogs/sampleEditor.h"
#include "src/gui/dialogs/warnings.h"
#include "src/gui/ui.h"
#include "src/utils/time.h"

extern giada::v::Ui*     g_ui;
extern giada::m::Engine* g_engine;

namespace giada::c::sampleEditor
{
Data::Data(const m::model::RealtimeModel::Snapshot& snapshot, ID channelId)
: channelId(channelId)
, scene(snapshot.getParameters().sequencer.getCurrentScene())
, name(snapshot.getDocument().tracks.getChannel(channelId).getName(scene))
, volume(snapshot.getDocument().tracks.getChannel(channelId).volume)
, pan(snapshot.getDocument().tracks.getChannel(channelId).pan.asFloat())
, waveSize(0)
, waveBits(0)
, waveDuration(0.0f)
, waveRate(0)
, isLogical(false)
, m_snapshot(snapshot)
, m_sampleChannelView(m::model::makeSampleChannelView(
      m_snapshot.getDocument().tracks.getChannel(channelId), m_snapshot.getAssets(), scene))
, m_previewChannelView(m::model::makeSampleChannelView(
      m_snapshot.getDocument().tracks.getChannel(PREVIEW_CHANNEL_ID), m_snapshot.getAssets(), scene))
{
	if (!m_sampleChannelView.hasWave())
		return;

	const m::model::Wave* wave = m_sampleChannelView.getWave();

	waveSize     = wave->getBuffer().countFrames();
	waveBits     = wave->getBits();
	waveDuration = wave->getDuration();
	waveRate     = wave->getRate();
	wavePath     = wave->getPath();
	isLogical    = wave->isLogical();
}

bool Data::isValid() const
{
	return getWave() != nullptr;
}

ChannelStatus Data::getPreviewStatus() const
{
	return m_previewChannelView.getParameters().getPlayStatus();
}

Frame Data::getPreviewTracker() const
{
	return m_previewChannelView.getParameters().getTracker();
}

Frame Data::getFramesInBar() const
{
	const auto& document  = m_snapshot.getDocument();
	const auto& sequencer = document.sequencer;
	return u::time::tickToFrame(sequencer.getTicksInBar(), document.kernelAudio.samplerate,
	    sequencer.getBpm());
}

Frame Data::getFramesInLoop() const
{
	const auto& document  = m_snapshot.getDocument();
	const auto& sequencer = document.sequencer;
	return u::time::tickToFrame(sequencer.getTicksInLoop(), document.kernelAudio.samplerate,
	    sequencer.getBpm());
}

Sample Data::getSample() const
{
	return m_sampleChannelView.getSample();
}

const m::model::Wave* Data::getWave() const
{
	return m_sampleChannelView.getWave();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data getData(ID channelId)
{
	const auto snapshot = g_engine->getModelSnapshot();
	return Data(snapshot, channelId);
}

/* -------------------------------------------------------------------------- */

v::gdSampleEditor* getWindow()
{
	return static_cast<v::gdSampleEditor*>(g_ui->getSubwindow(WID_SAMPLE_EDITOR));
}

/* -------------------------------------------------------------------------- */

void setRange(ID channelId, FrameRange range)
{
	g_engine->getSampleEditorApi().setRange(channelId, range);
}

/* -------------------------------------------------------------------------- */

void cut(ID channelId, Frame a, Frame b)
{
	g_engine->getSampleEditorApi().cut(channelId, a, b);
}

/* -------------------------------------------------------------------------- */

void copy(ID channelId, Frame a, Frame b)
{
	g_engine->getSampleEditorApi().copy(channelId, a, b);
}

/* -------------------------------------------------------------------------- */

void paste(ID channelId, Frame a)
{
	g_engine->getSampleEditorApi().paste(channelId, a);
}

/* -------------------------------------------------------------------------- */

void silence(ID channelId, Frame a, Frame b)
{
	g_engine->getSampleEditorApi().silence(channelId, a, b);
}

/* -------------------------------------------------------------------------- */

void fade(ID channelId, Frame a, Frame b, m::wfx::Fade type)
{
	g_engine->getSampleEditorApi().fade(channelId, a, b, type);
}

/* -------------------------------------------------------------------------- */

void smoothEdges(ID channelId, Frame a, Frame b)
{
	g_engine->getSampleEditorApi().smoothEdges(channelId, a, b);
}

/* -------------------------------------------------------------------------- */

void reverse(ID channelId, Frame a, Frame b)
{
	g_engine->getSampleEditorApi().reverse(channelId, a, b);
}

/* -------------------------------------------------------------------------- */

void normalize(ID channelId, Frame a, Frame b)
{
	g_engine->getSampleEditorApi().normalize(channelId, a, b);
}

/* -------------------------------------------------------------------------- */

void trim(ID channelId, Frame a, Frame b)
{
	g_engine->getSampleEditorApi().trim(channelId, a, b);
}

/* -------------------------------------------------------------------------- */

void preparePreview(ID channelId)
{
	g_engine->getSampleEditorApi().loadPreviewChannel(channelId);
}

void setLoop(bool shouldLoop)
{
	g_engine->getSampleEditorApi().setPreviewLoop(shouldLoop);
}

void togglePreview()
{
	g_engine->getSampleEditorApi().togglePreview();
}

void setPreviewTracker(Frame f)
{
	g_engine->getSampleEditorApi().setPreviewTracker(f);
	getWindow()->refresh();
}

void cleanupPreview()
{
	g_engine->getSampleEditorApi().freePreviewChannel();
}

/* -------------------------------------------------------------------------- */

void toNewChannel(ID channelId, Frame a, Frame b)
{
	g_engine->getSampleEditorApi().toNewChannel(channelId, a, b);
}

/* -------------------------------------------------------------------------- */

void reload(ID channelId)
{
	if (!v::gdConfirmWin(g_ui->getI18Text(v::LangMap::COMMON_WARNING), "Reload sample: are you sure?"))
		return;
	g_engine->getSampleEditorApi().reload(channelId);
}

/* -------------------------------------------------------------------------- */

void shift(ID channelId, Frame offset)
{
	g_engine->getSampleEditorApi().shift(channelId, offset);
}
} // namespace giada::c::sampleEditor
