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

#include "gui/dialogs/sampleEditor.h"
#include "channel.h"
#include "core/const.h"
#include "core/engine.h"
#include "core/kernelAudio.h"
#include "core/model/model.h"
#include "core/sequencer.h"
#include "core/wave.h"
#include "core/waveFactory.h"
#include "glue/events.h"
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

extern giada::v::Ui     g_ui;
extern giada::m::Engine g_engine;

namespace giada::c::sampleEditor
{
namespace
{
m::Channel& getChannel_(ID channelId)
{
	return g_engine.model.get().getChannel(channelId);
}

m::SamplePlayer& getSamplePlayer_(ID channelId)
{
	return getChannel_(channelId).samplePlayer.value();
}

m::Wave& getWave_(ID channelId)
{
	return *const_cast<m::Wave*>(getSamplePlayer_(channelId).getWave());
}

/* -------------------------------------------------------------------------- */

/* waveBuffer
A Wave used during cut/copy/paste operations. */

std::unique_ptr<m::Wave> waveBuffer_;
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data::Data(const m::Channel& c)
: channelId(c.id)
, name(c.name)
, volume(c.volume)
, pan(c.pan)
, pitch(c.samplePlayer->pitch)
, begin(c.samplePlayer->begin)
, end(c.samplePlayer->end)
, shift(c.samplePlayer->shift)
, waveSize(c.samplePlayer->getWave()->getBuffer().countFrames())
, waveBits(c.samplePlayer->getWave()->getBits())
, waveDuration(c.samplePlayer->getWave()->getDuration())
, waveRate(c.samplePlayer->getWave()->getRate())
, wavePath(c.samplePlayer->getWave()->getPath())
, isLogical(c.samplePlayer->getWave()->isLogical())
, m_channel(&c)
{
}

ChannelStatus Data::a_getPreviewStatus() const
{
	return getChannel_(m::Mixer::PREVIEW_CHANNEL_ID).shared->playStatus.load();
}

Frame Data::a_getPreviewTracker() const
{
	return getChannel_(m::Mixer::PREVIEW_CHANNEL_ID).shared->tracker.load();
}

const m::Wave& Data::getWaveRef() const
{
	return *m_channel->samplePlayer->getWave();
}

Frame Data::getFramesInBar() const
{
	return g_engine.sequencer.getFramesInBar();
}

Frame Data::getFramesInLoop() const
{
	return g_engine.sequencer.getFramesInLoop();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data getData(ID channelId)
{
	/* Prepare the preview channel first, then return Data object. */

	g_engine.getChannelsEngine().loadPreviewChannel(channelId);
	return Data(getChannel_(channelId));
}

/* -------------------------------------------------------------------------- */

v::gdSampleEditor* getWindow()
{
	return static_cast<v::gdSampleEditor*>(g_ui.getSubwindow(*g_ui.mainWindow.get(), WID_SAMPLE_EDITOR));
}

/* -------------------------------------------------------------------------- */

void setBeginEnd(ID channelId, Frame b, Frame e)
{
	g_engine.getSampleEditorEngine().setBeginEnd(channelId, b, e);
}

/* -------------------------------------------------------------------------- */

void cut(ID channelId, Frame a, Frame b)
{
	copy(channelId, a, b);
	m::model::DataLock lock = g_engine.model.lockData();
	m::wfx::cut(getWave_(channelId), a, b);
	g_engine.getSampleEditorEngine().resetBeginEnd(channelId);
}

/* -------------------------------------------------------------------------- */

void copy(ID channelId, Frame a, Frame b)
{
	waveBuffer_ = m::waveFactory::createFromWave(getWave_(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void paste(ID channelId, Frame a)
{
	if (!isWaveBufferFull())
	{
		u::log::print("[sampleEditor::paste] Buffer is empty, nothing to paste\n");
		return;
	}

	/* Get the existing wave in channel. */

	m::Wave& wave = getWave_(channelId);

	/* Temporary disable wave reading in channel. From now on, the audio 
		thread won't be reading any wave, so editing it is safe.  */

	m::model::DataLock lock = g_engine.model.lockData();

	/* Paste copied data to destination wave. */

	m::wfx::paste(*waveBuffer_, wave, a);

	/* Pass the old wave that contains the pasted data to channel. */

	getChannel_(channelId).samplePlayer->setWave(&wave, 1.0f);

	/* Just brutally restore begin/end points. */

	g_engine.getSampleEditorEngine().resetBeginEnd(channelId);

	getWindow()->rebuild();
}

/* -------------------------------------------------------------------------- */

void silence(ID channelId, int a, int b)
{
	m::model::DataLock lock = g_engine.model.lockData();
	m::wfx::silence(getWave_(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void fade(ID channelId, int a, int b, m::wfx::Fade type)
{
	m::model::DataLock lock = g_engine.model.lockData();
	m::wfx::fade(getWave_(channelId), a, b, type);
}

/* -------------------------------------------------------------------------- */

void smoothEdges(ID channelId, int a, int b)
{
	m::model::DataLock lock = g_engine.model.lockData();
	m::wfx::smooth(getWave_(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void reverse(ID channelId, Frame a, Frame b)
{
	m::model::DataLock lock = g_engine.model.lockData();
	m::wfx::reverse(getWave_(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void normalize(ID channelId, int a, int b)
{
	m::model::DataLock lock = g_engine.model.lockData();
	m::wfx::normalize(getWave_(channelId), a, b);
}

/* -------------------------------------------------------------------------- */

void trim(ID channelId, int a, int b)
{
	m::model::DataLock lock = g_engine.model.lockData();
	m::wfx::trim(getWave_(channelId), a, b);
	g_engine.getSampleEditorEngine().resetBeginEnd(channelId);
}

/* -------------------------------------------------------------------------- */

void setLoop(bool shouldLoop)
{
	channel::setSamplePlayerMode(m::Mixer::PREVIEW_CHANNEL_ID, shouldLoop ? SamplePlayerMode::SINGLE_ENDLESS : SamplePlayerMode::SINGLE_BASIC_PAUSE);
}

void playPreview()
{
	events::pressChannel(m::Mixer::PREVIEW_CHANNEL_ID, G_MAX_VELOCITY, Thread::MAIN);
}

void stopPreview()
{
	/* Let the Sample Editor show the final tracker position, then kill the
	channel. */
	getWindow()->refresh();
	events::killChannel(m::Mixer::PREVIEW_CHANNEL_ID, Thread::MAIN);
}

void togglePreview()
{
	const bool isPlaying = getChannel_(m::Mixer::PREVIEW_CHANNEL_ID).isPlaying();
	isPlaying ? stopPreview() : playPreview();
}

void setPreviewTracker(Frame f)
{
	g_engine.getChannelsEngine().setPreviewTracker(f);
	getWindow()->refresh();
}

void cleanupPreview()
{
	g_engine.getChannelsEngine().freePreviewChannel();
}

/* -------------------------------------------------------------------------- */

void toNewChannel(ID channelId, Frame a, Frame b)
{
	auto channelEngine = g_engine.getChannelsEngine();

	const ID columnId = g_ui.mainWindow->keyboard->getChannelColumnId(channelId);

	std::unique_ptr<m::Wave> wavePtr = m::waveFactory::createFromWave(getWave_(channelId), a, b);
	g_engine.model.addShared(std::move(wavePtr));
	m::Wave& wave = g_engine.model.backShared<m::Wave>();

	const m::Channel& ch = channelEngine.add(columnId, ChannelType::SAMPLE);
	channelEngine.loadSampleChannel(ch.id, wave);
}

/* -------------------------------------------------------------------------- */

bool isWaveBufferFull()
{
	return waveBuffer_ != nullptr;
}

/* -------------------------------------------------------------------------- */

void reload(ID channelId)
{
	if (!v::gdConfirmWin(g_ui.langMapper.get(v::LangMap::COMMON_WARNING), "Reload sample: are you sure?"))
		return;
	channel::loadChannel(channelId, getWave_(channelId).getPath());
}

/* -------------------------------------------------------------------------- */

void shift(ID channelId, Frame offset)
{
	const Frame shift = getSamplePlayer_(channelId).shift;

	m::model::DataLock lock = g_engine.model.lockData();

	m::wfx::shift(getWave_(channelId), offset - shift);
	getSamplePlayer_(channelId).shift = offset; // Model has been swapped by DataLock, needs getSamplePlayer_ again
}
} // namespace giada::c::sampleEditor
