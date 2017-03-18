/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_envelopeWidget
 *
 * Parent class of any envelope controller, from volume to VST parameter
 * automations.
 *
 * ---------------------------------------------------------------------
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
 * ------------------------------------------------------------------ */


#include <FL/fl_draw.H>
#include "../../../core/channel.h"
#include "../../../core/recorder.h"
#include "../../../core/mixer.h"
#include "../../../core/clock.h"
#include "../../dialogs/gd_actionEditor.h"
#include "../../dialogs/gd_mainWindow.h"
#include "../mainWindow/keyboard/keyboard.h"
#include "gridTool.h"
#include "envelopeEditor.h"


extern Mixer         G_Mixer;
extern Clock         G_Clock;
extern Recorder      G_Recorder;
extern gdMainWindow *G_MainWin;


geEnvelopeEditor::geEnvelopeEditor(int x, int y, gdActionEditor *pParent,
	int type, int range, const char *l)
	:	geBaseActionEditor(x, y, 200, 80, pParent),
		l                 (l),
		type              (type),
		range             (range),
		selectedPoint     (-1),
		draggedPoint      (-1)
{
	size(pParent->totalWidth, h());
}


/* ------------------------------------------------------------------ */


geEnvelopeEditor::~geEnvelopeEditor() {
	clearPoints();
}


/* ------------------------------------------------------------------ */


void geEnvelopeEditor::addPoint(int frame, int iValue, float fValue, int px, int py) {
	point p;
	p.frame  = frame;
	p.iValue = iValue;
	p.fValue = fValue;
	p.x = px;
	p.y = py;
	points.push_back(p);
}


/* ------------------------------------------------------------------ */


void geEnvelopeEditor::updateActions() {
	for (unsigned i=0; i<points.size(); i++)
		points.at(i).x = points.at(i).frame / pParent->zoom;
}


/* ------------------------------------------------------------------ */


void geEnvelopeEditor::draw() {

	baseDraw();

	/* print label */

	fl_color(COLOR_BG_1);
	fl_font(FL_HELVETICA, 12);
	fl_draw(l, x()+4, y(), 80, h(), (Fl_Align) (FL_ALIGN_LEFT));

	int pxOld = x()-3;
	int pyOld = y()+1;
	int pxNew = 0;
	int pyNew = 0;

	fl_color(COLOR_BG_2);

	for (unsigned i=0; i<points.size(); i++) {

		pxNew = points.at(i).x+x()-3;
		pyNew = points.at(i).y+y();

		if (selectedPoint == (int) i) {
			fl_color(COLOR_BD_1);
			fl_rectf(pxNew, pyNew, 7, 7);
			fl_color(COLOR_BG_2);
		}
		else
			fl_rectf(pxNew, pyNew, 7, 7);

		if (i > 0)
			fl_line(pxOld+3, pyOld+3, pxNew+3, pyNew+3);

		pxOld = pxNew;
		pyOld = pyNew;
	}
}


/* ------------------------------------------------------------------ */


int geEnvelopeEditor::handle(int e) {

	/* Adding an action: no further checks required, just record it on frame
	 * mx*pParent->zoom. Deleting action is trickier: find the active
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

				if (selectedPoint != -1) {
					draggedPoint = selectedPoint;
				}
				else {

					/* top & border fix */

					if (my > h()-8) my = h()-8;
					if (mx > pParent->coverX-x()) mx = pParent->coverX-x();

					if (range == RANGE_FLOAT) {

						/* if this is the first point ever, add other two points at the beginning
						 * and the end of the range */

						if (points.size() == 0) {
							addPoint(0, 0, 1.0f, 0, 1);
							G_Recorder.rec(pParent->chan->index, type, 0, 0, 1.0f);
							addPoint(G_Clock.getTotalFrames(), 0, 1.0f, pParent->coverX, 1);
							G_Recorder.rec(pParent->chan->index, type, G_Clock.getTotalFrames(), 0, 1.0f);
              pParent->chan->hasActions = true;
						}

						/* line between 2 points y = (x-a) / (b-a); a = h() - 8; b = 1 */

						int frame   = mx * pParent->zoom;
						float value = (my - h() + 8) / (float) (1 - h() + 8);
						addPoint(frame, 0, value, mx, my);
						G_Recorder.rec(pParent->chan->index, type, frame, 0, value);
            pParent->chan->hasActions = true;
						G_Recorder.sortActions();
						sortPoints();
					}
					else {
						/// TODO
					}
					G_MainWin->keyboard->setChannelWithActions((geSampleChannel*)pParent->chan->guiChannel); // update mainWindow
					redraw();
				}
			}
			else {

				/* right click on point 0 or point size-1 deletes the entire envelope */

				if (selectedPoint != -1) {
					if (selectedPoint == 0 || (unsigned) selectedPoint == points.size()-1) {
						G_Recorder.clearAction(pParent->chan->index, type);
            pParent->chan->hasActions = G_Recorder.hasActions(pParent->chan->index);
						points.clear();
					}
					else {
						G_Recorder.deleteAction(pParent->chan->index,
              points.at(selectedPoint).frame, type, false, &G_Mixer.mutex_recs);
            pParent->chan->hasActions = G_Recorder.hasActions(pParent->chan->index);
            G_Recorder.sortActions();
						points.erase(points.begin() + selectedPoint);
					}
					G_MainWin->keyboard->setChannelWithActions((geSampleChannel*)pParent->chan->guiChannel); // update mainWindow
					redraw();
				}
			}

			ret = 1;
			break;
		}

		case FL_RELEASE: {
			if (draggedPoint != -1) {

				if (points.at(draggedPoint).x == previousXPoint) {
					//gu_log("nothing to do\n");
				}
				else {
					int newFrame = points.at(draggedPoint).x * pParent->zoom;

					/* x edge correction */

					if (newFrame < 0)
						newFrame = 0;
					else if (newFrame > G_Clock.getTotalFrames())
						newFrame = G_Clock.getTotalFrames();

					/* vertical line check */

					int vp = verticalPoint(points.at(draggedPoint));
					if (vp == 1) 			 newFrame -= 256;
					else if (vp == -1) newFrame += 256;

					/*  delete previous point and record a new one */

					G_Recorder.deleteAction(pParent->chan->index,
            points.at(draggedPoint).frame, type, false, &G_Mixer.mutex_recs);
          pParent->chan->hasActions = G_Recorder.hasActions(pParent->chan->index);

					if (range == RANGE_FLOAT) {
						float value = (points.at(draggedPoint).y - h() + 8) / (float) (1 - h() + 8);
						G_Recorder.rec(pParent->chan->index, type, newFrame, 0, value);
            pParent->chan->hasActions = true;
					}
					else {
						/// TODO
					}

					G_Recorder.sortActions();
					points.at(draggedPoint).frame = newFrame;
					draggedPoint  = -1;
					selectedPoint = -1;
				}
			}
			ret = 1;
			break;
		}

		case FL_DRAG: {

			if (draggedPoint != -1) {

				/* y constraint */

				if (my > h()-8)
					points.at(draggedPoint).y = h()-8;
				else
				if (my < 1)
					points.at(draggedPoint).y = 1;
				else
					points.at(draggedPoint).y = my;

				/* x constraint
				 * constrain the point between two ends (leftBorder-point, point-point,
				 * point-rightBorder). First & last points cannot be shifted on x */

				if (draggedPoint == 0)
					points.at(draggedPoint).x = x()-8;
				else
				if ((unsigned) draggedPoint == points.size()-1)
					points.at(draggedPoint).x = pParent->coverX;
				else {
					int prevPoint = points.at(draggedPoint-1).x;
					int nextPoint = points.at(draggedPoint+1).x;
					if (mx <= prevPoint)
						points.at(draggedPoint).x = prevPoint;
					else
					if (mx >= nextPoint)
						points.at(draggedPoint).x = nextPoint;
					//else
					//	points.at(draggedPoint).x = mx;
					else {
						if (pParent->gridTool->isOn())
							points.at(draggedPoint).x = pParent->gridTool->getSnapPoint(mx)-1;
						else
							points.at(draggedPoint).x = mx;
					}
				}
				redraw();
			}

			ret = 1;
			break;
		}
	}

	return ret;
}


/* ------------------------------------------------------------------ */


int geEnvelopeEditor::verticalPoint(const point &p) {
	for (unsigned i=0; i<points.size(); i++) {
		if (&p == &points.at(i)) {
			if (i == 0 || i == points.size()-1)  // first or last point
				return 0;
			else {
				if (points.at(i-1).x == p.x)    // vertical with point[i-1]
					return -1;
				else
				if (points.at(i+1).x == p.x)    // vertical with point[i+1]
					return 1;
			}
			break;
		}
	}
	return 0;
}


/* ------------------------------------------------------------------ */


void geEnvelopeEditor::sortPoints() {
	for (unsigned i=0; i<points.size(); i++)
		for (unsigned j=0; j<points.size(); j++)
			if (points.at(j).x > points.at(i).x)
				std::swap(points.at(j), points.at(i));
}


/* ------------------------------------------------------------------ */


int geEnvelopeEditor::getSelectedPoint() {

	/* point is a 7x7 dot */

	for (unsigned i=0; i<points.size(); i++) {
		if (Fl::event_x() >= points.at(i).x+x()-4  &&
				Fl::event_x() <= points.at(i).x+x()+4  &&
				Fl::event_y() >= points.at(i).y+y()    &&
				Fl::event_y() <= points.at(i).y+y()+7)
		return i;
	}
	return -1;
}


/* ------------------------------------------------------------------ */


void geEnvelopeEditor::fill() {
	points.clear();
	for (unsigned i=0; i<G_Recorder.global.size(); i++)
		for (unsigned j=0; j<G_Recorder.global.at(i).size(); j++) {
			Recorder::action *a = G_Recorder.global.at(i).at(j);
			if (a->type == type && a->chan == pParent->chan->index) {
				if (range == RANGE_FLOAT)
					addPoint(
						a->frame,                      // frame
						0,                             // int value (unused)
						a->fValue,                     // float value
						a->frame / pParent->zoom,       // x
						((1-h()+8)*a->fValue)+h()-8);  // y = (b-a)x + a (line between two points)
				// else: TODO
			}
		}

}
