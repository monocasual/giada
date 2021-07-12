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

#include "baseActionEditor.h"
#include "core/action.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/graphics.h"
#include "glue/channel.h"
#include "gui/elems/actionEditor/gridTool.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/scrollPack.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <cassert>
#include <limits>
#include <string>

namespace giada::v
{
gdBaseActionEditor::gdBaseActionEditor(ID channelId, m::conf::Conf& conf)
: gdWindow(conf.actionEditorX, conf.actionEditorY, conf.actionEditorW, conf.actionEditorH)
, channelId(channelId)
, gridTool(0, 0)
, zoomInBtn(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", zoomInOff_xpm, zoomInOn_xpm)
, zoomOutBtn(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", zoomOutOff_xpm, zoomOutOn_xpm)
, ratio(conf.actionEditorZoom)
, m_barTop(0, 0, Direction::HORIZONTAL)
, m_splitScroll(0, 0, 0, 0)
, m_conf(conf)
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
	m_conf.actionEditorZoom   = ratio;
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

void gdBaseActionEditor::computeWidth()
{
	fullWidth = frameToPixel(m::clock::getFramesInSeq());
	loopWidth = frameToPixel(m::clock::getFramesInLoop());
}

/* -------------------------------------------------------------------------- */

Pixel gdBaseActionEditor::frameToPixel(Frame f) const
{
	return f / ratio;
}

Frame gdBaseActionEditor::pixelToFrame(Pixel p, bool snap) const
{
	return snap ? gridTool.getSnapFrame(p * ratio) : p * ratio;
}

/* -------------------------------------------------------------------------- */

void gdBaseActionEditor::zoomIn()
{
	const float ratioPrev = ratio;

	// Explicit type <int> to fix MINMAX macro hell on Windows
	ratio = std::max<int>(ratio / RATIO_STEP, MIN_RATIO);

	if (ratioPrev != ratio)
		centerZoom([](int pos) { return pos * RATIO_STEP; });
}

/* -------------------------------------------------------------------------- */

void gdBaseActionEditor::zoomOut()
{
	const float ratioPrev = ratio;

	// Explicit type <int> to fix MINMAX macro hell on Windows
	ratio = std::min<int>(ratio * RATIO_STEP, MAX_RATIO);

	if (ratioPrev != ratio)
		centerZoom([](int pos) { return pos / RATIO_STEP; });
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

void gdBaseActionEditor::centerZoom(std::function<int(int)> f)
{
	/* Determine how much the point under mouse has changed given the zoom
	operation (i.e. delta). */

	const int mpre = getMouseOverContent();
	rebuild();
	const int mnow = f(getMouseOverContent());

	/* Add that delta to the current scroll position, then redraw to apply. */

	m_splitScroll.setScrollX(m_splitScroll.getScrollX() + (mnow - mpre));
	redraw();
}
} // namespace giada::v