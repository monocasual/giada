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


#include <cmath>
#include <cassert>
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include "glue/channel.h"
#include "glue/sampleEditor.h"
#include "core/model/model.h"
#include "core/channels/sampleChannel.h"
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
gdSampleEditor::gdSampleEditor(ID channelId, ID waveId)
: gdWindow   (m::conf::sampleEditorX, m::conf::sampleEditorY, 
	          m::conf::sampleEditorW, m::conf::sampleEditorH),
  m_channelId(channelId),
  m_waveId   (waveId)
{
	Fl_Group* upperBar = createUpperBar();
	
	waveTools = new geWaveTools(channelId, waveId, G_GUI_OUTER_MARGIN, upperBar->y()+upperBar->h()+G_GUI_OUTER_MARGIN, 
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
	m::conf::sampleEditorX = x();
	m::conf::sampleEditorY = y();
	m::conf::sampleEditorW = w();
	m::conf::sampleEditorH = h();
	m::conf::sampleEditorGridVal = atoi(grid->text());
	m::conf::sampleEditorGridOn  = snap->value();
	
	c::sampleEditor::setPreview(m_channelId, PreviewMode::NONE);
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::rebuild()
{
	m::model::onGet(m::model::channels, m_channelId, [&](m::Channel& c)
	{
		copy_label(c.name.c_str());
	});
	
	volumeTool->rebuild();
	waveTools->rebuild();
	panTool->rebuild();
	pitchTool->rebuild();
	rangeTool->rebuild();
	shiftTool->rebuild();

	m::model::onGet(m::model::waves, m_waveId, [&](m::Wave& w)
	{
		updateInfo(w);
		if (w.isLogical()) // Logical samples (aka takes) cannot be reloaded.
			reload->deactivate();
	});
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::refresh()
{
	waveTools->refresh();
	
	m::model::onGet(m::model::channels, m_channelId, [&](m::Channel& c)
	{
		play->setStatus(c.previewMode == PreviewMode::LOOP || c.previewMode == PreviewMode::NORMAL ? 1 : 0);
	});
}


/* -------------------------------------------------------------------------- */


Fl_Group* gdSampleEditor::createUpperBar()
{
	Fl_Group* g = new Fl_Group(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, w()-16, G_GUI_UNIT);
	g->begin();
		grid    = new geChoice(g->x(), g->y(), 50, G_GUI_UNIT);
		snap    = new geCheck(grid->x()+grid->w()+4, g->y()+3, 12, 12, "Snap");
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
	if (m::conf::sampleEditorGridVal == 0)
		grid->value(0);
	else 
		grid->value(grid->find_item(u::string::iToString(m::conf::sampleEditorGridVal).c_str()));
	grid->callback(cb_changeGrid, (void*)this);

	snap->value(m::conf::sampleEditorGridOn);
	snap->callback(cb_enableSnap, (void*)this);

	/* TODO - redraw grid if != (off) */

	zoomOut->callback(cb_zoomOut, (void*)this);
	zoomIn->callback(cb_zoomIn, (void*)this);

	return g;
}


/* -------------------------------------------------------------------------- */


Fl_Group* gdSampleEditor::createOpTools(int x, int y, int h)
{
	Fl_Group* g = new Fl_Group(x, y, 572, h);
	g->begin();
	g->resizable(0);
		volumeTool = new geVolumeTool(m_channelId, g->x(), g->y());
		panTool    = new gePanTool(m_channelId, volumeTool->x()+volumeTool->w()+4, g->y());
	 
		pitchTool = new gePitchTool(m_channelId, g->x(), panTool->y()+panTool->h()+8);

		rangeTool = new geRangeTool(m_channelId, m_waveId, g->x(), pitchTool->y()+pitchTool->h()+8);
		shiftTool = new geShiftTool(m_channelId, m_waveId, rangeTool->x()+rangeTool->w()+4, pitchTool->y()+pitchTool->h()+8);
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
		play   = new geStatusButton(rewind->x()+rewind->w()+4, g->y()+(g->h()/2)-12, 25, 25, play_xpm, pause_xpm);
		loop   = new geCheck(play->x()+play->w()+6, g->y()+(g->h()/2)-6, 12, 12, "Loop");
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


Fl_Group* gdSampleEditor::createBottomBar(int x, int y, int h)
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


void gdSampleEditor::cb_reload       (Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->cb_reload(); }
void gdSampleEditor::cb_zoomIn       (Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->cb_zoomIn(); }
void gdSampleEditor::cb_zoomOut      (Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->cb_zoomOut(); }
void gdSampleEditor::cb_changeGrid   (Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->cb_changeGrid(); }
void gdSampleEditor::cb_enableSnap   (Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->cb_enableSnap(); }
void gdSampleEditor::cb_togglePreview(Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->cb_togglePreview(); }
void gdSampleEditor::cb_rewindPreview(Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->cb_rewindPreview(); }


/* -------------------------------------------------------------------------- */


void gdSampleEditor::cb_enableSnap()
{
	waveTools->waveform->setSnap(!waveTools->waveform->getSnap());
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::cb_togglePreview()
{
	if (play->getStatus())
		c::sampleEditor::setPreview(m_channelId, PreviewMode::NONE);
	else
		c::sampleEditor::setPreview(m_channelId, loop->value() ? PreviewMode::LOOP : PreviewMode::NORMAL);
}


void gdSampleEditor::cb_rewindPreview()
{
	c::sampleEditor::rewindPreview(m_channelId);
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::cb_reload()
{
	c::sampleEditor::reload(m_channelId, m_waveId);
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


void gdSampleEditor::updateInfo(const m::Wave& w)
{
	std::string bitDepth = w.getBits() != 0 ? u::string::iToString(w.getBits()) : "(unknown)";
	std::string infoText = 
		"File: "      + w.getPath() + "\n"
		"Size: "      + u::string::iToString(w.getSize()) + " frames\n"
		"Duration: "  + u::string::iToString(w.getDuration()) + " seconds\n"
		"Bit depth: " + bitDepth + "\n"
		"Frequency: " + u::string::iToString(w.getRate()) + " Hz\n";

	info->copy_label(infoText.c_str());
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::setWaveId(ID id)
{
	m_waveId = id;
	waveTools->waveId = id;
	waveTools->waveform->setWaveId(id);
}
}} // giada::v::
