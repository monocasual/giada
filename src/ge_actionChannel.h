/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_actionChannel
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

#ifndef GE_ACTIONCHANNEL_H
#define GE_ACTIONCHANNEL_H

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include "ge_actionWidget.h"
#include "gui_utils.h"
#include "mixer.h"
#include "recorder.h"


class gAction : public Fl_Box {
private:
	bool                  selected;
	unsigned              index;
  class gdActionEditor *parent;   // pointer to parent (gActionEditor)
  char                  type;     // type of action

public:
	gAction(int x, int y, int h, int frame_a, unsigned index, gdActionEditor *parent, bool record, char type);
	void draw();
	int  handle(int e);
	void addAction();
	void delAction();

	/* moveAction
	 * shift the action on the x-axis and update Recorder. If frame_a != -1
	 * use the new frame in input (used while snapping) */

	void moveAction(int frame_a=-1);

	/* absx
	 * x() is relative to scrolling position. absx() returns the absolute
	 * x value of the action, from the leftmost edge. */

	int  absx();

	/* xToFrame_a,b
	 * return the real frames of x() position */

	int xToFrame_a();
	int xToFrame_b();

	int frame_a;  // initial frame (KEYPRESS for singlemode.press)
	int frame_b;  // terminal frame (KEYREL for singlemode.press, null for others)

	bool onRightEdge;
	bool onLeftEdge;

	static const int MIN_WIDTH;
};


/* ------------------------------------------------------------------ */


class gActionChannel : public gActionWidget {
private:

	/* getSelectedAction
	 * get the action under the mouse. NULL if nothing found. */

	gAction *getSelectedAction();

	/* selected
	 * pointer to the selected action. Useful when dragging around. */

	gAction *selected;

	/* actionOriginalX, actionOriginalW
	 * x and w of the action, when moved. Useful for checking if the action
	 * overlaps another one: in that case the moved action returns to
	 * actionOriginalX (and to actionOriginalW if resized). */

	int actionOriginalX;
	int actionOriginalW;

	/* actionPickPoint
	 * the precise x point in which the action has been picked with the mouse,
	 * before a dragging action. */

	int actionPickPoint;


	/* actionCollides
	 * true if an action collides with another. Used while adding new points
	 * with snap active.*/

	bool actionCollides(int frame);

public:
	gActionChannel(int x, int y, gdActionEditor *parent);
	void draw();
	int  handle(int e);
	void updateActions();
};


#endif
