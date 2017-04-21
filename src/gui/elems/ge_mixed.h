/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_mixed
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


#ifndef GE_MIXED_H
#define GE_MIXED_H

#include <stdio.h>
#include <dirent.h>
#include <stdint.h>  // for intptr_t
#include <string>
#include <FL/Fl.H>
#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Dial.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Scroll.H>

#ifdef _WIN32
	#include <shlobj.h>  // for SHGetFolderPath
#endif


/* cb_window_closer
 * callback for when closing windows. Deletes the widget (delete). */

void __cb_window_closer(Fl_Widget *v, void *p);


/* -------------------------------------------------------------------------- */


class gCheck : public Fl_Check_Button
{
public:
	gCheck(int x, int y, int w, int h, const char *L=0);
	void draw();
};


/* -------------------------------------------------------------------------- */


class gRadio : public Fl_Radio_Button
{
public:
	gRadio(int x, int y, int w, int h, const char *L=0);
	void draw();
};


/* -------------------------------------------------------------------------- */


class gProgress : public Fl_Progress
{
public:
	gProgress(int x, int y, int w, int h, const char *L=0);
};


/* -------------------------------------------------------------------------- */


class gSlider : public Fl_Slider
{
public:
	gSlider(int x, int y, int w, int h, const char *l=0);
	int id;
};


#endif
