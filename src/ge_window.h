/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_window
 * A custom window.
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


#ifndef __GE_WINDOW_H__
#define __GE_WINDOW_H__


#include <FL/Fl_Double_Window.H>
#include "utils.h"


class gWindow : public Fl_Double_Window {

protected:
	gVector <gWindow *> subWindows;
	int id;
	gWindow *parent;

public:
	gWindow(int x, int y, int w, int h, const char *title=0, int id=0);
	gWindow(int w, int h, const char *title=0, int id=0);
	~gWindow();

	static void cb_closeChild(Fl_Widget *v, void *p);

	void addSubWindow(gWindow *w);
	void delSubWindow(gWindow *w);
	void delSubWindow(int id);

	int  getId();
	void setId(int id);
	void debug();

	void     setParent(gWindow *);
	gWindow *getParent();
	gWindow *getChild(int id);

	/* hasWindow
	 * true if the window with id 'id' exists in the stack. */

	bool hasWindow(int id);

};


#endif
