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


#include <cmath>
#include <cassert>
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include "glue/channel.h"
#include "glue/sampleEditor.h"
#include "core/waveFx.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/graphics.h"
#include "core/mixer.h"
#include "core/wave.h"
#include "utils/gui.h"
#include "utils/string.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/statusButton.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/sampleEditor/waveform.h"
#include "gui/elems/sampleEditor/waveTools.h"
#include "gui/elems/sampleEditor/volumeTool.h"
#include "gui/elems/sampleEditor/boostTool.h"
#include "gui/elems/sampleEditor/panTool.h"
#include "gui/elems/sampleEditor/pitchTool.h"
#include "gui/elems/sampleEditor/rangeTool.h"
#include "gui/elems/sampleEditor/shiftTool.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/dialogs/warnings.h"
#include "sampleEditor.h"


namespace giada {
namespace v 
{
gdSampleEditor::gdSampleEditor(ID channelId)
: gdWindow   (m::conf::conf.sampleEditorX, m::conf::conf.sampleEditorY, 
              m::conf::conf.sampleEditorW, m::conf::conf.sampleEditorH)
, m_channelId(channelId)
{
	Fl_Group* upperBar = createUpperBar();
	
	waveTools = new geWaveTools(G_GUI_OUTER_MARGIN, upperBar->y()+upperBar->h()+G_GUI_OUTER_MARGIN, 
		w()-16, h()-128);
	
	Fl_Group* bottomBar = createBottomBar(G_GUI_OUTER_MARGIN, waveTools->y()+waveTools->h()+G_GUI_OUTER_MARGIN, 
		h()-waveTools->h()-upperBar->h()-32);

	end();

	add(upperBar);
	add(waveTools);
	add(bottomBar);

	resizable(waveTools);

	u::gui::setFavicon(this);

	size_range(720, 480);
	set_non_modal();
	rebuild();
	show();
}


/* -------------------------------------------------------------------------- */


gdSampleEditor::~gdSampleEditor()
{
	m::conf::conf.sampleEditorX = x();
	m::conf::conf.sampleEditorY = y();
	m::conf::conf.sampleEditorW = w();
	m::conf::conf.sampleEditorH = h();
	m::conf::conf.sampleEditorGridVal = atoi(grid->text());
	m::conf::conf.sampleEditorGridOn  = snap->value();
	
	c::sampleEditor::stopPreview();
	c::sampleEditor::cleanupPreview();
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::rebuild()
{
	m_data = c::sampleEditor::getData(m_channelId);

	copy_label(m_data.name.c_str());

	waveTools->rebuild(m_data);
	volumeTool->rebuild(m_data);
	panTool->rebuild(m_data);
	pitchTool->rebuild(m_data);
	rangeTool->rebuild(m_data);
	shiftTool->rebuild(m_data);

	updateInfo();

	if (m_data.isLogical) // Logical samples (aka takes) cannot be reloaded.
		reload->deactivate();	
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::refresh()
{
	waveTools->refresh();
	play->setStatus(m_data.a_getPreviewStatus() == ChannelStatus::PLAY);
}


/* -------------------------------------------------------------------------- */


Fl_Group* gdSampleEditor::createUpperBar()
{
	Fl_Group* g = new Fl_Group(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, w()-16, G_GUI_UNIT);
	g->begin();
		grid    = new geChoice(g->x(), g->y(), 50, G_GUI_UNIT);
		snap    = new geCheck(grid->x()+grid->w()+4, g->y(), 12, G_GUI_UNIT, "Snap");
		sep1    = new geBox(snap->x()+snap->w()+4, g->y(), g->w() - 118, G_GUI_UNIT);
		zoomOut = new geButton(sep1->x()+sep1->w()+4, g->y(), G_GUI_UNIT, G_GUI_UNIT, "", zoomOutOff_xpm, zoomOutOn_xpm);
		zoomIn  = new geButton(zoomOut->x()+zoomOut->w()+4, g->y(), G_GUI_UNIT, G_GUI_UNIT, "", zoomInOff_xpm, zoomInOn_xpm);
	g->end();
	g->resizable(sep1);

	grid->add("(off)");
	grid->add("2");
	grid->add("3");
	grid->add("4");
	grid->add("6");
	grid->add("8");
	grid->add("16");
	grid->add("32");
	grid->add("64");
	if (m::conf::conf.sampleEditorGridVal == 0)
		grid->value(0);
	else 
		grid->value(grid->find_item(u::string::iToString(m::conf::conf.sampleEditorGridVal).c_str()));
	grid->callback(cb_changeGrid, (void*)this);

	snap->value(m::conf::conf.sampleEditorGridOn);
	snap->callback(cb_enableSnap, (void*)this);

	/* TODO - redraw grid if != (off) */

	zoomOut->callback(cb_zoomOut, (void*)this);
	zoomIn->callback(cb_zoomIn, (void*)this);

	return g;
}

\
/* -------------------------------------------------------------------------- */
\

Fl_Group* gdSampleEditor::createOpTools(int x, int y, int h)
{
	Fl_Group* g = new Fl_Group(x, y, 572, h);
	g->begin();
	g->resizable(0);
		volumeTool = new geVolumeTool(m_data, g->x(), g->y());
		panTool    = new gePanTool(m_data, volumeTool->x()+volumeTool->w()+4, g->y());
	 
		pitchTool = new gePitchTool(m_data, g->x(), panTool->y()+panTool->h()+8);

		rangeTool = new geRangeTool(m_data, g->x(), pitchTool->y()+pitchTool->h()+8);
		shiftTool = new geShiftTool(m_data, rangeTool->x()+rangeTool->w()+4, pitchTool->y()+pitchTool->h()+8);
		reload    = new geButton(g->x()+g->w()-70, shiftTool->y(), 70, 20, "Reload");
	g->end();

	reload->callback(cb_reload, (void*)this);

	return g;
}


/* -------------------------------------------------------------------------- */


Fl_Group* gdSampleEditor::createPreviewBox(int x, int y, int h)
{
	Fl_Group* g = new Fl_Group(x, y, 110, h);
	g->begin();
		rewind = new geButton(g->x(), g->y()+(g->h()/2)-12, 25, 25, "", rewindOff_xpm, rewindOn_xpm);
		play   = new geStatusButton(rewind->x()+rewind->w()+4, rewind->y(), 25, 25, play_xpm, pause_xpm);
		loop   = new geCheck(play->x()+play->w()+4, play->y(), 12, 25, "Loop");
	g->end();

	play->callback(cb_togglePreview, (void*)this);
	rewind->callback(cb_rewindPreview, (void*)this);

	return g;
}


/* -------------------------------------------------------------------------- */


Fl_Group* gdSampleEditor::createInfoBox(int x, int y, int h)
{
	Fl_Group* g = new Fl_Group(x, y, 400, h);
	g->begin();
		info = new geBox(g->x(), g->y(), g->w(), g->h());
	g->end();	

	info->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_TOP);

	return g;
}


/* -------------------------------------------------------------------------- */


Fl_Group* gdSampleEditor::createBottomBar(int /*x*/, int /*y*/, int h)
{
	Fl_Group* g = new Fl_Group(8, waveTools->y()+waveTools->h()+8, w()-16, h);
	g->begin();
		Fl_Group* previewBox = createPreviewBox(g->x(), g->y(), g->h());

		geBox* divisor1 = new geBox(previewBox->x()+previewBox->w()+8, g->y(), 1, g->h());
		divisor1->box(FL_BORDER_BOX);

		Fl_Group* opTools = createOpTools(divisor1->x()+divisor1->w()+12, g->y(), g->h());

		geBox* divisor2 = new geBox(opTools->x()+opTools->w()+8, g->y(), 1, g->h());
		divisor2->box(FL_BORDER_BOX);

		createInfoBox(divisor2->x()+divisor2->w()+8, g->y(), g->h());

	g->end();
	g->resizable(0);

	return g;
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::cb_reload       (Fl_Widget* /*w*/, void* p) { ((gdSampleEditor*)p)->cb_reload(); }
void gdSampleEditor::cb_zoomIn       (Fl_Widget* /*w*/, void* p) { ((gdSampleEditor*)p)->cb_zoomIn(); }
void gdSampleEditor::cb_zoomOut      (Fl_Widget* /*w*/, void* p) { ((gdSampleEditor*)p)->cb_zoomOut(); }
void gdSampleEditor::cb_changeGrid   (Fl_Widget* /*w*/, void* p) { ((gdSampleEditor*)p)->cb_changeGrid(); }
void gdSampleEditor::cb_enableSnap   (Fl_Widget* /*w*/, void* p) { ((gdSampleEditor*)p)->cb_enableSnap(); }
void gdSampleEditor::cb_togglePreview(Fl_Widget* /*w*/, void* p) { ((gdSampleEditor*)p)->cb_togglePreview(); }
void gdSampleEditor::cb_rewindPreview(Fl_Widget* /*w*/, void* p) { ((gdSampleEditor*)p)->cb_rewindPreview(); }


/* -------------------------------------------------------------------------- */


void gdSampleEditor::cb_enableSnap()
{
	waveTools->waveform->setSnap(!waveTools->waveform->getSnap());
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::cb_togglePreview()
{
	if (!play->getStatus())
		c::sampleEditor::playPreview(loop->value());
	else
		c::sampleEditor::stopPreview();
}


void gdSampleEditor::cb_rewindPreview()
{
	c::sampleEditor::setPreviewTracker(m_data.begin);
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::cb_reload()
{
	c::sampleEditor::reload(m_data.channelId, m_data.waveId);
	redraw();
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::cb_zoomIn()
{
	waveTools->waveform->setZoom(geWaveform::Zoom::IN);
	waveTools->redraw();
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::cb_zoomOut()
{
	waveTools->waveform->setZoom(geWaveform::Zoom::OUT);
	waveTools->redraw();
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::cb_changeGrid()
{
	waveTools->waveform->setGridLevel(atoi(grid->text()));
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::updateInfo()
{
	std::string bitDepth = m_data.waveBits != 0 ? u::string::iToString(m_data.waveBits) : "(unknown)";
	std::string infoText = 
		"File: "      + m_data.wavePath + "\n"
		"Size: "      + u::string::iToString(m_data.waveSize) + " frames\n"
		"Duration: "  + u::string::iToString(m_data.waveDuration) + " seconds\n"
		"Bit depth: " + bitDepth + "\n"
		"Frequency: " + u::string::iToString(m_data.waveRate) + " Hz\n";

	info->copy_label(infoText.c_str());
}
}} // giada::v::
