/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_waveform
 * an element which represents a waveform.
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_WAVEFORM_H
#define GE_WAVEFORM_H

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <math.h>
#include "utils.h"

#define  FLAG_WIDTH  14
#define  FLAG_HEIGHT 12
#define  BORDER      8				// window border <-> widget border


class gWaveform : public Fl_Widget {

private:

	/* data
	 * real graphic stuff from the underlying waveform */
	
	struct data {
		int *sup;
		int *inf;
		int  size;
	} data;

	/* grid */

	struct grid {
		bool snap;
		int  level;
		gVector<int> points;
	} grid;

	/* chan
	 * chan in use. */

	class SampleChannel *chan;

	/* menuOpen
	 * is the menu open? */

	bool menuOpen;

	/* mouseOnStart/end
	 * is mouse on start or end flag? */

	bool mouseOnStart();
	bool mouseOnEnd();

	/* mouseOnSelectionA/B
	 * as above, for the selection */

	bool mouseOnSelectionA();
	bool mouseOnSelectionB();

	/* absolutePoint
	 * from a relative 'p' point (zoom affected) returns the same point
	 * zoom 1:1 based */

	int absolutePoint(int p);

	/* relativePoint
	 * from an absolute 'p' point (1:1 zoom), returns the same point zoom
	 * affected */

	int relativePoint(int p);

	/* straightSel
	 * helper function which flattens the selection if it was made from
	 * right to left (inverse selection) */

	void straightSel();

	/* freeData
	 * destroy any graphical buffer */

	void freeData();

	/* smaller
	 * is the waveform smaller than the parent window? */

	bool smaller();

  /* applySnap
   * snap a point at 'pos' pixel */

  int applySnap(int pos);

public:

	gWaveform(int x, int y, int w, int h, class SampleChannel *ch, const char *l=0);
	~gWaveform();
	void draw();
	int  handle(int e);

	/* alloc
	 * allocate memory for the picture */

	int alloc(int datasize=0);

	/* recalcPoints
	 * re-calc chanStart, chanEnd, ... */

	void recalcPoints();

	/* openEditMenu
	 * show edit menu on right-click */
	
	void openEditMenu();

	/* displayRatio
	 * how much of the waveform is being displayed on screen */

	inline float displayRatio() { return 1.0f / (data.size / (float) w()); };

	/* zoom
	 * type == 1 : zoom out, type == -1: zoom in */

	void setZoom(int type);

	/* strecthToWindow
	 * shrink or enlarge the waveform to match parent's width (gWaveTools) */

	void stretchToWindow();
	
	/* setGridLevel
	 * set a new frequency level for the grid. 0 means disabled. */

	void setGridLevel(int l);

  inline void setSnap(bool v) { grid.snap = v; }
  inline bool getSnap()       { return grid.snap; }
    
	inline int getSize() { return data.size; }

	int  chanStart;
	bool chanStartLit;
	int  chanEnd;
	bool chanEndLit;
	bool pushed;
	bool dragged;
	bool resized;

	float ratio;
  
  /* TODO - useless! use Fl::mouse_x() and Fl::mouse_y() instead */
	int  mouseX;					 // mouse pos for drag.n.drop
	int  mouseY;

	/* selectionA/B  = portion of the selected wave
	 * " " "" " _abs = selectionA/B not affected by zoom */
	/** TODO - change selectionA to selectionA_rel
	    TODO - change selectionB to selectionB_rel */
	int selectionA;
	int selectionB;
	int selectionA_abs;
	int selectionB_abs;
};


#endif
