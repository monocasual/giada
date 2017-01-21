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

#ifndef GUI_UTILS_H
#define GUI_UTILS_H

#include <dirent.h>
#include <string>
#include <FL/x.H>
#include <FL/Fl.H>
#ifdef __APPLE__
	#include <libgen.h>	// in osx, for basename() (but linux?)
#endif

/* including stuff for the favicon */

#if defined(_WIN32)
	#include "../ext/resource.h"
#elif defined(__linux__)
	#include <X11/xpm.h>
#endif


using std::string;


/* refresh
 * refresh all GUI elements. */

void gu_refreshUI();

/* getBlinker
*  return blinker value, used to make widgets blink. */

int gu_getBlinker();

/* updateControls
 * update attributes of control elements (sample names, volumes, ...).
 * Useful when loading a new patch. */

void gu_updateControls();

/* update_win_label
 * update the name of the main window */

void gu_updateMainWinLabel(const string &s);

void gu_setFavicon(Fl_Window *w);

void gu_openSubWindow(class gWindow *parent, gWindow *child, int id);

/* refreshActionEditor
 * reload the action editor window by closing and reopening it. It's used
 * when you delete some actions from the mainWindow and the action editor
 * window is open. */

void gu_refreshActionEditor();


/* closeAllSubwindows
 * close all subwindows attached to mainWin. */

void gu_closeAllSubwindows();


/* getSubwindow
 * return a pointer to an open subwindow, otherwise NULL. */

gWindow *gu_getSubwindow(class gWindow *parent, int id);

/* removeFltkChars
 * Strip special chars used by FLTK to split menus into sub-menus. */

string gu_removeFltkChars(const string &s);

#endif
