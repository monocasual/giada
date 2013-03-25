/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_envelopeWidget
 *
 * parent class of any envelope controller, from volume to VST parameter
 * automations.
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */

#ifndef __GE_ENVELOPECHANNEL_H__
#define __GE_ENVELOPECHANNEL_H__

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include "ge_actionWidget.h"
#include "utils.h"


class gEnvelopeChannel : public gActionWidget {

	/* channel, type
	 * channel and type of action */

	struct channel *ch;
	int    type;
	int    range;

	/* point
	 * a single dot in the graph. x = relative frame, y = relative value */

	struct point {
		int   frame;
		int   iValue;
		float fValue;
		int   x;
		int   y;
	};

	/* points
	 * array of points, filled by fillPoints() */

	gVector<point> points;

	/* selectedPoint
	 * which point we are selecting? */

	int selectedPoint;

	/* draggedPoint
	 * which point we are dragging? */

	int draggedPoint;

	/* previousXPoint
	 * x coordinate of point at time t-1. Used to check effective shifts */

	int previousXPoint;

	void draw();

	int handle(int e);

	int getSelectedPoint();

	void sortPoints();

public:
	gEnvelopeChannel(int x, int y, gdActionEditor *parent, int type, int range, const char *l);
	~gEnvelopeChannel();

	/* addPoint
	 * add a point made of frame+value to points[]. This class, being general,
	 * doesn't query recorder or any other stacks for the actions. It's up
	 * to the caller to fill points[] with this method. */

	void addPoint(int frame, int iValue=0, float fValue=0.0f);

	void updatePoints();

	inline void clearPoints() { points.clear(); }
};

#endif
