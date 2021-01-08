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


namespace giada::c::sampleEditor
{
namespace
{
m::channel::Data& getChannel_(ID channelId)
{
	return m::model::get().getChannel(channelId);
}


m::samplePlayer::Data& getSamplePlayer_(ID channelId)
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

Frame previewTracker_ = 0;


/* -------------------------------------------------------------------------- */

/* resetBeginEnd_
Resets begin/end points to 0/max. */

void resetBeginEnd_(ID channelId)
{
	Frame begin = getSamplePlayer_(channelId).begin;
	Frame end   = getSamplePlayer_(channelId).getWaveSize();
	setBeginEnd(channelId, begin, end);
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Data::Data(const m::channel::Data& c)
: channelId   (c.id)
, name        (c.name)
, volume      (c.volume)
, pan         (c.pan)
, pitch       (c.samplePlayer->pitch)
, begin       (c.samplePlayer->begin)
, end         (c.samplePlayer->end)
, shift       (c.samplePlayer->shift)
, waveSize    (c.samplePlayer->getWave()->getSize())
, waveBits    (c.samplePlayer->getWave()->getBits())
, waveDuration(c.samplePlayer->getWave()->getDuration())
, waveRate    (c.samplePlayer->getWave()->getRate())
, wavePath    (c.samplePlayer->getWave()->getPath())
, isLogical   (c.samplePlayer->getWave()->isLogical())
, m_channel   (&c)
{
}


ChannelStatus Data::a_getPreviewStatus() const
{
	return getChannel_(m::mixer::PREVIEW_CHANNEL_ID).state->playStatus.load();
}


Frame Data::a_getPreviewTracker() const
{
	return getChannel_(m::mixer::PREVIEW_CHANNEL_ID).state->tracker.load();
}


const m::Wave& Data::getWaveRef() const
{
	return *m_channel->samplePlayer->getWave();
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Data getData(ID channelId)
{
	/* Prepare the preview channel first, then return Data object. */
	m::samplePlayer::loadWave(getChannel_(m::mixer::PREVIEW_CHANNEL_ID), &getWave_(channelId));
    m::model::swap(m::model::SwapType::SOFT);

	return Data(getChannel_(channelId));
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


void setBeginEnd(ID channelId, Frame b, Frame e)
{
	m::channel::Data& c = getChannel_(channelId);
	
	b = std::clamp(b, 0, c.samplePlayer->getWaveSize() - 1);
	e = std::clamp(e, 1, c.samplePlayer->getWaveSize() - 1);
	if      (b >= e) b = e - 1;
	else if (e < b)  e = b + 1;

	if (c.state->tracker.load() < b)
		c.state->tracker.store(b);

	getSamplePlayer_(channelId).begin = b;
	getSamplePlayer_(channelId).end   = e;
    m::model::swap(m::model::SwapType::SOFT);

	/* TODO waveform widget is dumb and wants a rebuild. Refactoring needed! */
	getSampleEditorWindow()->rebuild();
}


/* -------------------------------------------------------------------------- */


void cut(ID channelId, Frame a, Frame b)
{
	copy(channelId, a, b);
    m::model::DataLock lock;
	m::wfx::cut(getWave_(channelId), a, b);
	resetBeginEnd_(channelId);
}


/* -------------------------------------------------------------------------- */


void copy(ID channelId, Frame a, Frame b)
{
	waveBuffer_ = m::waveManager::createFromWave(getWave_(channelId), a, b);
}


/* -------------------------------------------------------------------------- */


void paste(ID channelId, Frame a)
{
	if (!isWaveBufferFull()) {
		u::log::print("[sampleEditor::paste] Buffer is empty, nothing to paste\n");
		return;
	}

	/* Get the existing wave in channel. */

	m::Wave& wave = getWave_(channelId);

	/* Temporary disable wave reading in channel. From now on, the audio thread
	won't be reading any wave, so editing it is safe.  */

	m::model::DataLock lock;

	/* Paste copied data to destination wave. */

	m::wfx::paste(*waveBuffer_, wave, a);

	/* Pass the old wave that contains the pasted data to channel. */

	m::samplePlayer::setWave(getChannel_(channelId), &wave, 1.0f);

	/* In the meantime, shift begin/end points to keep the previous position. */

	int   delta = waveBuffer_->getSize();
	Frame begin = getSamplePlayer_(channelId).begin;
	Frame end   = getSamplePlayer_(channelId).end;

	if (a < begin && a < end)
		setBeginEnd(channelId, begin + delta, end + delta);
	else
	if (a < end)
		setBeginEnd(channelId, begin, end + delta);

	getSampleEditorWindow()->rebuild();
}


/* -------------------------------------------------------------------------- */


void silence(ID channelId, int a, int b)
{
    m::model::DataLock lock;
	m::wfx::silence(getWave_(channelId), a, b);
}


/* -------------------------------------------------------------------------- */


void fade(ID channelId, int a, int b, m::wfx::Fade type)
{
    m::model::DataLock lock;
	m::wfx::fade(getWave_(channelId), a, b, type);
}


/* -------------------------------------------------------------------------- */


void smoothEdges(ID channelId, int a, int b)
{
    m::model::DataLock lock;
	m::wfx::smooth(getWave_(channelId), a, b);
}


/* -------------------------------------------------------------------------- */


void reverse(ID channelId, Frame a, Frame b)
{
    m::model::DataLock lock;
	m::wfx::reverse(getWave_(channelId), a, b);
}


/* -------------------------------------------------------------------------- */


void normalize(ID channelId, int a, int b)
{
    m::model::DataLock lock;
    m::wfx::normalize(getWave_(channelId), a, b);
}


/* -------------------------------------------------------------------------- */


void trim(ID channelId, int a, int b)
{
    m::model::DataLock lock;
	m::wfx::trim(getWave_(channelId), a, b);
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

	mm::get().getChannel(m::mixer::PREVIEW_CHANNEL_ID).state->tracker.store(f);
    mm::swap(mm::SwapType::SOFT);

	previewTracker_ = f;

	getSampleEditorWindow()->refresh();
}


void cleanupPreview()
{
	namespace mm = m::model;

    m::samplePlayer::loadWave(mm::get().getChannel(m::mixer::PREVIEW_CHANNEL_ID), nullptr);
    mm::swap(mm::SwapType::SOFT);
}


/* -------------------------------------------------------------------------- */


void toNewChannel(ID channelId, Frame a, Frame b)
{
	ID columnId = G_MainWin->keyboard->getChannel(channelId)->getColumnId();
	m::mh::addAndLoadChannel(columnId, m::waveManager::createFromWave(getWave_(channelId), a, b));
}


/* -------------------------------------------------------------------------- */


bool isWaveBufferFull()
{
	return waveBuffer_ != nullptr;
}


/* -------------------------------------------------------------------------- */


void reload(ID channelId)
{
	if (!v::gdConfirmWin("Warning", "Reload sample: are you sure?"))
		return;

	if (channel::loadChannel(channelId, getWave_(channelId).getPath()) != G_RES_OK) {
		v::gdAlert("Unable to reload sample!");
		return;
	}

	getSampleEditorWindow()->rebuild();
}


/* -------------------------------------------------------------------------- */


void shift(ID channelId, Frame offset)
{
	namespace mm = m::model;

	Frame shift = getSamplePlayer_(channelId).shift;

	mm::DataLock lock();
	m::wfx::shift(getWave_(channelId), offset - shift);
    getSamplePlayer_(channelId).shift = offset;

	getSampleEditorWindow()->shiftTool->update(offset);
}
} // giada::c::sampleEditor::
