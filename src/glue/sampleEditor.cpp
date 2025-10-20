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

#include "src/gui/dialogs/sampleEditor.h"
#include "src/core/engine.h"
#include "src/gui/dialogs/warnings.h"
#include "src/gui/ui.h"

extern giada::v::Ui*     g_ui;
extern giada::m::Engine* g_engine;

namespace giada::c::sampleEditor
{
Data::Data(const m::Channel& c, std::size_t scene)
: channelId(c.id)
, name(c.getName(scene))
, volume(c.volume)
, pan(c.pan.asFloat())
, sample(c.sampleChannel->getSample(scene))
, waveSize(c.sampleChannel->getWave(scene)->getBuffer().countFrames())
, waveBits(c.sampleChannel->getWave(scene)->getBits())
, waveDuration(c.sampleChannel->getWave(scene)->getDuration())
, waveRate(c.sampleChannel->getWave(scene)->getRate())
, wavePath(c.sampleChannel->getWave(scene)->getPath())
, isLogical(c.sampleChannel->getWave(scene)->isLogical())
, m_channel(&c)
, m_scene(scene)
{
}

ChannelStatus Data::a_getPreviewStatus() const
{
	return g_engine->getSampleEditorApi().getPreviewStatus();
}

Frame Data::a_getPreviewTracker() const
{
	return g_engine->getSampleEditorApi().getPreviewTracker();
}

Frame Data::getFramesInBar() const
{
	return g_engine->getMainApi().getFramesInBar();
}

Frame Data::getFramesInLoop() const
{
	return g_engine->getMainApi().getFramesInLoop();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data getData(ID channelId)
{
	return Data(g_engine->getChannelsApi().get(channelId), g_engine->getMainApi().getCurrentScene());
}

/* -------------------------------------------------------------------------- */

v::gdSampleEditor* getWindow()
{
	return static_cast<v::gdSampleEditor*>(g_ui->getSubwindow(WID_SAMPLE_EDITOR));
}

/* -------------------------------------------------------------------------- */

void setRange(ID channelId, SampleRange range)
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
	getWindow()->rebuild();
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
