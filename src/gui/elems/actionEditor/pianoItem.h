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


#ifndef GE_PIANO_ITEM_H
#define GE_PIANO_ITEM_H


#include "../../../core/recorder.h"
#include "../../../core/midiEvent.h"
#include "basePianoItem.h"


class gdActionEditor;


class gePianoItem : public geBasePianoItem
{
private:

	struct giada::m::recorder::action a;
	struct giada::m::recorder::action b;

	int push_x;

	/* changed
	If Item has been moved or resized: re-recording needed. */

	bool changed;

	/* onLeft, RightEdge
	If cursor is on a widget's edge. */

	bool onLeftEdge;
	bool onRightEdge;

	/* old_x, old_w
	Store previous width and position while dragging and moving, in order to 
	restore it if overlap. */

	int old_x, old_w;

	/* getRelX/Y
	Returns x/y point of this item, relative to piano roll (and not to entire 
	screen). */

	int getRelY();
	int getRelX();

	/* overlap
	Checks if this item don't overlap with another one. */

	bool overlap();

public:

	static const int MIN_WIDTH    = 10;
	static const int HANDLE_WIDTH = 5;

	gePianoItem(int x, int y, int rel_x, int rel_y, 
		struct giada::m::recorder::action a, struct giada::m::recorder::action b, 
		gdActionEditor* pParent);
 
	void draw() override;
	int handle(int e) override;

  void reposition(int pianoRollX) override;

	void removeAction();
};


#endif
