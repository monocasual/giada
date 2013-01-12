/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_actionEditor
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
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

#ifndef GD_ACTIONEDITOR_H
#define GD_ACTIONEDITOR_H

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include "ge_window.h"
#include "gui_utils.h"
#include "mixer.h"
#include "recorder.h"


/* gActionEditor
 * main window which contains the tools for dealing with actions.
 * This class calculates chan, zoom, frames per beat, and so on. Each
 * sub-widget contains a pointer to this window to query those data. */

class gdActionEditor : public gWindow {

public:

	gdActionEditor(int chan);
	~gdActionEditor();

	int getActionType();

	static void cb_zoomIn(Fl_Widget *w, void *p);
	static void cb_zoomOut(Fl_Widget *w, void *p);
	inline void __cb_zoomIn();
	inline void __cb_zoomOut();

	class gChoice        *actionType;
	class gGridTool      *gridTool;
	class gClick         *zoomIn;
	class gClick         *zoomOut;
	class Fl_Scroll      *scroller;  // widget container
	class gActionChannel *ac;
	class gMuteChannel   *mc;

	int chan;
	int zoom;
	int framesPerBar;
	int framesPerBeat;
	int framesPerBeats;
	int totalFrames;
	int totalWidth;      // total width of the widget, in pixel (zoom affected)
	int beatWidth;       // total width of a beat cell, in pixel (zoom affected)
	int coverX; 				 // x1 of the unused area (x2 = totalWidth)
};


/* ------------------------------------------------------------------ */


class gGridTool : public Fl_Group {

private:
	class gChoice  *gridType;
	class gCheck   *active;

	class gdActionEditor *parent;

	static void cb_changeType(Fl_Widget *w, void *p);
	inline void __cb_changeType();

public:

	gGridTool(int x, int y, gdActionEditor *parent);
	~gGridTool();
	int  getValue();
	void init(int val, bool on);
	bool isOn();
	void calc();

	/* getSnapPoint
	 * given a cursor position in input, return the x coordinates of the
	 * nearest snap point (in pixel, clean, ie. not x()-shifted) */

	int  getSnapPoint(int v);

	gVector<int> points;   // points of the grid
};


#endif
