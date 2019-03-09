/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include "core/model/data.h"
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


m::SampleChannel* getChannel_(std::shared_ptr<m::model::Layout>&& l, ID chanID)
{
	return static_cast<m::SampleChannel*>(l->getChannel(chanID));
}


/* -------------------------------------------------------------------------- */


void onWave_(ID chanID, std::function<void(m::Wave&)> f)
{
	std::shared_ptr<m::model::Layout> layout = m::model::cloneLayout();
	m::model::Data&                   data   = m::model::getData();

	/* Create a new Wave out of the original one and modify it. */

	std::shared_ptr<m::Wave> w = std::make_shared<m::Wave>(*data.waves.at(chanID));
	f(*w.get());

	/* Push back the new Wave into Data. */	

	data.waves.at(chanID) = w;

	/* Tell Layout something has changed: the Wave pointer. */
	
	static_cast<m::SampleChannel*>(layout->getChannel(chanID))->pushWave(w);
	m::model::swapLayout(layout);
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


void setBeginEnd(ID chanID, int b, int e)
{
	getChannel_(m::model::getLayout(), chanID)->setBegin(b);
	getChannel_(m::model::getLayout(), chanID)->setEnd(e);
	
	getSampleEditorWindow()->rebuild();
}


/* -------------------------------------------------------------------------- */


void cut(ID chanID, int a, int b)
{
	copy(chanID, a, b);
	onWave_(chanID, [=](m::Wave& w) {
		m::wfx::cut(w, a, b);
	});

	/* Model has changed, needs a new Channel pointer. */
	
	const m::SampleChannel* ch = getChannel_(m::model::getLayout(), chanID);

	setBeginEnd(chanID, ch->getBegin(), ch->getEnd());
}


/* -------------------------------------------------------------------------- */


void copy(ID chanID, int a, int b)
{
	waveBuffer_ = m::waveManager::createFromWave(*getChannel_(m::model::getLayout(), chanID)->wave, a, b);
}


/* -------------------------------------------------------------------------- */


void paste(ID chanID, int a)
{
	if (!isWaveBufferFull()) {
		gu_log("[sampleEditor::paste] Buffer is empty, nothing to paste\n");
		return;
	}

	onWave_(chanID, [=](m::Wave& w) {
		m::wfx::paste(*waveBuffer_, w, a);
	});	

	/* Shift begin/end points to keep the previous position. */

	int begin = getChannel_(m::model::getLayout(), chanID)->getBegin();
	int end   = getChannel_(m::model::getLayout(), chanID)->getEnd();
	int delta = waveBuffer_->getSize();

	if (a < begin && a < end)
		setBeginEnd(chanID, begin + delta, end + delta);
	else
	if (a < end)
		setBeginEnd(chanID, begin, end + delta);

	getSampleEditorWindow()->rebuild();
}

/* -------------------------------------------------------------------------- */


void silence(ID chanID, int a, int b)
{
	onWave_(chanID, [=](m::Wave& w) {
		m::wfx::silence(w, a, b);
	});
}


/* -------------------------------------------------------------------------- */


void fade(ID chanID, int a, int b, int type)
{
	onWave_(chanID, [=](m::Wave& w) {
		m::wfx::fade(w, a, b, type);
	});
}


/* -------------------------------------------------------------------------- */


void smoothEdges(ID chanID, int a, int b)
{
	onWave_(chanID, [=](m::Wave& w) {
		m::wfx::smooth(w, a, b);
	});
}


/* -------------------------------------------------------------------------- */


void reverse(ID chanID, int a, int b)
{
	onWave_(chanID, [=](m::Wave& w) {
		m::wfx::reverse(w, a, b);
	});
}


/* -------------------------------------------------------------------------- */


void normalizeHard(ID chanID, int a, int b)
{
	onWave_(chanID, [=](m::Wave& w) {
		m::wfx::normalizeHard(w, a, b);
	});
}


/* -------------------------------------------------------------------------- */


void trim(ID chanID, int a, int b)
{
	onWave_(chanID, [=](m::Wave& w) {
		m::wfx::trim(w, a, b);
	});
	
	/* Model has changed, needs a new Channel pointer. */
	
	const m::SampleChannel* ch = getChannel_(m::model::getLayout(), chanID);

	setBeginEnd(chanID, ch->getBegin(), ch->getEnd());
}


/* -------------------------------------------------------------------------- */


void setPlayHead(ID chanID, Frame f)
{
	getChannel_(m::model::getLayout(), chanID)->trackerPreview.store(f);
	getSampleEditorWindow()->refresh();
}


/* -------------------------------------------------------------------------- */


void setPreview(ID chanID, PreviewMode mode)
{
	getChannel_(m::model::getLayout(), chanID)->previewMode.store(mode);
}


/* -------------------------------------------------------------------------- */


void rewindPreview(ID chanID)
{
	const v::geWaveform* waveform = getSampleEditorWindow()->waveTools->waveform;
	
	if (waveform->isSelected() && getChannel_(m::model::getLayout(), chanID)->trackerPreview.load() != waveform->getSelectionA())
		setPlayHead(chanID, waveform->getSelectionA());
	else
		setPlayHead(chanID, 0);
}


/* -------------------------------------------------------------------------- */


void toNewChannel(ID chanID, int a, int b)
{
	const size_t   colIndex = G_MainWin->keyboard->getChannel(chanID)->getColumnIndex();
	const m::Wave* wave     = getChannel_(m::model::getLayout(), chanID)->wave.get();

	m::mh::addAndLoadChannel(colIndex, m::waveManager::createFromWave(*wave, a, b));
}


/* -------------------------------------------------------------------------- */


bool isWaveBufferFull()
{
	return waveBuffer_ != nullptr;
}


/* -------------------------------------------------------------------------- */


void reload(ID chanID)
{
	if (!v::gdConfirmWin("Warning", "Reload sample: are you sure?"))
		return;

	m::SampleChannel* ch = getChannel_(m::model::getLayout(), chanID);

	if (channel::loadChannel(ch->id, ch->wave->getPath()) != G_RES_OK)
		return;

	/* Model has changed, needs a new Channel pointer. */
	
	ch = getChannel_(m::model::getLayout(), chanID);

	ch->setBoost(G_DEFAULT_BOOST);
	ch->setPitch(G_DEFAULT_PITCH);
	ch->setPan(0.5f);
	ch->setBegin(0);
	ch->setEnd(ch->wave->getSize());

	getSampleEditorWindow()->rebuild();
}


/* -------------------------------------------------------------------------- */


void shift(ID chanID, int offset)
{
	m::SampleChannel* ch = static_cast<m::SampleChannel*>(getChannel_(m::model::getLayout(), chanID));
	
	onWave_(chanID, [=](m::Wave& w) {
		m::wfx::shift(w, offset - ch->shift);
	});
	
	ch->shift = offset;
}


}}}; // giada::c::sampleEditor::
