/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include <FL/Fl.H>
#include "glue/events.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/sampleEditor/waveTools.h"
#include "gui/elems/sampleEditor/volumeTool.h"
#include "gui/elems/sampleEditor/boostTool.h"
#include "gui/elems/sampleEditor/panTool.h"
#include "gui/elems/sampleEditor/pitchTool.h"
#include "gui/elems/sampleEditor/rangeTool.h"
#include "gui/elems/sampleEditor/shiftTool.h"
#include "gui/elems/sampleEditor/waveform.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "core/model/model.h"
#include "core/wave.h"
#include "core/waveManager.h"
#include "core/mixerHandler.h"
#include "core/const.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "channel.h"
#include "sampleEditor.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace c {
namespace sampleEditor
{
namespace
{
/* waveBuffer
A Wave used during cut/copy/paste operations. */

std::unique_ptr<m::Wave> waveBuffer_;

Frame previewTracker_ = 0;


/* -------------------------------------------------------------------------- */

/* resetBeginEnd_
Resets begin/end points when model has changed and a new Channel pointer is
needed for the operation. */

void resetBeginEnd_(ID channelId)
{
	m::model::onGet(m::model::channels, channelId, [&](const m::Channel& c)
	{
		Frame begin = c.samplePlayer->state->begin.load();
		Frame end   = c.samplePlayer->state->end.load();
		setBeginEnd(channelId, begin, end);
	});
}


/* -------------------------------------------------------------------------- */

/* updateWavePtr_
Updates the Wave pointer in Channel::WaveReader. */

void updateWavePtr_(ID channelId, ID waveId)
{
	namespace mm = m::model;

	mm::WavesLock wl(mm::waves);
	const m::Wave& wave = mm::get(mm::waves, waveId);

	mm::onSwap(mm::channels, channelId, [&](m::Channel& c)
	{
		c.samplePlayer->loadWave(&wave);
	});
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Data::Data(const m::Channel& c, const m::Wave& w)
: channelId   (c.id)
, waveId      (w.id)
, name        (c.state->name)
, volume      (c.state->volume.load())
, pan         (c.state->pan.load())
, pitch       (c.samplePlayer->state->pitch.load())
, begin       (c.samplePlayer->state->begin.load())
, end         (c.samplePlayer->state->end.load())
, shift       (c.samplePlayer->state->shift.load())
, waveSize    (w.getSize())
, waveBits    (w.getBits())
, waveDuration(w.getDuration())
, waveRate    (w.getRate())
, wavePath    (w.getPath())
, isLogical   (w.isLogical())
{
}

/* TODO - use c::channel::a_get() */
ChannelStatus Data::a_getPreviewStatus() const
{
	namespace mm = m::model;

	mm::ChannelsLock l(mm::channels);
	return mm::get(mm::channels, m::mixer::PREVIEW_CHANNEL_ID).state->playStatus.load();
}

/* TODO - use c::channel::a_get() */
Frame Data::a_getPreviewTracker() const
{
	namespace mm = m::model;

	mm::ChannelsLock l(mm::channels);
	return mm::get(mm::channels, m::mixer::PREVIEW_CHANNEL_ID).samplePlayer->state->tracker.load();
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Data getData(ID channelId)
{
	namespace mm = m::model;

	mm::ChannelsLock cl(mm::channels);
	mm::WavesLock        wl(mm::waves);

	const m::Channel& channel = mm::get(mm::channels, channelId);
	const m::Wave&        wave    = mm::get(mm::waves, channel.samplePlayer->getWaveId());

	/* Prepare the preview channel. */

	m::Channel& preview = mm::get(mm::channels, m::mixer::PREVIEW_CHANNEL_ID);
	preview.samplePlayer->loadWave(&wave);

	return Data(channel, wave);
}


/* -------------------------------------------------------------------------- */


void onRefresh(bool gui, std::function<void(v::gdSampleEditor&)> f)
{
	v::gdSampleEditor* se = static_cast<v::gdSampleEditor*>(u::gui::getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	if (se == nullptr) 
		return;
	if (!gui) Fl::lock();
	f(*se);
	if (!gui) Fl::unlock();
}


v::gdSampleEditor* getSampleEditorWindow()
{
	v::gdSampleEditor* se = static_cast<v::gdSampleEditor*>(u::gui::getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	assert(se != nullptr);
	return se;
}


/* -------------------------------------------------------------------------- */


void setBeginEnd(ID channelId, int b, int e)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		b = std::clamp(b, 0, c.samplePlayer->getWaveSize() - 1);
		e = std::clamp(e, 1, c.samplePlayer->getWaveSize() - 1);
		if      (b >= e) b = e - 1;
		else if (e < b)  e = b + 1;

		c.samplePlayer->state->begin.store(b);
		c.samplePlayer->state->end.store(e);
		if (c.samplePlayer->state->tracker.load() < b)
			c.samplePlayer->state->tracker.store(b);
	});

	/* TODO waveform widget is dumb and wants a rebuild. Refactoring needed! */
	getSampleEditorWindow()->rebuild();
}


/* -------------------------------------------------------------------------- */


void cut(ID channelId, ID waveId, int a, int b)
{
	copy(waveId, a, b);
	m::wfx::cut(waveId, a, b);
	updateWavePtr_(channelId, waveId);
	resetBeginEnd_(channelId);
}


/* -------------------------------------------------------------------------- */


void copy(ID waveId, int a, int b)
{
	m::model::WavesLock lock(m::model::waves);
	waveBuffer_ = m::waveManager::createFromWave(m::model::get(m::model::waves, waveId), a, b);
}


/* -------------------------------------------------------------------------- */


void paste(ID channelId, ID waveId, int a)
{
	if (!isWaveBufferFull()) {
		u::log::print("[sampleEditor::paste] Buffer is empty, nothing to paste\n");
		return;
	}

	m::wfx::paste(*waveBuffer_, waveId, a);
	updateWavePtr_(channelId, waveId);

	/* Shift begin/end points to keep the previous position. */

	int   delta = waveBuffer_->getSize();
	Frame begin;
	Frame end;

	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		begin = c.samplePlayer->state->begin.load();
		end   = c.samplePlayer->state->end.load();
	});

	if (a < begin && a < end)
		setBeginEnd(channelId, begin + delta, end + delta);
	else
	if (a < end)
		setBeginEnd(channelId, begin, end + delta);

	getSampleEditorWindow()->rebuild();
}


/* -------------------------------------------------------------------------- */


void silence(ID channelId, ID waveId, int a, int b)
{
	m::wfx::silence(waveId, a, b);
	updateWavePtr_(channelId, waveId);
}


/* -------------------------------------------------------------------------- */


void fade(ID channelId, ID waveId, int a, int b, m::wfx::Fade type)
{
	m::wfx::fade(waveId, a, b, type);
	updateWavePtr_(channelId, waveId);
}


/* -------------------------------------------------------------------------- */


void smoothEdges(ID channelId, ID waveId, int a, int b)
{
	m::wfx::smooth(waveId, a, b);
	updateWavePtr_(channelId, waveId);
}


/* -------------------------------------------------------------------------- */


void reverse(ID channelId, ID waveId, int a, int b)
{
	m::wfx::reverse(waveId, a, b);
	updateWavePtr_(channelId, waveId);
}


/* -------------------------------------------------------------------------- */


void normalize(ID channelId, ID waveId, int a, int b)
{
	m::wfx::normalize(waveId, a, b);
	updateWavePtr_(channelId, waveId);
}


/* -------------------------------------------------------------------------- */


void trim(ID channelId, ID waveId, int a, int b)
{
	m::wfx::trim(waveId, a, b);
	updateWavePtr_(channelId, waveId);
	resetBeginEnd_(channelId);
}


/* -------------------------------------------------------------------------- */


/* TODO - this arcane logic of keeping previewTracker_ will go away as soon as
the One-shot pause mode is implemented: 
	https://github.com/monocasual/giada/issues/88 */

void playPreview(bool loop)
{	
	setPreviewTracker(previewTracker_);
	channel::setSamplePlayerMode(m::mixer::PREVIEW_CHANNEL_ID, loop ? SamplePlayerMode::SINGLE_ENDLESS : SamplePlayerMode::SINGLE_BASIC);
	events::pressChannel(m::mixer::PREVIEW_CHANNEL_ID, G_MAX_VELOCITY, Thread::MAIN);
}


void stopPreview()
{
	/* Let the Sample Editor show the initial tracker position, then kill the
	channel. */
	setPreviewTracker(previewTracker_);
	getSampleEditorWindow()->refresh();
	events::killChannel(m::mixer::PREVIEW_CHANNEL_ID, Thread::MAIN);
}


void setPreviewTracker(Frame f)
{
	namespace mm = m::model;

	mm::onGet(mm::channels, m::mixer::PREVIEW_CHANNEL_ID, [&](m::Channel& c)
	{
		c.samplePlayer->state->tracker.store(f);
	});

	previewTracker_ = f;

	getSampleEditorWindow()->refresh();
}


void cleanupPreview()
{
	namespace mm = m::model;

	mm::ChannelsLock cl(mm::channels);
	mm::get(mm::channels, m::mixer::PREVIEW_CHANNEL_ID).samplePlayer->loadWave(nullptr);
}


/* -------------------------------------------------------------------------- */


void toNewChannel(ID channelId, ID waveId, int a, int b)
{
	ID columnId = G_MainWin->keyboard->getChannel(channelId)->getColumnId();

	m::model::onGet(m::model::waves, waveId, [&](m::Wave& w)
	{
		m::mh::addAndLoadChannel(columnId, m::waveManager::createFromWave(w, a, b));
	});
}


/* -------------------------------------------------------------------------- */


bool isWaveBufferFull()
{
	return waveBuffer_ != nullptr;
}


/* -------------------------------------------------------------------------- */


void reload(ID channelId, ID waveId)
{
	if (!v::gdConfirmWin("Warning", "Reload sample: are you sure?"))
		return;

	std::string wavePath;
	m::model::onGet(m::model::waves, waveId, [&](const m::Wave& w)
	{
		wavePath = w.getPath();
	});

	if (channel::loadChannel(channelId, wavePath) != G_RES_OK) {
		v::gdAlert("Unable to reload sample!");
		return;
	}

	getSampleEditorWindow()->rebuild();
}


/* -------------------------------------------------------------------------- */


void shift(ID channelId, ID waveId, int offset)
{
	Frame shift;
	m::model::onGet(m::model::channels, channelId, [&](const m::Channel& c)
	{
		shift = c.samplePlayer->state->shift.load();
	});
	
	m::wfx::shift(waveId, offset - shift);
	updateWavePtr_(channelId, waveId);

	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		c.samplePlayer->state->shift.store(offset);
	});

	getSampleEditorWindow()->shiftTool->update(offset);
}
}}} // giada::c::sampleEditor::
