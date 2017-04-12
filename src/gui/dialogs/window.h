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


#ifndef GD_WINDOW_H
#define GD_WINDOW_H


#include <vector>
#include <FL/Fl_Double_Window.H>


class gdWindow : public Fl_Double_Window
{
protected:

	std::vector <gdWindow*> subWindows;
	int id;
	gdWindow *parent;

public:

	gdWindow(int x, int y, int w, int h, const char *title=0, int id=0);
	gdWindow(int w, int h, const char *title=0, int id=0);
	~gdWindow();

	static void cb_closeChild(Fl_Widget *v, void *p);

	void addSubWindow(gdWindow *w);
	void delSubWindow(gdWindow *w);
	void delSubWindow(int id);

	int  getId();
	void setId(int id);
	void debug();

	void     setParent(gdWindow *);
	gdWindow *getParent();
	gdWindow *getChild(int id);

	/* hasWindow
	 * true if the window with id 'id' exists in the stack. */

	bool hasWindow(int id);
};


#endif
