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


#ifndef GE_WAVEFORM_H
#define GE_WAVEFORM_H


#include <vector>
#include <FL/Fl_Widget.H>


class SampleChannel;


class geWaveform : public Fl_Widget
{
private:

	static const int FLAG_WIDTH  = 20;
	static const int FLAG_HEIGHT = 20;
	static const int BORDER      = 8;  // window border <-> widget border
	static const int SNAPPING    = 10;

	/* selection
	Portion of the selected wave, in pixel and in frames. */

	struct
	{
		int aPixel;
		int bPixel;
		int aFrame;
		int bFrame;
	} selection;

	/* data
	Real graphic stuff from the underlying waveform. */

	struct
  {
		int *sup;   // upper part of the waveform
		int *inf;   // lower ""   "" ""  ""
		int  size;  // width of the waveform to draw (in pixel)
	} data;

	struct
  {
		bool snap;
		int level;
		std::vector<int> points;
	} grid;

	SampleChannel* chan;

	/* mouseOnStart/end
	Is mouse on start or end flag? */

	bool mouseOnStart();
	bool mouseOnEnd();

	/* mouseOnSelectionA/B
	As above, for the selection. */

	bool mouseOnSelectionA();
	bool mouseOnSelectionB();

	/* absolutePoint
	From a relative 'p' point (zoom affected) returns the same point zoom 1:1 
	based. */

	int absolutePoint(int p);

	/* relativePoint
	From an absolute 'p' point (1:1 zoom) returns the same point zoom affected. */

	int relativePoint(int p);

	/* fixSelection
	Helper function which flattens the selection if it was made from right to left 
	(inverse selection). It also computes the absolute points. Call this one
	whenever the selection gesture is done. */

	void fixSelection();

	/* freeData
	Destroys any graphical buffer. */

	void freeData();

	/* smaller
	Is the waveform smaller than the parent window? */

	bool smaller();

  /* applySnap
  Snap a point at 'pos' pixel. */

  int applySnap(int pos);

public:

	geWaveform(int x, int y, int w, int h, SampleChannel* ch, const char* l=0);
	~geWaveform();

	void draw() override;
	int  handle(int e) override;

	/* alloc
	 * allocate memory for the picture */

	int alloc(int datasize=0);

	/* recalcPoints
	 * re-calc chanStart, chanEnd, ... */

	void recalcPoints();

	/* zoom
	 * type == 1 : zoom out, type == -1: zoom in */

	void setZoom(int type);

	/* strecthToWindow
	 * shrink or enlarge the waveform to match parent's width (gWaveTools) */

	void stretchToWindow();

	/* refresh
	Redraws the waveform. */

	void refresh();

	/* setGridLevel
	 * set a new frequency level for the grid. 0 means disabled. */

	void setGridLevel(int l);

  void setSnap(bool v) { grid.snap = v; }
  bool getSnap()       { return grid.snap; }

	int getSize() { return data.size; }

	/* isSelected
	Tells whether a portion of the waveform has been selected. */

	bool isSelected();

	int getSelectionA();
	int getSelectionB();

	/* clearSel
	Removes any active selection. */

	void clearSel();

	int  chanStart;
	bool chanStartLit;
	int  chanEnd;
	bool chanEndLit;
	bool pushed;
	bool dragged;
	bool resized;
	float ratio;
	int mouseX;
	int mouseY;
};


#endif
