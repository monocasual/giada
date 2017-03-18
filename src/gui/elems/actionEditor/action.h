/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
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
 * -------------------------------------------------------------------------- */


#ifndef GE_ACTION_H
#define GE_ACTION_H


#include <FL/Fl_Box.H>


class gdActionEditor;
class SampleChannel;


class geAction : public Fl_Box
{
private:

	bool            selected;
	unsigned        index;
  gdActionEditor *parent;   // pointer to parent (geActionEditor)
	SampleChannel  *ch;
  char            type;     // type of action

public:

	geAction(int x, int y, int h, int frame_a, unsigned index,
		gdActionEditor *parent, SampleChannel *ch, bool record, char type);
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

	int absx();

	/* xToFrame_a,b
	 * return the real frames of x() position */

	int xToFrame_a();
	int xToFrame_b();

	int frame_a;  // initial frame (KEYPRESS for singlemode.press)
	int frame_b;  // terminal frame (KEYREL for singlemode.press, null for others)

	bool onRightEdge;
	bool onLeftEdge;

	static const int MIN_WIDTH = 8;
};


#endif
