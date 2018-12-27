/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/sampleEditor.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../gui/elems/basics/button.h"
#include "../gui/elems/sampleEditor/waveTools.h"
#include "../gui/elems/sampleEditor/volumeTool.h"
#include "../gui/elems/sampleEditor/boostTool.h"
#include "../gui/elems/sampleEditor/panTool.h"
#include "../gui/elems/sampleEditor/pitchTool.h"
#include "../gui/elems/sampleEditor/rangeTool.h"
#include "../gui/elems/sampleEditor/shiftTool.h"
#include "../gui/elems/sampleEditor/waveform.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../core/sampleChannel.h"
#include "../core/waveFx.h"
#include "../core/wave.h"
#include "../core/waveManager.h"
#include "../core/const.h"
#include "../utils/gui.h"
#include "../utils/log.h"
#include "channel.h"
#include "sampleEditor.h"


extern gdMainWindow* G_MainWin;


namespace giada {
namespace c     {
namespace sampleEditor
{
namespace
{
	/* waveBuffer
	A Wave used during cut/copy/paste operations. */

	std::unique_ptr<Wave> waveBuffer_;
}; // {anonymous}


/* -------------------------------------------------------------------------- */


gdSampleEditor* getSampleEditorWindow()
{
	gdSampleEditor* se = static_cast<gdSampleEditor*>(gu_getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	assert(se != nullptr);
	return se;
}


/* -------------------------------------------------------------------------- */


void setBeginEnd(m::SampleChannel* ch, int b, int e)
{
	ch->setBegin(b);
	ch->setEnd(e);
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	Fl::lock();
	gdEditor->rangeTool->refresh();
	Fl::unlock();

	gdEditor->waveTools->waveform->recalcPoints();
	gdEditor->waveTools->waveform->clearSel();
	gdEditor->waveTools->waveform->redraw();
}


/* -------------------------------------------------------------------------- */


void cut(m::SampleChannel* ch, int a, int b)
{
	copy(ch, a, b);
	if (!m::wfx::cut(*ch->wave, a, b)) {
		gdAlert("Unable to cut the sample!");
		return;
	}
	setBeginEnd(ch, ch->getBegin(), ch->getEnd());
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->clearSel();
	gdEditor->waveTools->waveform->refresh();
	gdEditor->updateInfo();
}


/* -------------------------------------------------------------------------- */


void copy(m::SampleChannel* ch, int a, int b)
{
	waveBuffer_ = m::waveManager::createFromWave(ch->wave.get(), a, b);
}


/* -------------------------------------------------------------------------- */


void paste(m::SampleChannel* ch, int a)
{
	if (!isWaveBufferFull()) {
		gu_log("[sampleEditor::paste] Buffer is empty, nothing to paste\n");
		return;
	}
	
	m::wfx::paste(*waveBuffer_.get(), *ch->wave.get(), a);

	/* Shift begin/end points to keep the previous position. */

	int delta = waveBuffer_->getSize();
	if (a < ch->getBegin() && a < ch->getEnd())
		setBeginEnd(ch, ch->getBegin() + delta, ch->getEnd() + delta);
	else
	if (a < ch->getEnd())
		setBeginEnd(ch, ch->getBegin(), ch->getEnd() + delta);

	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->clearSel();
	gdEditor->waveTools->waveform->refresh();
	gdEditor->updateInfo();
}

/* -------------------------------------------------------------------------- */


void silence(m::SampleChannel* ch, int a, int b)
{
	m::wfx::silence(*ch->wave, a, b);
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->refresh();
}


/* -------------------------------------------------------------------------- */


void fade(m::SampleChannel* ch, int a, int b, int type)
{
	m::wfx::fade(*ch->wave, a, b, type);
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->refresh();
}


/* -------------------------------------------------------------------------- */


void smoothEdges(m::SampleChannel* ch, int a, int b)
{
	m::wfx::smooth(*ch->wave, a, b);
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->refresh();
}


/* -------------------------------------------------------------------------- */


void reverse(m::SampleChannel* ch, int a, int b)
{
	m::wfx::reverse(*ch->wave, a, b);
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->refresh();
}


/* -------------------------------------------------------------------------- */


void normalizeHard(m::SampleChannel* ch, int a, int b)
{
	m::wfx::normalizeHard(*ch->wave, a, b);
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->refresh();
}


/* -------------------------------------------------------------------------- */


void trim(m::SampleChannel* ch, int a, int b)
{
	if (!m::wfx::trim(*ch->wave, a, b)) {
		gdAlert("Unable to trim the sample!");
		return;
	}
	setBeginEnd(ch, ch->getBegin(), ch->getEnd());
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->clearSel();
	gdEditor->waveTools->waveform->refresh();
	gdEditor->updateInfo();
}


/* -------------------------------------------------------------------------- */


void setPlayHead(m::SampleChannel* ch, int f)
{
	ch->trackerPreview = f;
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->waveTools->waveform->redraw();
}


/* -------------------------------------------------------------------------- */


void setPreview(m::SampleChannel* ch, PreviewMode mode)
{
	ch->previewMode = mode;
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->play->value(!gdEditor->play->value());
}


/* -------------------------------------------------------------------------- */


void rewindPreview(m::SampleChannel* ch)
{
	geWaveform* waveform = getSampleEditorWindow()->waveTools->waveform;
	if (waveform->isSelected() && ch->trackerPreview != waveform->getSelectionA())
		setPlayHead(ch, waveform->getSelectionA());
	else
		setPlayHead(ch, 0);
}


/* -------------------------------------------------------------------------- */


void toNewChannel(m::SampleChannel* ch, int a, int b)
{
	m::SampleChannel* newCh = static_cast<m::SampleChannel*>(c::channel::addChannel(
		ch->guiChannel->getColumnIndex(), ChannelType::SAMPLE, G_GUI_CHANNEL_H_1));

	newCh->pushWave(m::waveManager::createFromWave(ch->wave.get(), a, b));
	newCh->guiChannel->update();
}


/* -------------------------------------------------------------------------- */


bool isWaveBufferFull()
{
	return waveBuffer_ != nullptr;
}


/* -------------------------------------------------------------------------- */


void shift(m::SampleChannel* ch, int offset)
{
	m::wfx::shift(*ch->wave, offset - ch->shift);
	ch->shift = offset;
	gdSampleEditor* gdEditor = getSampleEditorWindow();
	gdEditor->shiftTool->refresh();
	gdEditor->waveTools->waveform->refresh();	
}


}}}; // giada::c::sampleEditor::
