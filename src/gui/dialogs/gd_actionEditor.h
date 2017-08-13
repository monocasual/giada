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


#ifndef GD_ACTIONEDITOR_H
#define GD_ACTIONEDITOR_H


#include <vector>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include "window.h"


class Channel;
class geChoice;
class geGridTool;
class geButton;
class geButton;
class geScroll;
class geActionEditor;
class geMuteEditor;
class geEnvelopeEditor;
class geNoteEditor;


/* gActionEditor
Main window which contains the tools for dealing with actions. This class
calculates chan, zoom, frames per beat, and so on. Each sub-widget contains a
pointer to this window to query those data. */

class gdActionEditor : public gdWindow
{
private:

	/* update
  Computes total width, in pixel. */

	void update();

public:

	gdActionEditor(Channel *chan);
	~gdActionEditor();

	int handle(int e);

	int getActionType();

	static void cb_zoomIn(Fl_Widget *w, void *p);
	static void cb_zoomOut(Fl_Widget *w, void *p);
	inline void __cb_zoomIn();
	inline void __cb_zoomOut();

	geChoice   *actionType;
	geGridTool *gridTool;
	geButton   *zoomIn;
	geButton   *zoomOut;
	geScroll   *scroller;       // widget container

	geActionEditor   *ac;
	geMuteEditor     *mc;
	geEnvelopeEditor *vc;
	geNoteEditor     *pr;

	Channel *chan;

	int zoom;
	int totalWidth;  // total width of the widget, in pixel (zoom affected)
	int coverX; 		 // x1 of the unused area (x2 = totalWidth)
};


#endif
