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


#include <FL/fl_draw.H>
#include "ge_envelopeChannel.h"
#include "gd_actionEditor.h"
#include "channel.h"
#include "recorder.h"


gEnvelopeChannel::gEnvelopeChannel(int x, int y, gdActionEditor *parent, int type, int range)
	:	gActionWidget(x, y, 200, 60, parent), type(type), range(range) {}


/* ------------------------------------------------------------------ */


gEnvelopeChannel::~gEnvelopeChannel() {
	clearPoints();
}


/* ------------------------------------------------------------------ */


void gEnvelopeChannel::addPoint(int frame, int iValue, float fValue) {
	point p;
	p.frame  = frame;
	p.iValue = iValue;
	p.fValue = fValue;
	p.x = (p.frame / parent->zoom) + x();
	if (range == RANGE_CHAR)
		p.y = p.iValue / h();
	else
		p.y = p.fValue / h();
	points.add(p);
}


/* ------------------------------------------------------------------ */


void gEnvelopeChannel::updatePoints() {
	for (unsigned i=0; i<points.size; i++)
		points.at(i).x = (points.at(i).frame / parent->zoom) + x();
}


/* ------------------------------------------------------------------ */


void gEnvelopeChannel::draw() {
	baseDraw();

	for (unsigned i=0; i<points.size; i++) {
		//printf("draw point %d, x=%d, val=%f\n", i, points.at(i).x, points.at(i).fValue);

		fl_color(COLOR_BG_2);

		if (selectedPoint == (int) i) {
			fl_color(COLOR_BD_1);
			fl_rectf(points.at(i).x, y(), 7, 7);
			fl_color(COLOR_BG_2);
		}
		else
			fl_rectf(points.at(i).x, y(), 7, 7);
	}
}


/* ------------------------------------------------------------------ */


int gEnvelopeChannel::handle(int e) {

	/* Adding an action: no further checks required, just record it on frame
	 * mx*parent->zoom. Deleting action is trickier: find the active
	 * point and derive from it the corresponding frame. */

	int ret = 0;
	int mx  = Fl::event_x()-x();  // mouse x
	int my  = Fl::event_y()-y();  // mouse y

	switch (e) {

		case FL_ENTER: {
			ret = 1;
			break;
		}

		case FL_MOVE: {
			selectedPoint = getSelectedPoint();
			redraw();
			ret = 1;
			break;
		}

		case FL_LEAVE: {
			draggedPoint  = -1;
			selectedPoint = -1;
			redraw();
			ret = 1;
			break;
		}

		case FL_PUSH: {

			/* left click on point: drag
			 * right click on point: delete
			 * left click on void: add */

			if (Fl::event_button1()) {

				int frame = mx*parent->zoom;

				if (range == RANGE_FLOAT) {
					float value = (-1.0f / (h()-1)) * (my - (h()-1));
					addPoint(frame, 0, value);
					recorder::rec(parent->chan->index, type, frame, 0, value);
				}
				else {
					//int value = (-255 / (float) (h()-1)) * (my - h()-1);
					//addPoint(frame, value, 0.0f);
					//recorder::rec(parent->chan->index, type, frame, value, 0.0f);
					//printf("rec action INT at x=%d (%d), h=%d, y=%d, v=%d\n", mx, mx*parent->zoom, h(), my, v);
				}
				redraw();
			}
			else {

				if (selectedPoint != -1) {
					recorder::deleteAction(parent->chan->index, points.at(selectedPoint).frame, type);
					points.del(selectedPoint);
					redraw();
				}
			}

			ret = 1;
			break;
		}
	}

	return ret;
}


/* ------------------------------------------------------------------ */


int gEnvelopeChannel::getSelectedPoint() {

	/* point is a 7x7 dot */

	for (unsigned i=0; i<points.size; i++) {
		if (Fl::event_x() >= points.at(i).x-3 &&
				Fl::event_x() <= points.at(i).x+3)
		return i;
	}
	return -1;
}
