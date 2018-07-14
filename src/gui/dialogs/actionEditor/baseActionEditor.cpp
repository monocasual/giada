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
#include <string>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "../../../utils/gui.h"
#include "../../../utils/string.h"
#include "../../../core/conf.h"
#include "../../../core/const.h"
#include "../../../core/clock.h"
#include "../../../core/channel.h"
#include "../../elems/actionEditor/gridTool.h"
#include "../../elems/basics/scroll.h"
#include "../../elems/basics/choice.h"
#include "baseActionEditor.h"


using std::string;


namespace giada {
namespace v
{
gdBaseActionEditor::gdBaseActionEditor(Channel* ch)
:	gdWindow (640, 284),
	ch       (ch),
	ratio    (G_DEFAULT_ZOOM_RATIO)
{
	using namespace giada::m;

	if (conf::actionEditorW) {
		resize(conf::actionEditorX, conf::actionEditorY, conf::actionEditorW, conf::actionEditorH);
		ratio = conf::actionEditorZoom;
	}
}


/* -------------------------------------------------------------------------- */


gdBaseActionEditor::~gdBaseActionEditor()
{
	using namespace giada::m;

	conf::actionEditorX = x();
	conf::actionEditorY = y();
	conf::actionEditorW = w();
	conf::actionEditorH = h();
	conf::actionEditorZoom = ratio;
}


/* -------------------------------------------------------------------------- */


void gdBaseActionEditor::cb_zoomIn(Fl_Widget *w, void *p)  { ((gdBaseActionEditor*)p)->zoomIn(); }
void gdBaseActionEditor::cb_zoomOut(Fl_Widget *w, void *p) { ((gdBaseActionEditor*)p)->zoomOut(); }


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
	return snap ? gridTool->getSnapFrame(p * ratio) : p * ratio;
}


/* -------------------------------------------------------------------------- */


void gdBaseActionEditor::zoomIn()
{
	if (ratio / 2 > MIN_RATIO) {
		ratio /= 2;
		rebuild();
		centerViewportIn();
		redraw();
	}
	else
		ratio = MIN_RATIO;
}


/* -------------------------------------------------------------------------- */


void gdBaseActionEditor::zoomOut()
{
	if (ratio * 2 < MAX_RATIO) {
		ratio *= 2;
		rebuild();
		centerViewportOut();
		redraw();
	}
	else
		ratio = MAX_RATIO;
}


/* -------------------------------------------------------------------------- */


void gdBaseActionEditor::centerViewportIn()
{
	Pixel sx = Fl::event_x() + (viewport->xposition() * 2);
	viewport->scroll_to(sx, viewport->yposition());	
}


void gdBaseActionEditor::centerViewportOut()
{
	Pixel sx = -((Fl::event_x() + viewport->xposition()) / 2) + viewport->xposition();
	if (sx < 0) sx = 0;
	viewport->scroll_to(sx, viewport->yposition());
}


/* -------------------------------------------------------------------------- */


int gdBaseActionEditor::getActionType() const
{
	if (actionType->value() == 0)
		return G_ACTION_KEYPRESS;
	else
	if (actionType->value() == 1)
		return G_ACTION_KEYREL;
	else
	if (actionType->value() == 2)
		return G_ACTION_KILL;

	assert(false);
	return -1;
}


/* -------------------------------------------------------------------------- */


void gdBaseActionEditor::prepareWindow()
{
	gu_setFavicon(this);

	string l = "Action Editor";
	if (ch->name != "") l += " - " + ch->name;
	copy_label(l.c_str());

	set_non_modal();
	size_range(640, 284);
	resizable(viewport);

	show();
}


/* -------------------------------------------------------------------------- */


int gdBaseActionEditor::handle(int e)
{
	switch (e) {
		case FL_MOUSEWHEEL:
			Fl::event_dy() == -1 ? zoomIn() : zoomOut();
			return 1;
		default:
			return Fl_Group::handle(e);
	}
}
}} // giada::v::