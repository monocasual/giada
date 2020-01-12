/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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
#include "core/channels/sampleChannel.h"
#include "core/waveFx.h"
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


/* -------------------------------------------------------------------------- */

/* resetBeginEnd_
Resets begin/end points when model has changed and a new Channel pointer is
needed for the operation. */

void resetBeginEnd_(ID channelId)
{
	Frame begin;
	Frame end;
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		begin = static_cast<m::SampleChannel&>(c).begin;
		end   = static_cast<m::SampleChannel&>(c).end;
	});

	setBeginEnd(channelId, begin, end);
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


v::gdSampleEditor* getSampleEditorWindow()
{
	v::gdSampleEditor* se = static_cast<v::gdSampleEditor*>(u::gui::getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	assert(se != nullptr);
	return se;
}


/* -------------------------------------------------------------------------- */


void setBeginEnd(ID channelId, int b, int e)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& c)
	{
		static_cast<m::SampleChannel&>(c).setBegin(b);
		static_cast<m::SampleChannel&>(c).setEnd(e);
	});

	getSampleEditorWindow()->rebuild();
}


/* -------------------------------------------------------------------------- */


void cut(ID channelId, ID waveId, int a, int b)
{
	copy(waveId, a, b);
	m::wfx::cut(waveId, a, b);
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

	/* Shift begin/end points to keep the previous position. */

	int   delta = waveBuffer_->getSize();
	Frame begin;
	Frame end;

	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		begin = static_cast<m::SampleChannel&>(c).begin;
		end   = static_cast<m::SampleChannel&>(c).end;
	});

	if (a < begin && a < end)
		setBeginEnd(channelId, begin + delta, end + delta);
	else
	if (a < end)
		setBeginEnd(channelId, begin, end + delta);

	getSampleEditorWindow()->rebuild();
}


/* -------------------------------------------------------------------------- */


void silence(ID waveId, int a, int b)
{
	m::wfx::silence(waveId, a, b);
}


/* -------------------------------------------------------------------------- */


void fade(ID waveId, int a, int b, int type)
{
	m::wfx::fade(waveId, a, b, type);
}


/* -------------------------------------------------------------------------- */


void smoothEdges(ID waveId, int a, int b)
{
	m::wfx::smooth(waveId, a, b);
}


/* -------------------------------------------------------------------------- */


void reverse(ID waveId, int a, int b)
{
	m::wfx::reverse(waveId, a, b);
}


/* -------------------------------------------------------------------------- */


void normalizeHard(ID waveId, int a, int b)
{
	m::wfx::normalizeHard(waveId, a, b);
}


/* -------------------------------------------------------------------------- */


void trim(ID channelId, ID waveId, int a, int b)
{
	m::wfx::trim(waveId, a, b);
	resetBeginEnd_(channelId);
}


/* -------------------------------------------------------------------------- */


void setPlayHead(ID channelId, Frame f)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		static_cast<m::SampleChannel&>(c).trackerPreview.store(f);
	});
	getSampleEditorWindow()->refresh();
}


/* -------------------------------------------------------------------------- */


void setPreview(ID channelId, PreviewMode mode)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& c)
	{
		static_cast<m::SampleChannel&>(c).previewMode = mode;
	});
}


/* -------------------------------------------------------------------------- */


void rewindPreview(ID channelId)
{
	setPlayHead(channelId, 0);
}


/* -------------------------------------------------------------------------- */


void toNewChannel(ID channelId, int a, int b)
{
	ID columnId = G_MainWin->keyboard->getChannel(channelId)->getColumnId();
	ID waveId;
	
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		waveId = static_cast<m::SampleChannel&>(c).waveId;
	});

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
	Frame       waveSize;
	m::model::onGet(m::model::waves, waveId, [&](m::Wave& w)
	{
		wavePath = w.getPath();
		waveSize = w.getSize();
	});

	if (channel::loadChannel(channelId, wavePath) != G_RES_OK)
		return;

	ID newWaveId;
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& c)
	{
		m::SampleChannel& sc = static_cast<m::SampleChannel&>(c);
		newWaveId = sc.waveId;
	});

	getSampleEditorWindow()->setWaveId(newWaveId);
	getSampleEditorWindow()->rebuild();
}


/* -------------------------------------------------------------------------- */


void shift(ID channelId, ID waveId, int offset)
{
	Frame shift;

	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		shift = static_cast<m::SampleChannel&>(c).shift;
	});
	
	m::wfx::shift(waveId, offset - shift);

	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& c)
	{
		static_cast<m::SampleChannel&>(c).shift = offset;
	});
}
}}}; // giada::c::sampleEditor::
