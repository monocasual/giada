/* -----------------------------------------------------------------------------
*
* Giada - Your Hardcore Loopmachine
*
* ------------------------------------------------------------------------------
*
* Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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
* --------------------------------------------------------------------------- */


#include <cmath>
#include "../../../core/conf.h"
#include "../../../core/const.h"
#include "../../../core/clock.h"
#include "../../dialogs/gd_actionEditor.h"
#include "../ge_mixed.h"
#include "actionEditor.h"
#include "gridTool.h"


extern Conf	 G_Conf;
extern Clock G_Clock;


geGridTool::geGridTool(int x, int y, gdActionEditor *parent)
	:	Fl_Group(x, y, 80, 20), parent(parent)
{
	gridType = new gChoice(x, y, 40, 20);
	gridType->add("1");
	gridType->add("2");
	gridType->add("3");
	gridType->add("4");
	gridType->add("6");
	gridType->add("8");
	gridType->add("16");
	gridType->add("32");
	gridType->value(0);
	gridType->callback(cb_changeType, (void*)this);

	active = new gCheck (x+44, y+4, 12, 12);

	gridType->value(G_Conf.actionEditorGridVal);
	active->value(G_Conf.actionEditorGridOn);

	end();
}


/* -------------------------------------------------------------------------- */


geGridTool::~geGridTool()
{
	G_Conf.actionEditorGridVal = gridType->value();
	G_Conf.actionEditorGridOn  = active->value();
}


/* -------------------------------------------------------------------------- */


void geGridTool::cb_changeType(Fl_Widget *w, void *p)  { ((geGridTool*)p)->__cb_changeType(); }


/* -------------------------------------------------------------------------- */


void geGridTool::__cb_changeType()
{
	calc();
	parent->redraw();
}


/* -------------------------------------------------------------------------- */


bool geGridTool::isOn()
{
	return active->value();
}


/* -------------------------------------------------------------------------- */


int geGridTool::getValue()
{
	switch (gridType->value()) {
		case 0:	return 1;
		case 1: return 2;
		case 2: return 3;
		case 3: return 4;
		case 4: return 6;
		case 5: return 8;
		case 6: return 16;
		case 7: return 32;
	}
	return 0;
}


/* -------------------------------------------------------------------------- */


void geGridTool::calc()
{
	points.clear();
	frames.clear();
	bars.clear();
	beats.clear();

	/* find beats, bars and grid. The method is the same of the waveform in sample
	 * editor. Take totalwidth (the width in pixel of the area to draw), knowing
	 * that totalWidth = totalFrames / zoom. Then, for each pixel of totalwidth,
	 * put a concentrate of each block (which is totalFrames / zoom) */

	int  j   = 0;
	int fpgc = floor(G_Clock.getFramesPerBeat() / getValue());  // frames per grid cell

	for (int i=1; i<parent->totalWidth; i++) {   // if i=0, step=0 -> useless cycle
		int step = parent->zoom*i;
		while (j < step && j < G_Clock.getTotalFrames()) {
			if (j % fpgc == 0) {
				points.push_back(i);
				frames.push_back(j);
			}
			if (j % G_Clock.getFramesPerBeat() == 0)
				beats.push_back(i);
			if (j % G_Clock.getFramesPerBar() == 0 && i != 1)
				bars.push_back(i);
			if (j == G_Clock.getTotalFrames() - 1)
				parent->coverX = i;
			j++;
		}
		j = step;
	}

	/* fix coverX if == 0, which means G_Mixer.beats == G_MAX_BEATS */

	if (G_Clock.getBeats() == G_MAX_BEATS)
		parent->coverX = parent->totalWidth;
}


/* -------------------------------------------------------------------------- */


int geGridTool::getSnapPoint(int v)
{
	if (v == 0) return 0;

	for (int i=0; i<(int)points.size(); i++) {

		if (i == (int) points.size()-1)
			return points.at(i);

		int gp  = points.at(i);
		int gpn = points.at(i+1);

		if (v >= gp && v < gpn)
			return gp;
	}
	return v;  // default value
}


/* -------------------------------------------------------------------------- */


int geGridTool::getSnapFrame(int v)
{
	v *= parent->zoom;  // transformation pixel -> frame

	for (int i=0; i<(int)frames.size(); i++) {

		if (i == (int) frames.size()-1)
			return frames.at(i);

		int gf  = frames.at(i);     // grid frame
		int gfn = frames.at(i+1);   // grid frame next

		if (v >= gf && v < gfn) {

			/* which one is the closest? gf < v < gfn */

			if ((gfn - v) < (v - gf))
				return gfn;
			else
				return gf;
		}
	}
	return v;  // default value
}


/* -------------------------------------------------------------------------- */


int geGridTool::getCellSize()
{
	return (parent->coverX - parent->ac->x()) / G_Clock.getBeats() / getValue();
}
