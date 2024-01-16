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

#include "gui/dialogs/sampleEditor.h"
#include "channel.h"
#include "core/const.h"
#include "core/engine.h"
#include "core/kernelAudio.h"
#include "core/model/model.h"
#include "core/sequencer.h"
#include "core/wave.h"
#include "core/waveFactory.h"
#include "glue/main.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/sampleEditor/pitchTool.h"
#include "gui/elems/sampleEditor/rangeTool.h"
#include "gui/elems/sampleEditor/shiftTool.h"
#include "gui/elems/sampleEditor/waveTools.h"
#include "gui/elems/sampleEditor/waveform.h"
#include "gui/ui.h"
#include "sampleEditor.h"
#include "src/gui/elems/panTool.h"
#include "src/gui/elems/volumeTool.h"
#include "utils/gui.h"
#include "utils/log.h"
#include <FL/Fl.H>
#include <cassert>
#include <memory>

extern giada::v::Ui*     g_ui;
extern giada::m::Engine* g_engine;

namespace giada::c::sampleEditor
{
Data::Data(const m::Channel& c)
: channelId(c.id)
, name(c.name)
, volume(c.volume)
, pan(c.pan)
, pitch(c.sampleChannel->pitch)
, begin(c.sampleChannel->begin)
, end(c.sampleChannel->end)
, shift(c.sampleChannel->shift)
, waveSize(c.sampleChannel->getWave()->getBuffer().countFrames())
, waveBits(c.sampleChannel->getWave()->getBits())
, waveDuration(c.sampleChannel->getWave()->getDuration())
, waveRate(c.sampleChannel->getWave()->getRate())
, wavePath(c.sampleChannel->getWave()->getPath())
, isLogical(c.sampleChannel->getWave()->isLogical())
, m_channel(&c)
{
}

ChannelStatus Data::a_getPreviewStatus() const
{
	return g_engine->getChannelsApi().get(m::Mixer::PREVIEW_CHANNEL_ID).shared->playStatus.load();
}

Frame Data::a_getPreviewTracker() const
{
	return g_engine->getChannelsApi().get(m::Mixer::PREVIEW_CHANNEL_ID).shared->tracker.load();
}

const m::Wave& Data::getWaveRef() const
{
	return *m_channel->sampleChannel->getWave();
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
	return Data(g_engine->getChannelsApi().get(channelId));
}

/* -------------------------------------------------------------------------- */

v::gdSampleEditor* getWindow()
{
	return static_cast<v::gdSampleEditor*>(g_ui->getSubwindow(WID_SAMPLE_EDITOR));
}

/* -------------------------------------------------------------------------- */

void setBeginEnd(ID channelId, Frame b, Frame e)
{
	g_engine->getSampleEditorApi().setBeginEnd(channelId, b, e);
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
	channel::setSamplePlayerMode(m::Mixer::PREVIEW_CHANNEL_ID, shouldLoop ? SamplePlayerMode::SINGLE_ENDLESS : SamplePlayerMode::SINGLE_BASIC_PAUSE);
}

void playPreview()
{
	channel::pressChannel(m::Mixer::PREVIEW_CHANNEL_ID, G_MAX_VELOCITY_FLOAT, Thread::MAIN);
}

void stopPreview()
{
	/* Let the Sample Editor show the final tracker position first. */
	getWindow()->refresh();
	channel::pressChannel(m::Mixer::PREVIEW_CHANNEL_ID, G_MAX_VELOCITY_FLOAT, Thread::MAIN);
}

void togglePreview()
{
	const bool isPlaying = g_engine->getChannelsApi().get(m::Mixer::PREVIEW_CHANNEL_ID).isPlaying();
	isPlaying ? stopPreview() : playPreview();
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
	const int         columnIndex = g_ui->mainWindow->keyboard->getChannelColumnIndex(channelId);
	const m::Channel& newChannel  = g_engine->getSampleEditorApi().toNewChannel(channelId, a, b);
	g_ui->model.addChannelToColumn(newChannel.id, columnIndex);
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
