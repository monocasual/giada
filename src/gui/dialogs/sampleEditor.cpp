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

#include "glue/sampleEditor.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/graphics.h"
#include "core/mixer.h"
#include "core/wave.h"
#include "core/waveFx.h"
#include "glue/channel.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/group.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/pack.h"
#include "gui/elems/basics/statusButton.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/sampleEditor/boostTool.h"
#include "gui/elems/sampleEditor/panTool.h"
#include "gui/elems/sampleEditor/pitchTool.h"
#include "gui/elems/sampleEditor/rangeTool.h"
#include "gui/elems/sampleEditor/shiftTool.h"
#include "gui/elems/sampleEditor/volumeTool.h"
#include "gui/elems/sampleEditor/waveTools.h"
#include "gui/elems/sampleEditor/waveform.h"
#include "sampleEditor.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <cassert>
#include <cmath>

#ifdef G_OS_WINDOWS
#undef IN
#undef OUT
#endif

namespace giada::v
{
gdSampleEditor::gdSampleEditor(ID channelId, m::Conf::Data& c)
: gdWindow(c.sampleEditorX, c.sampleEditorY, c.sampleEditorW, c.sampleEditorH)
, m_channelId(channelId)
, m_conf(c)
{
	end();

	gePack* upperBar = createUpperBar();

	waveTools = new geWaveTools(G_GUI_OUTER_MARGIN, upperBar->y() + upperBar->h() + G_GUI_OUTER_MARGIN,
	    w() - 16, h() - 168, m_conf.sampleEditorGridOn, m_conf.sampleEditorGridVal);

	gePack* bottomBar = createBottomBar(G_GUI_OUTER_MARGIN, waveTools->y() + waveTools->h() + G_GUI_OUTER_MARGIN,
	    h() - waveTools->h() - upperBar->h() - 32);

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
	m_conf.sampleEditorX       = x();
	m_conf.sampleEditorY       = y();
	m_conf.sampleEditorW       = w();
	m_conf.sampleEditorH       = h();
	m_conf.sampleEditorGridVal = atoi(grid->text());
	m_conf.sampleEditorGridOn  = snap->value();

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

gePack* gdSampleEditor::createUpperBar()
{
	reload  = new geButton(0, 0, 70, G_GUI_UNIT, "Reload");
	grid    = new geChoice(0, 0, 50, G_GUI_UNIT);
	snap    = new geCheck(0, 0, 12, G_GUI_UNIT, "Snap");
	sep1    = new geBox(0, 0, w() - 208, G_GUI_UNIT);
	zoomOut = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", zoomOutOff_xpm, zoomOutOn_xpm);
	zoomIn  = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", zoomInOff_xpm, zoomInOn_xpm);

	reload->callback(cb_reload, (void*)this);

	grid->add("(off)");
	grid->add("2");
	grid->add("3");
	grid->add("4");
	grid->add("6");
	grid->add("8");
	grid->add("16");
	grid->add("32");
	grid->add("64");
	grid->copy_tooltip("Grid frequency");

	if (m_conf.sampleEditorGridVal == 0)
		grid->value(0);
	else
		grid->value(grid->find_item(u::string::iToString(m_conf.sampleEditorGridVal).c_str()));
	grid->callback(cb_changeGrid, (void*)this);

	snap->value(m_conf.sampleEditorGridOn);
	snap->copy_tooltip("Snap to grid");
	snap->callback(cb_enableSnap, (void*)this);

	/* TODO - redraw grid if != (off) */

	zoomOut->callback(cb_zoomOut, (void*)this);
	zoomOut->copy_tooltip("Zoom out");
	zoomIn->callback(cb_zoomIn, (void*)this);
	zoomIn->copy_tooltip("Zoom in");

	gePack* g = new gePack(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, Direction::HORIZONTAL);
	g->add(reload);
	g->add(grid);
	g->add(snap);
	g->add(sep1);
	g->add(zoomOut);
	g->add(zoomIn);
	g->resizable(sep1);

	return g;
}

/* -------------------------------------------------------------------------- */

gePack* gdSampleEditor::createOpTools(int x, int y)
{
	volumeTool = new geVolumeTool(m_data, 0, 0);
	panTool    = new gePanTool(m_data, 0, 0);
	pitchTool  = new gePitchTool(m_data, 0, 0);
	rangeTool  = new geRangeTool(m_data, 0, 0);
	shiftTool  = new geShiftTool(m_data, 0, 0);

	gePack* g = new gePack(x, y, Direction::VERTICAL);
	g->add(volumeTool);
	g->add(panTool);
	g->add(pitchTool);
	g->add(rangeTool);
	g->add(shiftTool);

	return g;
}

/* -------------------------------------------------------------------------- */

geGroup* gdSampleEditor::createPreviewBox(int x, int y, int h)
{
	rewind = new geButton(x, y + (h / 2) - 12, 25, 25, "", rewindOff_xpm, rewindOn_xpm);
	play   = new geStatusButton(rewind->x() + rewind->w() + 4, rewind->y(), 25, 25, play_xpm, pause_xpm);
	loop   = new geCheck(play->x() + play->w() + 4, play->y(), 50, 25, "Loop");

	play->callback(cb_togglePreview, (void*)this);
	rewind->callback(cb_rewindPreview, (void*)this);

	geGroup* g = new geGroup(x, y);
	g->add(rewind);
	g->add(play);
	g->add(loop);

	return g;
}

/* -------------------------------------------------------------------------- */

gePack* gdSampleEditor::createBottomBar(int x, int y, int h)
{
	geGroup*  previewBox = createPreviewBox(0, 0, h);
	geBox*    divisor1   = new geBox(0, 0, 1, h);
	Fl_Group* opTools    = createOpTools(0, 0);
	geBox*    divisor2   = new geBox(0, 0, 1, h);
	info                 = new geBox(0, 0, 400, h);

	divisor1->box(FL_BORDER_BOX);
	divisor2->box(FL_BORDER_BOX);

	info->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_TOP);

	gePack* g = new gePack(x, y, Direction::HORIZONTAL, /*gutter=*/G_GUI_OUTER_MARGIN);
	g->add(previewBox);
	g->add(divisor1);
	g->add(opTools);
	g->add(divisor2);
	g->add(info);
	g->resizable(0);

	return g;
}

/* -------------------------------------------------------------------------- */

void gdSampleEditor::cb_reload(Fl_Widget* /*w*/, void* p) { ((gdSampleEditor*)p)->cb_reload(); }
void gdSampleEditor::cb_zoomIn(Fl_Widget* /*w*/, void* p) { ((gdSampleEditor*)p)->cb_zoomIn(); }
void gdSampleEditor::cb_zoomOut(Fl_Widget* /*w*/, void* p) { ((gdSampleEditor*)p)->cb_zoomOut(); }
void gdSampleEditor::cb_changeGrid(Fl_Widget* /*w*/, void* p) { ((gdSampleEditor*)p)->cb_changeGrid(); }
void gdSampleEditor::cb_enableSnap(Fl_Widget* /*w*/, void* p) { ((gdSampleEditor*)p)->cb_enableSnap(); }
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
	c::sampleEditor::reload(m_data.channelId);
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
	    "File: " + m_data.wavePath + "\n"
	                                 "Size: " +
	    u::string::iToString(m_data.waveSize) + " frames\n"
	                                            "Duration: " +
	    u::string::iToString(m_data.waveDuration) + " seconds\n"
	                                                "Bit depth: " +
	    bitDepth + "\n"
	               "Frequency: " +
	    u::string::iToString(m_data.waveRate) + " Hz\n";

	info->copy_label(infoText.c_str());
}
} // namespace giada::v
