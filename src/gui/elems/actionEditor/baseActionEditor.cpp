/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geBaseActionEditor
 * Parent class of any widget inside the action editor.
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


#include <FL/fl_draw.H>
#include "../../../core/mixer.h"
#include "../../../core/const.h"
#include "../../dialogs/gd_actionEditor.h"
#include "gridTool.h"
#include "baseActionEditor.h"


geBaseActionEditor::geBaseActionEditor(int x, int y, int w, int h,
	gdActionEditor *pParent)
	:	Fl_Group(x, y, w, h), pParent(pParent) {}


/* -------------------------------------------------------------------------- */


geBaseActionEditor::~geBaseActionEditor() {}


/* -------------------------------------------------------------------------- */


void geBaseActionEditor::baseDraw(bool clear) {

	/* clear the screen */

	if (clear)
		fl_rectf(x(), y(), w(), h(), G_COLOR_GREY_1);

	/* draw the container */

	fl_color(G_COLOR_GREY_4);
	fl_rect(x(), y(), w(), h());

	/* grid drawing, if > 1 */

	if (pParent->gridTool->getValue() > 1) {

		fl_color(fl_rgb_color(54, 54, 54));
		fl_line_style(FL_DASH, 0, nullptr);

		for (int i=0; i<(int) pParent->gridTool->points.size(); i++) {
			int px = pParent->gridTool->points.at(i)+x()-1;
			fl_line(px, y()+1, px, y()+h()-2);
		}
		fl_line_style(0);
	}

	/* bars and beats drawing */

	fl_color(G_COLOR_GREY_4);
	for (int i=0; i<(int) pParent->gridTool->beats.size(); i++) {
		int px = pParent->gridTool->beats.at(i)+x()-1;
		fl_line(px, y()+1, px, y()+h()-2);
	}

	fl_color(G_COLOR_LIGHT_1);
	for (int i=0; i<(int) pParent->gridTool->bars.size(); i++) {
		int px = pParent->gridTool->bars.at(i)+x()-1;
		fl_line(px, y()+1, px, y()+h()-2);
	}

	/* cover unused area. Avoid drawing cover if width == 0 (i.e. beats
	 * are 32) */

	int coverWidth = pParent->totalWidth-pParent->coverX;
	if (coverWidth != 0)
		fl_rectf(pParent->coverX+x(), y()+1, coverWidth, h()-2, G_COLOR_GREY_4);
}
