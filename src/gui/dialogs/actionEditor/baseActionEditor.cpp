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

#include "gui/dialogs/actionEditor/baseActionEditor.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/graphics.h"
#include "glue/channel.h"
#include "gui/drawing.h"
#include "gui/elems/actionEditor/gridTool.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/scrollPack.h"
#include "src/core/actions/action.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <cassert>
#include <limits>
#include <string>

namespace giada::v
{
gdBaseActionEditor::gdBaseActionEditor(ID channelId, m::Conf::Data& conf, Frame framesInBeat)
: gdWindow(conf.actionEditorX, conf.actionEditorY, conf.actionEditorW, conf.actionEditorH)
, channelId(channelId)
, gridTool(0, 0, conf, framesInBeat)
, zoomInBtn(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", zoomInOff_xpm, zoomInOn_xpm)
, zoomOutBtn(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", zoomOutOff_xpm, zoomOutOn_xpm)
, m_barTop(0, 0, Direction::HORIZONTAL)
, m_splitScroll(0, 0, 0, 0)
, m_conf(conf)
, m_ratio(conf.actionEditorZoom)
{
	end();

	m_barTop.position(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN);

	m_splitScroll.resize(
	    G_GUI_OUTER_MARGIN,
	    (G_GUI_OUTER_MARGIN * 2) + 20,
	    w() - G_GUI_OUTER_MARGIN * 2,
	    h() - (G_GUI_OUTER_MARGIN * 3) - 20);

	zoomInBtn.callback(cb_zoomIn, this);
	zoomInBtn.copy_tooltip("Zoom in");
	zoomOutBtn.callback(cb_zoomOut, this);
	zoomOutBtn.copy_tooltip("Zoom out");

	add(m_barTop);
	add(m_splitScroll);
}

/* -------------------------------------------------------------------------- */

gdBaseActionEditor::~gdBaseActionEditor()
{
	using namespace giada::m;

	m_conf.actionEditorX      = x();
	m_conf.actionEditorY      = y();
	m_conf.actionEditorW      = w();
	m_conf.actionEditorH      = h();
	m_conf.actionEditorSplitH = m_splitScroll.getTopContentH();
	m_conf.actionEditorZoom   = m_ratio;
}

/* -------------------------------------------------------------------------- */

int gdBaseActionEditor::getMouseOverContent() const
{
	return m_splitScroll.getScrollX() + (Fl::event_x() - G_GUI_OUTER_MARGIN);
}

/* -------------------------------------------------------------------------- */

void gdBaseActionEditor::cb_zoomIn(Fl_Widget* /*w*/, void* p) { ((gdBaseActionEditor*)p)->zoomIn(); }
void gdBaseActionEditor::cb_zoomOut(Fl_Widget* /*w*/, void* p) { ((gdBaseActionEditor*)p)->zoomOut(); }

/* -------------------------------------------------------------------------- */

void gdBaseActionEditor::computeWidth(Frame framesInSeq, Frame framesInLoop)
{
	fullWidth = frameToPixel(framesInSeq);
	loopWidth = frameToPixel(framesInLoop);
}

/* -------------------------------------------------------------------------- */

Pixel gdBaseActionEditor::frameToPixel(Frame f) const
{
	return f / m_ratio;
}

Frame gdBaseActionEditor::pixelToFrame(Pixel p, bool snap) const
{
	return snap ? gridTool.getSnapFrame(p * m_ratio) : p * m_ratio;
}

/* -------------------------------------------------------------------------- */

void gdBaseActionEditor::zoomIn()
{
	// Explicit type std::max<int> to fix MINMAX macro hell on Windows
	zoomAbout([&r = m_ratio]() { return std::max<int>(r / RATIO_STEP, MIN_RATIO); });
}

/* -------------------------------------------------------------------------- */

void gdBaseActionEditor::zoomOut()
{
	// Explicit type std::max<int> to fix MINMAX macro hell on Windows
	zoomAbout([&r = m_ratio]() { return std::min<int>(r * RATIO_STEP, MAX_RATIO); });
}

/* -------------------------------------------------------------------------- */

void gdBaseActionEditor::prepareWindow()
{
	u::gui::setFavicon(this);

	std::string l = "Action Editor";
	if (m_data.channelName != "")
		l += " - " + m_data.channelName;
	copy_label(l.c_str());

	set_non_modal();
	size_range(640, 284);

	show();
}

/* -------------------------------------------------------------------------- */

int gdBaseActionEditor::handle(int e)
{
	switch (e)
	{
	case FL_MOUSEWHEEL:
		Fl::event_dy() == -1 ? zoomIn() : zoomOut();
		return 1;
	default:
		return Fl_Group::handle(e);
	}
}

/* -------------------------------------------------------------------------- */

void gdBaseActionEditor::draw()
{
	gdWindow::draw();

	const geompp::Rect splitBounds = m_splitScroll.getBoundsNoScrollbar();
	const geompp::Line playhead    = splitBounds.getHeightAsLine().withX(currentFrameToPixel());

	if (splitBounds.contains(playhead))
		drawLine(playhead, G_COLOR_LIGHT_2);
}

/* -------------------------------------------------------------------------- */

void gdBaseActionEditor::zoomAbout(std::function<float()> f)
{
	const float ratioPrev = m_ratio;
	const int   minWidth  = w() - (G_GUI_OUTER_MARGIN * 2);

	m_ratio = f();

	/* Make sure the new content width doesn't underflow the window space (i.e. 
	the minimum width allowed). */

	if (frameToPixel(m_data.framesInSeq) < minWidth)
	{
		m_ratio = m_data.framesInSeq / static_cast<float>(minWidth);
		m_splitScroll.setScrollX(0);
	}

	/* 1. Store the current x-position, then the new x-position affected by the
	zoom change. */

	const int mpre = getMouseOverContent();
	const int mnow = mpre / (m_ratio / ratioPrev);

	/* 2. Rebuild everything and adjust scrolling given the change occurred in
	the x-position. This effectively centers the view on the mouse cursor. */

	rebuild();
	m_splitScroll.setScrollX(m_splitScroll.getScrollX() + (mnow - mpre));
	redraw();
}

/* -------------------------------------------------------------------------- */

void gdBaseActionEditor::refresh()
{
	redraw();
}

/* -------------------------------------------------------------------------- */

Pixel gdBaseActionEditor::currentFrameToPixel() const
{
	return (frameToPixel(m_data.getCurrentFrame()) + m_splitScroll.x()) - m_splitScroll.getScrollX();
}
} // namespace giada::v